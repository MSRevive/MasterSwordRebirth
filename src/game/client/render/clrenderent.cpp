/*
=====================
Game_AddObjects

Add game specific, client-side objects here
=====================
*/
#include "inc_weapondefs.h"
#include "../hud.h"
#include "../cl_util.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
//#include "triangleapi.h"

#include "studio_util.h"
#include "../r_studioint.h"

//#include "r_efx.h"
#include "entity_types.h"
#include "clglobal.h"
#include "clrender.h"
#include "studiomodelrenderer.h"

#include "hudmisc.h"
void CLFrameShowModel(cl_entity_t &Entity);
mslist<CRenderEntity *> g_RenderEnts;
bool ShowHUD();

bool g_FirstRender = false;

void Game_AddObjects(void)
{
	g_FirstRender = true;

	for (int i = 0; i < MSCLGlobals::m_ClModels.size(); i++)
	{
		cl_entity_t &Entity = MSCLGlobals::m_ClModels[i];
		CLFrameShowModel(Entity);
	}
}

//Animates the model and displays it for this frame
void CLFrameShowModel(cl_entity_t &Entity)
{
	if (Entity.model)
	{
		float Fps = 0;
		float Frames = 0;
		bool Loop = false;
		if (Entity.model->type == mod_studio)
		{
			studiohdr_t *pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(Entity.model);
			mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)pStudioHeader + pStudioHeader->seqindex) + Entity.curstate.sequence;
			Frames = pseqdesc->numframes;
			Fps = Entity.curstate.framerate * pseqdesc->fps;
			Loop = FBitSet(pseqdesc->flags, STUDIO_LOOPING);
		}
		else if (Entity.model->type == mod_sprite)
		{
			Frames = 1;
			Fps = Entity.curstate.framerate;
			Loop = true;
		}

		float Currentframe = (gpGlobals->time - Entity.curstate.animtime) * Fps;
		float LastFrame = ((gpGlobals->time - gpGlobals->frametime) - Entity.curstate.animtime) * Fps;
		if ((Currentframe >= Frames && LastFrame < Frames && Loop) || Currentframe < 0)
		{
			Entity.curstate.frame = 0;
			Entity.curstate.animtime = gpGlobals->time;
		}
		else
			Entity.curstate.frame = Currentframe / Frames; //0 - 1 (ratio of anim player)  1+ (Beyond anim frames)
	}

	int Ent = gEngfuncs.CL_CreateVisibleEntity(ET_NORMAL, &Entity);
}

void cl_entity_s::SetBody(int Group, int Value)
{
	studiohdr_t *pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(model);

	if (pStudioHeader)
		if (Group <= pStudioHeader->numbodyparts)
		{
			mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)((byte *)pStudioHeader + pStudioHeader->bodypartindex) + Group;

			if (Value < pbodypart->nummodels)
			{
				int iCurrent = (curstate.body / pbodypart->base) % pbodypart->nummodels;
				curstate.body = (curstate.body - (iCurrent * pbodypart->base) + (Value * pbodypart->base));
			}
		}
}

bool cl_entity_s::PlayAnim(msstring_ref Anim)
{
	studiohdr_t *pstudiohdr;

	pstudiohdr = (studiohdr_t *)IEngineStudio.Mod_Extradata(model);
	if (!pstudiohdr)
		return 0;

	mstudioseqdesc_t *pseqdesc;

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);

	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		if (stricmp(pseqdesc[i].label, Anim) != 0)
			continue;

		PlayAnim(i);
		return true;
	}

	return false;
}
bool cl_entity_s::SetModel(const char *pszModelName)
{
	model = gEngfuncs.CL_LoadModel(pszModelName, &curstate.modelindex);
	return model ? true : false;
}
void cl_entity_s::PlayAnim(int Anim)
{
	curstate.sequence = Anim;
	curstate.animtime = gpGlobals->time;
}
bool cl_entity_s::HasExtra()
{
	return FBitSet(mouth.sndavg, MSRDR_HASEXTRA) ? true : false;
}

//Full rotation is when a player model is allowed to rotate its pitch
bool cl_entity_s::FullRotate()
{
	return FBitSet(curstate.iuser3, PLAYER_MOVE_SWIMMING) ? true : false;
}

bool cl_entity_s::GetBonePos(int BoneNum, Vector &BonePos)
{
	//if( !HasExtra( ) )
	//	return false;

	BonePos = g_vecZero;
	modelinfo_t &ModelInfo = CModelMgr::m_ModelInfo[index];
	if (!ModelInfo.Used)
		return false;

	VectorTransform(g_vecZero, ModelInfo.BoneTransformations[BoneNum], BonePos);

	return true;
}

//AUG2013_25 Thothie - fixing cl bonepos acquisition
Vector cl_entity_s::GetBonePosVec(int BoneNum, Vector &BonePos)
{
	BonePos = g_vecZero;
	modelinfo_t &ModelInfo = CModelMgr::m_ModelInfo[index];
	VectorTransform(g_vecZero, ModelInfo.BoneTransformations[BoneNum], BonePos);
	return BonePos;
}

//AUG2013_25 Thothie - getting bone count
int cl_entity_s::GetBoneCount()
{
	modelinfo_t &ModelInfo = CModelMgr::m_ModelInfo[index];
	return ModelInfo.Bones;
}

bool cl_entity_s::Exists()
{
	return curstate.msg_time >= gEngfuncs.GetClientTime();
}
Vector cl_entity_s::Origin()
{
	if (curstate.movetype == MOVETYPE_FOLLOW)
	{
		cl_entity_t *pParent = gEngfuncs.GetEntityByIndex(curstate.aiment);
		if (pParent)
			return pParent->Origin();
	}
	return origin;
}

bool g_TempEntNewLevel = true;
bool g_InsetLoaded = false;
CRenderPlayerInset g_Inset;
void CBasePlayer::BeginRender()
{
	//Called each map load
	if (!g_InsetLoaded)
	{
		g_Inset.SetGear(&player.Gear);
		g_RenderEnts.add(&g_Inset);
		g_InsetLoaded = true;
	}

	CEnvMgr::Init();

	g_TempEntNewLevel = true;
}

void CBasePlayer::Render()
{
	for (int i = 0; i < g_RenderEnts.size(); i++)
		g_RenderEnts[i]->Render();
}
void CBasePlayer::RenderCleanup()
{
	mslist<CRenderEntity *> RenderEntListCopy;
	RenderEntListCopy = g_RenderEnts;

	int size = RenderEntListCopy.size();
	for (int i = 0; i < size; i++)
	{
		CRenderEntity *pRdrEnt = RenderEntListCopy[i];
		if (pRdrEnt->IsPermanent())
			continue;

		pRdrEnt->UnRegister();
	}
}

void CRenderEntity::Register()
{
	g_RenderEnts.add(this);
}
void CRenderEntity::Render()
{
	if (IsVisible() && IsClientEntity())
		CLFrameShowModel(GetEntity());
}
void CRenderEntity::UnRegister()
{
	for (int i = 0; i < g_RenderEnts.size(); i++)
		if (g_RenderEnts[i] == this)
		{
			g_RenderEnts.erase(i--);
			break;
		}
	CB_UnRegistered(); //Give the render ent a chance to delete itself
}
void CRenderEntity::CB_UnRegistered()
{
	delete this; //Default behavior - Delete myself
}

//Render a player model
//Attaches a bunch of gear to itself
CRenderPlayer::CRenderPlayer()
{
	//memset( m_BodyParts, 0, sizeof(m_BodyParts) );
}

void CRenderPlayer::Render()
{
	//m_Ent.origin and m_Ent.angles are calulated in V_CalcRefdef for precise accuracy.
	cl_entity_t &Ent = GetEntity();

	/* for (int i = 0; i < HUMAN_BODYPARTS; i++) 
	{
		cl_entity_t &BPEnt = m_BodyParts[i];
		BPEnt.SetModel( ModelList[i][m_Gender] );
		BPEnt.curstate.number = -1;
		BPEnt.AttachTo( Ent );
		BPEnt.curstate.colormap = 0;
		//Copy flags that propogate from playermodel to bodyparts
		int KeepMask = MSRDR_GLOW_RED|MSRDR_GLOW_GRN|MSRDR_LIGHT_DIM|MSRDR_LIGHT_NORMAL|MSRDR_DARK;
		int KeepFlags = Ent.curstate.colormap & KeepMask;
		SetBits( BPEnt.curstate.colormap, KeepFlags );

		BPEnt.curstate.renderfx = Ent.curstate.renderfx;
		BPEnt.curstate.rendercolor = Ent.curstate.rendercolor;
		BPEnt.curstate.renderamt = Ent.curstate.renderamt;
	}*/

	//Ent.curstate.body = 0; //MIB JAN2010_29 armor fix fix fix fix (comment this line)
	CRenderEntity::Render();

	CItemList &Gear = GetGear();
	for (int i = 0; i < Gear.size(); i++)
		RenderGearItem(*Gear[i]);

	/* for (int i = 0; i < HUMAN_BODYPARTS; i++) 
	{
		cl_entity_t &BPEnt = m_BodyParts[i];
		if( BPEnt.curstate.number != 0 ) CLFrameShowModel( BPEnt );
	}*/
}

void CRenderPlayer::RenderGearItem(CGenericItem &Item)
{
	cl_entity_t &Ent = GetEntity();
	cl_entity_t &ItemEnt = GearItemEntity(Item);

	if (!ItemEnt.model)
		return;

	ItemEnt.AttachTo(Ent);
	ItemEnt.curstate.number = -1;
	ItemEnt.curstate.framerate = 0;
	ItemEnt.curstate.frame = 0;

	//JAN2010_30 MiB Armor Fix Fix (fix) - comment out
	//	if( Item.IsWorn( ) )
	//	{
	//Thothie note: Does nothing until player is in world
	//		foreach( i, Item.m_WearModelPositions.size( ) )
	//			Ent.SetBody( Item.m_WearModelPositions[i], 1 );

	/* for (int i = 0; i < HUMAN_BODYPARTS; i++) 
		{
			cl_entity_t &BPEnt = m_BodyParts[i];
			int PlayerBody = Item.Armor_GetBody( i );
			if( PlayerBody > -1 )
				CL_SetModelBody( BPEnt, 0, PlayerBody );
			else
				BPEnt.curstate.number = 0;	//Don't render this bodypart
		}*/
	//	}

	CLFrameShowModel(ItemEnt);
}

// Render player model on HUD
// It's a permanent 3D inset

void CRenderPlayerInset::Render()
{
	//return;
	if (!player.pev || MSCLGlobals::CamThirdPerson || !ShowHUD() || EngineFunc::CVAR_GetFloat("ms_lildude") == 0)
		return; //Thothie - ms_lildude line

	cl_entity_t *clplayer = gEngfuncs.GetLocalPlayer();
	if (!clplayer)
		return;

	//m_Ent.player = 0;
	m_Ent.model = clplayer->model;
	m_Ent.index = MSGlobals::ClEntities[CLPERMENT_INSET];
	m_Ent.curstate.number = MSGlobals::ClEntities[CLPERMENT_INSET];

	m_Ent.curstate.scale = INSET_SCALE;

	//Thothie DEC2010_16 - new female model, again - this will always render male though
	//no way to get char gender from here ATM
	/*
	m_Ent.SetBody(0,1);
	m_Ent.SetBody(1,1);
	m_Ent.SetBody(2,1);
	m_Ent.SetBody(3,1);
	*/

	//Thothie FEB2011_22 - the fuck there ain't...
	if (EngineFunc::CVAR_GetFloat("ms_clgender") == 0)
	{
		//Print("Male Render\n");
		m_Ent.SetBody(0, 1);
		m_Ent.SetBody(1, 1);
		m_Ent.SetBody(2, 1);
		m_Ent.SetBody(3, 1);
	}
	else
	{
		//Print("Female Render\n");
		m_Ent.SetBody(0, 2);
		m_Ent.SetBody(1, 2);
		m_Ent.SetBody(2, 2);
		m_Ent.SetBody(3, 2);
	}
	//m_Ent.index = clplayer->index;
	//m_Ent.curstate.number = clplayer->index;

	/* for (int i = 0; i < 4; i++) 
		m_Ent.latched.prevcontroller[i] = m_Ent.curstate.controller[i] = 127;
 	m_Ent.curstate.gaitsequence = 0;
	m_Ent.curstate.framerate = 1;
	 for (int i = 0; i < 4; i++) 
		m_Ent.attachment[i] = clplayer->attachment[i];

	ClearBits( m_Ent.curstate.colormap, MSRDR_ANIM_ONCE );

	if( clplayer->curstate.sequence == ANIM_RUN ) //FBitSet(m_StatusFlags,PLAYER_MOVE_RUNNING)
		m_Ent.curstate.sequence = ANIM_RUN;					//Run anim
  	else if( clplayer->curstate.sequence == ANIM_SIT )
	{
		if( m_Ent.curstate.sequence != ANIM_SIT )
		{
			m_Ent.curstate.sequence = ANIM_SIT;				//Sit anim
			m_Ent.curstate.animtime = gpGlobals->time;
		}
		SetBits( m_Ent.curstate.colormap, MSRDR_ANIM_ONCE );
	}
	else if( clplayer->curstate.sequence == ANIM_CROUCH || clplayer->curstate.sequence == ANIM_CRAWL )
	{
		m_Ent.curstate.sequence = clplayer->curstate.sequence;				//Crouch anim
	}
	else if( clplayer->curstate.gaitsequence == ANIM_CRAWL )
	{
		if( player.pev->velocity.Length2D() )
		//if( player->curstate.velocity.Length2D( ) )
			m_Ent.curstate.sequence = clplayer->curstate.gaitsequence;		//Crouch anim (while attacking)
		else
			m_Ent.curstate.sequence = ANIM_CROUCH;
	}
	else m_Ent.curstate.sequence = 2;								//Default Idle*/

	SetBits(m_Ent.curstate.colormap, MSRDR_ASPLAYER | MSRDR_COPYPLAYER | MSRDR_GLOW_RED);
	SetBits(m_Ent.curstate.oldbuttons, MSRDR_NOREFLECT);
	CRenderPlayer::Render();

	//Thothie JAN2010_12 - commenting this out to play nice with new ref model
	//m_Ent.SetBody( HBP_LEGS, 1 );			//Don't show legs at all
}

void CRenderPlayerInset::RenderGearItem(CGenericItem &Item)
{
	if (Item.IsWorn())
	{
		//Item is only worn on the legs - don't show it at all
		if (Item.m_WearModelPositions.size() == 1 && Item.m_WearModelPositions[0] == HBP_LEGS)
			return;
	}

	cl_entity_t &ItemEnt = GearItemEntity(Item);

	ItemEnt = Item.m_ClEntity[CGenericItem::ITEMENT_NORMAL]; //Copy the normal ent, then modify the copy for the inset

	ItemEnt.curstate.scale = INSET_SCALE;

	SetBits(ItemEnt.curstate.oldbuttons, MSRDR_NOREFLECT);
	SetBits(ItemEnt.curstate.colormap, MSRDR_GLOW_RED);
	if (Item.m_Location == ITEMPOS_HANDS && player.m_CurrentHand == Item.m_Hand)
	{
		ClearBits(ItemEnt.curstate.colormap, MSRDR_GLOW_RED);
		SetBits(ItemEnt.curstate.colormap, MSRDR_GLOW_GRN);
	}
	else
	{
		ClearBits(ItemEnt.curstate.colormap, MSRDR_GLOW_GRN);
	}

	CRenderPlayer::RenderGearItem(Item);
}
