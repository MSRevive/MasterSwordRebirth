#include "inc_weapondefs.h"
#include "script.h"
#include "mseffects.h"
#include "shake.h"
#include "logfile.h"
//#include "monsters/bodyparts/bodyparts.h"

void UTIL_ScreenFadeBuild(ScreenFade &fade, const Vector &color, float fadeTime, float fadeHold, int alpha, int flags);
void UTIL_ScreenFadeWrite(const ScreenFade &fade, CBaseEntity *pEntity);

void CPortal ::Spawn2(void)
{
	pev->classname = MAKE_STRING("portal");
	SET_MODEL(edict(), "models/null.mdl");
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT;
	pev->effects &= ~EF_NODRAW;
	pev->effects = EF_BRIGHTFIELD | EF_LIGHT;
	pev->velocity = Vector(0, 0, 10);
	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	SetThink(&CPortal::CloseSpawnPortal);
	pev->nextthink = gpGlobals->time + 15.0;
}
void CPortal ::CloseSpawnPortal(void)
{
	SUB_Remove();
	//	pev->effects &= ~EF_BRIGHTFIELD;
	//	pev->effects &= ~EF_LIGHT;
}
//LINK_ENTITY_TO_CLASS( portal, CPortal );

void CMSSprite ::TorchInit(char *pszName, float flFramerate, float flScale, edict_t *peOwner)
{
	pev->owner = peOwner;
	if (!pev->owner)
		return;

	//A torch sprite
	SpriteInit(pszName, Vector(-4095, -4095, -4095));
	pev->classname = MAKE_STRING("env_sprite");
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	SetTransparency(kRenderTransAdd, 0, 0, 0, 255, kRenderFxNone);
	pev->framerate = flFramerate;
	pev->scale = flScale;
	SpriteType = SPRITE_TORCH;
	TurnOn();
	Think();
}
void CMSSprite ::Think()
{
	CBaseEntity::Think();

	if (SpriteType == SPRITE_TORCH)
	{
		CGenericItem *pTorch = (CGenericItem *)MSInstance(pev->owner);
		if (!pTorch || !FBitSet(pTorch->MSProperties(), ITEM_GENERIC))
		{
			SUB_Remove();
			//UTIL_Remove(this);
			return;
		}

		CGenericItem *pGenTorch = (CGenericItem *)pTorch;
		CBaseEntity *pTorchOwner = (CBaseEntity *)pTorch->m_pOwner;

		if (FBitSet(pTorch->pev->effects, EF_NODRAW) ||
			(!pTorchOwner ? pTorch->pev->waterlevel > 2 : pTorchOwner->pev->waterlevel > 2))
			SetBits(pev->effects, EF_NODRAW);
		else
		{
			ClearBits(pev->effects, EF_NODRAW);

			if (pTorchOwner)
			{
				//UTIL_SetOrigin( pev, pTorchOwner->pev->origin );
				//ALERT( at_console, "%i\n", pev->body );
				SetAttachment(pTorchOwner->edict(), pGenTorch->m_Hand + 1);
			}
			else
			{
				//Following commented line doesn't work for non-players!
				//SetAttachment( pTorch->edict(), 1 );
				//logfile << "2a\r\n";
				Vector vOfsOrigin, vTemp;

				//Be sure to check model, this function needs it
				if (pTorch->pev->model)
					GET_ATTACHMENT(pTorch->edict(), 0, vOfsOrigin, vTemp);
				UTIL_SetOrigin(pev, vOfsOrigin);
				pev->angles = vTemp;
				pev->aiment = NULL;
				pev->movetype = MOVETYPE_FLY;
				pev->body = 0;
				pev->skin = 0;
			}
		}

		pev->nextthink = gpGlobals->time;
	}
}

LINK_ENTITY_TO_CLASS(ms_sprite, CMSSprite);

void CTorchLight ::Create(float flDuration, edict_t *peOwner)
{
	CTorchLight *pLight = GetClassPtr((CTorchLight *)NULL);
	if (!pLight)
		return;
	pLight->pev->owner = peOwner;
	pLight->m_OwnerID = (ulong)MSInstance(pLight->pev->owner);
	pLight->Spawn();
}
void CTorchLight ::Spawn()
{
	for (int i = 0; i < TORCH_LIGHTS; i++)
	{
		pLight[i] = GetClassPtr((CBaseEntity *)NULL);
		SET_MODEL(pLight[i]->edict(), "models/null.mdl");
		pLight[i]->pev->renderamt = 0;
		pLight[i]->pev->rendermode = kRenderTransTexture;
		pLight[i]->pev->effects = EF_DIMLIGHT;
		//pLight[i]->pev->flags = FL_DORMANT;
	}
	Think();
}
void CTorchLight ::Think()
{
	CBasePlayerItem *pTorch = (CBasePlayerItem *)MSInstance(pev->owner);
	if (!pTorch || (ulong)pTorch != m_OwnerID)
	{
		SUB_Remove();
		return;
	}

	CBaseEntity *pTorchOwner = (CBaseEntity *)pTorch->m_pOwner,
				*pTorchTouse = pTorchOwner ? pTorchOwner : pTorch;

	TraceResult tr;

	if (pTorchOwner)
		UTIL_MakeVectors(pTorchOwner->pev->angles);

	if (FBitSet(pTorchTouse->pev->effects, EF_NODRAW) ||
		pTorchTouse->pev->waterlevel > 2)
		for (int i = 0; i < TORCH_LIGHTS; i++)
			SetBits(pLight[i]->pev->effects, EF_NODRAW);
	else
		for (int i = 0; i < TORCH_LIGHTS; i++)
			ClearBits(pLight[i]->pev->effects, EF_NODRAW);

	for (int i = 0; i < TORCH_LIGHTS; i++)
	{
		if (pLight[i])
		{
			if (pTorchOwner)
				pLight[i]->pev->origin = pTorchOwner->Center() + gpGlobals->v_right * 20 * (i ? -1 : 1) + gpGlobals->v_forward * 40 + Vector(0, 0, 16);
			pLight[i]->pev->origin = pTorch->Center() + Vector(0, 0, 4);
			UTIL_TraceLine(pTorchTouse->Center(), pLight[i]->pev->origin, ignore_monsters, pTorchTouse->edict(), &tr);
			pLight[i]->pev->origin = tr.vecEndPos;
			UTIL_SetOrigin(pLight[i]->pev, pLight[i]->pev->origin);
		}
	}
	pev->nextthink = gpGlobals->time;
}
void CTorchLight ::SUB_Remove()
{
	for (int i = 0; i < TORCH_LIGHTS; i++)
		pLight[i]->SUB_Remove();
	CBaseEntity ::SUB_Remove();
}
LINK_ENTITY_TO_CLASS(ms_torchlight, CTorchLight);

#include "player.h"

class CChangePlayerSpeed : public CBaseDelay
{
public:
	void Spawn(void);
	void Think(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	int Players;
	float m_Duration;

	struct playerinfo_t
	{
		edict_t *pePlayer;
		float OldSpeed, TimeResetSpeed;
	} m_PlayerInfo[256];
};

class CResetPlayerSpeed : public CBaseDelay
{
public:
	void Spawn(void)
	{
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_FLY;
		pev->effects = EF_NODRAW;
	}

	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
	{
		CBasePlayer *pPlayer = NULL;

		if (pActivator && pActivator->IsPlayer())
			pPlayer = (CBasePlayer *)pActivator;
		else if (pCaller && pCaller->IsPlayer())
			pPlayer = (CBasePlayer *)pCaller;

		if (pPlayer)
			pPlayer->pev->maxspeed = 0;
	}
};

LINK_ENTITY_TO_CLASS(mstrig_setplayerspeed, CChangePlayerSpeed);
LINK_ENTITY_TO_CLASS(mstrig_resetplayerspeed, CResetPlayerSpeed);

void CChangePlayerSpeed ::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_FLY;
	pev->effects = EF_NODRAW;
	pev->nextthink = gpGlobals->time + 0.1;
	ZeroMemory(&m_PlayerInfo, sizeof(playerinfo_t) * 256);
}

void CChangePlayerSpeed ::Think(void)
{

	for (int i = 0; i < 256; i++)
	{
		playerinfo_t *pInfo = &m_PlayerInfo[i];
		if (!pInfo->pePlayer)
			continue;

		if (!ENTINDEX(pInfo->pePlayer) || !GET_PRIVATE(pInfo->pePlayer))
		{
			pInfo->pePlayer = NULL;
			continue;
		}

		if (gpGlobals->time < pInfo->TimeResetSpeed)
			continue;

		CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pInfo->pePlayer);
		pPlayer->pev->maxspeed = pInfo->OldSpeed;
		pInfo->pePlayer = NULL;
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

void CChangePlayerSpeed ::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	CBasePlayer *pPlayer = NULL;

	if (pActivator && pActivator->IsPlayer())
		pPlayer = (CBasePlayer *)pActivator;
	else if (pCaller && pCaller->IsPlayer())
		pPlayer = (CBasePlayer *)pCaller;

	if (!pPlayer)
		return;

	playerinfo_t *pInfo = NULL;
	for (int i = 0; i < 256; i++)
	{
		if (!m_PlayerInfo[i].pePlayer)
		{
			pInfo = &m_PlayerInfo[i];
			break;
		}
	}

	if (!pInfo)
		return;

	pInfo->pePlayer = pPlayer->edict();
	pInfo->OldSpeed = pPlayer->pev->speed;
	pPlayer->pev->maxspeed = pev->speed;
	if (m_Duration >= 0)
		pInfo->TimeResetSpeed = gpGlobals->time + m_Duration;
	else
		pInfo->TimeResetSpeed = -1;
}

void CChangePlayerSpeed ::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "duration"))
	{
		m_Duration = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

//Makes entities glow for a time
CEntGlow *CEntGlow::Create(CBaseEntity *pTarget, Vector Color, float Amount, float Duration, float FadeDuration)
{
	CEntGlow *pGlowEnt = (CEntGlow *)GetClassPtr((CEntGlow *)NULL);
	pGlowEnt->m_Target = pTarget;
	pGlowEnt->m_Color = Color;
	pGlowEnt->m_Amount = Amount;
	pGlowEnt->m_CurentAmount = pGlowEnt->m_Amount;
	pGlowEnt->m_Duration = Duration;
	pGlowEnt->m_FadeDuration = FadeDuration;
	pGlowEnt->m_StartTime = gpGlobals->time;
	pGlowEnt->SetGlow(true);
	pGlowEnt->pev->nextthink = gpGlobals->time;

	return pGlowEnt;
}
void CEntGlow::SetGlow(bool On)
{
	if (!(bool)m_Target)
		return;

	CBaseEntity *pEntity = m_Target.Entity();
	if (!pEntity)
		return;

	if (On)
	{
		/*if( pEntity->IsPlayer( ) )
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pEntity;
			if( pPlayer->Body )
			{
				pPlayer->Body->Set( BPS_RDRFX, (void *)kRenderFxGlowShell );
				pPlayer->Body->Set( BPS_RDRCOLOR, &m_Color );
				pPlayer->Body->Set( BPS_RDRAMT, &m_CurentAmount );
			}
		}
		else*/
		{
			pEntity->pev->renderfx = kRenderFxGlowShell;
			pEntity->pev->rendercolor = m_Color;
			pEntity->pev->renderamt = m_CurentAmount;
		}
	}
	else
	{
		if (pEntity->IsPlayer())
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pEntity;
			if (pPlayer->Body)
				pPlayer->Body->Set(BPS_RDRFX, (void *)kRenderFxNone);
		}

		pEntity->pev->renderfx = kRenderFxNone;
	}
}

//MAR2008a - Rolled back to JAN2008a
//- have determined glow shell optimization is not a cause of monster gibbing
void CEntGlow::Think()
{
	if (!(bool)m_Target)
		return;

	//MiB DEC2007a - remove glow shell on death
	if (gpGlobals->time >= m_StartTime + m_Duration || !m_Target.pvPrivData->IsAlive())
	{
		SetGlow(false);
		SUB_Remove();
		return;
	}

	float FadeStartTime = (m_StartTime + m_Duration) - m_FadeDuration;
	if (gpGlobals->time >= FadeStartTime)
	{
		float FadeTime = gpGlobals->time - FadeStartTime;
		float FadeAmt = FadeTime / m_FadeDuration;
		if (FadeAmt < 0)
			FadeAmt = 0;
		if (FadeAmt > 1)
			FadeAmt = 1;

		m_CurentAmount = m_Amount * (1 - FadeAmt);
		SetGlow(true);
		pev->nextthink = gpGlobals->time; //MiB
	}
	else //MiB
		pev->nextthink = min(gpGlobals->time + 6, min(FadeStartTime, m_StartTime + m_Duration));
}

//
// Changelevel
//

class CMSChangeLevel : public CBaseEntity
{
	string_t sDestMap, sDestTrans;
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
	{
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
			if (!pPlayer)
				continue;

			//Let the client know they can now change levels
			MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_CLDLLFUNC], NULL, pPlayer->pev);
			WRITE_BYTE(3);
			WRITE_BYTE(0);
			WRITE_BYTE(0);
			WRITE_STRING(STRING(sDestMap));
			WRITE_STRING(pPlayer->m_OldTransition);
			WRITE_STRING(STRING(sDestTrans));
			MESSAGE_END();
		}
		pev->nextthink = gpGlobals->time + 0.2;
		SetThink(&CMSChangeLevel::ChangeLevel);
	}
	void ChangeLevel()
	{
		//MAR2008a - Thothie - Let game master handle mstrig_changelevel level changes
		//- original: CHANGE_LEVEL( (char *)STRING(sDestMap), NULL );
		CBaseEntity *pGameMasterEnt = UTIL_FindEntityByString(NULL, "netname", msstring("�") + "game_master");
		IScripted *pGMScript = pGameMasterEnt->GetScripted();

		msstringlist Parameters;
		Parameters.add(STRING(sDestMap));
		Parameters.add(STRING(sDestTrans));
		pGMScript->CallScriptEvent("gm_manual_map_change", &Parameters);
	}
	void KeyValue(KeyValueData *pkvd)
	{
		if (FStrEq(pkvd->szKeyName, "destmap"))
		{
			sDestMap = ALLOC_STRING(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else if (FStrEq(pkvd->szKeyName, "desttrans"))
		{
			sDestTrans = ALLOC_STRING(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else
			CBaseEntity::KeyValue(pkvd);
	}
};
LINK_ENTITY_TO_CLASS(mstrig_changelevel, CMSChangeLevel);

#define REQPARAMS(a)                                                                                                 \
	if (Params.size() < a)                                                                                           \
	{                                                                                                                \
		ALERT(at_console, "Script: %s, effect '%s' missing parameters!\n", m.ScriptFile.c_str(), Params[0].c_str()); \
		return;                                                                                                      \
	}

void CScript::ScriptedEffect(msstringlist &Params)
{
	startdbg;
	dbg("Begin");
	if (!Params.size())
	{
		ALERT(at_console, "Script: %s, effect missing parameters!\n", m.ScriptFile.c_str());
		return;
	}

	//Thothie attempting to find a method for edict control
	//this area affects server-side fx
	//msstringlist Parameters;
	//Parameters.add( UTIL_VarArgs("%s",Params[0]) );
	//Thothie JUN2007a - axing ms_fxlimit due to ineffectiveness
	/*float fxlimiter = 0;
	fxlimiter = CVAR_GET_FLOAT("ms_fxlimit");
	CBasePlayer *pOtherPlayer = (CBasePlayer *)UTIL_PlayerByIndex( 1 ); //running this on the index#1 player SUX - but I can't figure a way to run it on the global from here
	if ( fxlimiter > 0 ) pOtherPlayer->CallScriptEvent( "game_fx_spawn" ); 
	//CallScriptEventAll( "game_fx_spawn" );
	int fxfloodlevel = 0;
	fxfloodlevel = atoi(GetVar( "G_TOTAL_FX" ));
	if ( fxfloodlevel > fxlimiter && fxlimiter > 0 )
	{
		ALERT( at_console, UTIL_VarArgs("Some FX not displayed - Total Flood Level: %i/%f", fxfloodlevel, fxlimiter) );
		return;
	}*/

	//[/Thothie]

	dbg(Params[0]);

	//Thothie MAR2008a - major changes to beam
	//- beam ents now works (never could get it to work before), also changed syntax (see below)
	//- beams created with beam ents can be changed later with beam update (see below)

	if (Params[0] == "beam")
	{
		REQPARAMS(4);
		int Type = 0;
		if (Params[1] == "end")
			Type = 0;
		else if (Params[1] == "point")
			Type = 1;
		else if (Params[1] == "ents")
			Type = 2;
		else if (Params[1] == "vector")
			Type = 3;
		else if (Params[1] == "update")
		{
			//       0    1      2         3          4	      5
			//effect beam update <beam_id> <property> <value> [option]
			//MSMonster *pMonster = (CMSMonster *)pEnt;
			CBeam *pBeam = (CBeam *)m.pScriptedEnt->RetrieveEntity(Params[2]);

			if (pBeam)
			{
				if (Params[3] == "color")
				{
					Vector Color = StringToVec(Params[4]);
					pBeam->SetColor(Color.x, Color.y, Color.z);
				}
				else if (Params[3] == "end_target")
				{
					CBaseEntity *pTargetNew = m.pScriptedEnt->RetrieveEntity(Params[4]);
					if (pTargetNew)
					{
						pBeam->SetEndEntity(pTargetNew->entindex());
						if (Params.size() >= 6)
							pBeam->SetEndAttachment(atoi(Params[5]));
					}
				}
				else if (Params[3] == "start_target")
				{
					CBaseEntity *pTargetNew = m.pScriptedEnt->RetrieveEntity(Params[4]);
					if (pTargetNew)
					{
						pBeam->SetStartEntity(pTargetNew->entindex());
						if (Params.size() >= 6)
							pBeam->SetStartAttachment(atoi(Params[5]));
					}
				}
				else if (Params[3] == "end_origin") //untested
				{
					Vector vec = StringToVec(Params[4]);
					pBeam->SetEndPos(vec);
					pBeam->RelinkBeam();
				}
				else if (Params[3] == "start_origin") //untested
				{
					Vector vec = StringToVec(Params[4]);
					pBeam->SetStartPos(vec);
					pBeam->RelinkBeam();
				}
				else if (Params[3] == "points")
				{
					pBeam->PointsInit(StringToVec(Params[4]), StringToVec(Params[5]));
				}
				else if (Params[3] == "brightness")
				{
					pBeam->SetBrightness(atoi(Params[4]));
				}
				else if (Params[3] == "noise")
				{
					pBeam->SetNoise(atoi(Params[4]));
				}
				else if (Params[3] == "remove")
				{
					pBeam->SetThink(&CBaseEntity::SUB_Remove);
					int rem_delay = atof(Params[4]);
					if (rem_delay == 0)
						pBeam->pev->nextthink = gpGlobals->time + 0.1;
					else
						pBeam->pev->nextthink = gpGlobals->time + rem_delay;
				}
			}
			//Might neeed pBeam->RelinkBeam
			return;
		}
		//Thothie AUG2013_17 - ye old follow beam
		else if (Params[1] == "follow")
		{
			//beam follow 2<model> 3<target> 4<attach> 5<width> 6<life> 7<brightness> 8<color>
			int Modelindex = MODEL_INDEX(msstring("sprites/") + Params[2]);
			if (!Modelindex)
			{
				ALERT(at_console, "Script: %s, effect %s %s. Sprite/Model: %s not found!!\n", m.ScriptFile.c_str(), Params[0].c_str(), Params[1].c_str(), Params[2].c_str());
				return;
			}

			CBaseEntity *pTarget = m.pScriptedEnt->RetrieveEntity(Params[3]);
			if (!pTarget)
			{
				ALERT(at_console, "Script: %s, effect beam follow target not found.\n", m.ScriptFile.c_str());
				return;
			}

			Vector Color = StringToVec(Params[8]);

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY);

			WRITE_BYTE(TE_BEAMFOLLOW);
			//h4x
			if (Params[4] == "0")
				WRITE_SHORT(pTarget->entindex() | 0x0000);
			else if (Params[4] == "1")
				WRITE_SHORT(pTarget->entindex() | 0x1000);
			else if (Params[4] == "2")
				WRITE_SHORT(pTarget->entindex() | 0x2000);
			else if (Params[4] == "3")
				WRITE_SHORT(pTarget->entindex() | 0x3000);
			else if (Params[4] == "4")
				WRITE_SHORT(pTarget->entindex() | 0x4000);

			WRITE_SHORT(Modelindex);	 // model
			WRITE_BYTE(atof(Params[6])); // life
			WRITE_BYTE(atoi(Params[5])); // width
			WRITE_BYTE(Color.x);		 // r, g, b
			WRITE_BYTE(Color.y);		 // r, g, b
			WRITE_BYTE(Color.z);		 // r, g, b
			WRITE_BYTE(atoi(Params[7])); // brightness

			MESSAGE_END();

			return;
		}

		if (Type != 1 && !m.pScriptedEnt)
			return; //'end' and 'ents' need an owner for entity lookups.  'points' doesn't need one

		//CBeam &Beam = *CBeam::BeamCreate( msstring("sprites/") + Params[2], atoi(Params[3]) );
		CBeam *Beam = GetClassPtr((CBeam *)NULL);
		Beam->pev->classname = MAKE_STRING("beam");
		Beam->BeamInit(msstring("sprites/") + Params[2], atoi(Params[3]));

		int NextParam = 4;
		if (!Type)
		{
			Vector Start(StringToVec(Params[NextParam++]));

			CBaseEntity *pTarget = m.pScriptedEnt->RetrieveEntity(Params[NextParam++]);
			if (!pTarget)
				return;
			Beam->PointEntInit(Start, pTarget->entindex());
			Beam->SetEndAttachment(atoi(Params[NextParam++]));
		}
		else if (Type == 1)
		{
			int StartPoint = NextParam++;
			Beam->PointsInit(StringToVec(Params[StartPoint]), StringToVec(Params[NextParam++])); //Can't use NextParam++ twice
		}
		else if (Type == 2)
		{
			//CBaseEntity *pOtherEntity = m.pScriptedEnt->IsMSMonster() ? ((CMSMonster *)m.pScriptedEnt)->RetrieveEntity( Params[NextParam++] ) : NULL;
			/*CBaseEntity *pOtherEntity = m.pScriptedEnt->RetrieveEntity( Params[NextParam++] );
			if( !pOtherEntity )
			{
				ALERT( at_console, "Script: %s, effect beam ent got NULL for entity: %s!\n", m.ScriptFile.c_str(), Params[NextParam-1] );
				return;
			}
			Beam.SetEndAttachment( atoi(Params[NextParam++]) );*/
			//This is never used properly anywhere in the scripts, so changing format
			//beam ents <spritename> <width> <target1> <attach_idx1> <target2> <attach_idx2> <(r,g,b)> <brightness> <noise> <duration>
			//entity beam
			CBaseEntity *pTargetStart = m.pScriptedEnt->RetrieveEntity(Params[NextParam++]);
			int start_attach = atoi(Params[NextParam++]);
			CBaseEntity *pTargetEnd = m.pScriptedEnt->RetrieveEntity(Params[NextParam++]);
			int end_attach = atoi(Params[NextParam++]);

			if (!pTargetStart || !pTargetEnd)
			{
				ALERT(at_console, "Script: %s, effect beam ent got NULL for entity: %s!\n", m.ScriptFile.c_str(), Params[NextParam - 1]);
				return;
			}

			m.pScriptedEnt->StoreEntity(Beam, ENT_LASTCREATED);
			SetVar("G_LAST_BEAM_ID", EntToString(Beam), true);

			Beam->EntsInit(pTargetStart->entindex(), pTargetEnd->entindex());
			Beam->SetStartAttachment(start_attach);
			Beam->SetEndAttachment(end_attach);
		}
		else if (Type == 3)
		{
			//effect beam vector <spritename> <width> <origin> <origin> <(r,g,b)> <brightness> <noise> <duration>
			int StartPoint = NextParam++;
			Beam->PointsInit(StringToVec(Params[StartPoint]), StringToVec(Params[NextParam++])); //Can't use NextParam++ twice

			m.pScriptedEnt->StoreEntity(Beam, ENT_LASTCREATED);
			SetVar("G_LAST_BEAM_ID", EntToString(Beam), true);
		}

		Vector Color = StringToVec(Params[NextParam++]);
		Beam->SetColor(Color.x, Color.y, Color.z);
		Beam->SetBrightness(atoi(Params[NextParam++]));
		Beam->SetNoise(atoi(Params[NextParam++]));

		float remove_delay = atof(Params[NextParam]);
		if (remove_delay > 0)
			Beam->SetThink(&CBaseEntity::SUB_Remove);
		Beam->pev->nextthink = gpGlobals->time + atof(Params[NextParam]);
	}
	else if (Params[0] == "tempent")
	{
		int Type = 0;
		if (Params[1] == "gibs")
		{
			Type = 0;
			REQPARAMS(9);
		}
		else if (Params[1] == "spray")
		{
			Type = 1;
			REQPARAMS(8);
		}
		else if (Params[1] == "trail")
		{
			Type = 2;
			REQPARAMS(10);
		}

		msstring_ref ModelName = GetFullResourceName(Params[2]);
		int Modelindex = MODEL_INDEX(ModelName);
		if (!Modelindex)
		{
			ALERT(at_console, "Script: %s, effect %s %s. Sprite/Model: %s not found!!\n", m.ScriptFile.c_str(), Params[0].c_str(), Params[1].c_str(), Params[2].c_str());
			return;
		}

		if (Type == 0)
		{
			Vector Position = StringToVec(Params[3]);
			Vector Size = StringToVec(Params[4]);
			Vector Velocity = StringToVec(Params[5]);
			int Random = atoi(Params[6]);
			int Amount = atoi(Params[7]);
			float Duration = atof(Params[8]);
			int Flags = TEFIRE_FLAG_ALPHA;

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m.pScriptedEnt->pev->origin);
			WRITE_BYTE(TE_BREAKMODEL);

			// position
			WRITE_COORD(Position.x);
			WRITE_COORD(Position.y);
			WRITE_COORD(Position.z);

			// size
			WRITE_COORD(Size.x);
			WRITE_COORD(Size.y);
			WRITE_COORD(Size.z);

			// velocity
			WRITE_COORD(Velocity.x);
			WRITE_COORD(Velocity.y);
			WRITE_COORD(Velocity.z);

			// randomization
			WRITE_BYTE(Random);

			// Model
			WRITE_SHORT(Modelindex); //model id#

			// # of shards
			WRITE_BYTE(Amount); // let client decide

			// duration
			WRITE_BYTE(Duration); // 2.5 seconds

			// flags
			WRITE_BYTE(Flags);
			MESSAGE_END();
		}
		else if (Type == 1)
		{
			Vector Position = StringToVec(Params[3]);
			Vector Direction = StringToVec(Params[4]);
			int Count = atoi(Params[5]);
			int Speed = atoi(Params[6]);
			int Noise = atoi(Params[7]);

			/*MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, m.pScriptedEnt->pev->origin );
				WRITE_BYTE( TE_SPRITE_SPRAY );
				WRITE_COORD( Position.x);	// pos
				WRITE_COORD( Position.y);	
				WRITE_COORD( Position.z);	
				WRITE_COORD( Direction.x);	// dir
				WRITE_COORD( Direction.y);	
				WRITE_COORD( Direction.z);	
				WRITE_SHORT( Modelindex );	// model
				WRITE_BYTE ( Count );			// count
				WRITE_BYTE ( Speed );			// speed
				WRITE_BYTE ( Noise );			// noise ( client will divide by 100 )
			MESSAGE_END();*/
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m.pScriptedEnt->pev->origin);
			WRITE_BYTE(TE_SPRAY);
			WRITE_COORD(Position.x); // pos
			WRITE_COORD(Position.y);
			WRITE_COORD(Position.z);
			WRITE_COORD(Direction.x); // dir
			WRITE_COORD(Direction.y);
			WRITE_COORD(Direction.z);
			WRITE_SHORT(Modelindex);	   // model
			WRITE_BYTE(Count);			   // count
			WRITE_BYTE(Speed);			   // speed
			WRITE_BYTE(Noise);			   // noise ( client will divide by 100 )
			WRITE_BYTE(TEFIRE_FLAG_ALPHA); // rendermode
			MESSAGE_END();
		}
		else if (Type == 2)
		{
			Vector Position = StringToVec(Params[3]);
			Vector Destination = StringToVec(Params[4]);
			int Count = atoi(Params[5]);
			int Life = atoi(Params[6]);
			int Scale = atoi(Params[7]);
			int Speed = atoi(Params[8]);
			int Random = atoi(Params[9]);

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m.pScriptedEnt->pev->origin);
			WRITE_BYTE(TE_SPRITETRAIL);
			WRITE_COORD(Position.x); // pos
			WRITE_COORD(Position.y);
			WRITE_COORD(Position.z);
			WRITE_COORD(Destination.x); // dest
			WRITE_COORD(Destination.y);
			WRITE_COORD(Destination.z);
			WRITE_SHORT(Modelindex); // sprite
			WRITE_BYTE(Count);		 // count
			WRITE_BYTE(Life);		 // (life in 0.1's)
			WRITE_BYTE(Scale);		 // (scale in 0.1's)
			WRITE_BYTE(Speed);		 // (velocity along vector in 10's)
			WRITE_BYTE(Random);		 // (randomness of velocity in 10's)
			MESSAGE_END();
		}
	}
	else if (Params[0] == "screenshake")
	{
		REQPARAMS(6);

		Vector Origin = StringToVec(Params[1]);
		float Amplitude = atof(Params[2]);
		float Frequency = atof(Params[3]);
		float Duration = atof(Params[4]);
		float Radius = atof(Params[5]);

		UTIL_ScreenShake(Origin, Amplitude, Frequency, Duration, Radius);
	}
	else if (Params[0] == "screenfade")
	{
		bool ApplyToAll = Params[1] == "all" ? true : false;
		CBaseEntity *pEntity = NULL;

		if (!ApplyToAll)
			pEntity = m.pScriptedEnt ? m.pScriptedEnt->RetrieveEntity(Params[1]) : NULL;

		if (ApplyToAll || (pEntity && pEntity->IsPlayer()))
		{
			Vector Color = StringToVec(Params[4]);

			int Flags = FFADE_IN;

			if (Params[6].find("fadein") != msstring_error)
				Flags |= FFADE_IN;
			if (Params[6].find("fadeout") != msstring_error)
				Flags |= FFADE_OUT;
			if (Params[6].find("noblend") != msstring_error)
				Flags |= FFADE_MODULATE;
			if (Params[6].find("perm") != msstring_error)
				Flags |= FFADE_STAYOUT;

			if (!ApplyToAll)
				UTIL_ScreenFade(pEntity, Color, atof(Params[2]), atof(Params[3]), atoi(Params[5]), Flags);
			else
				UTIL_ScreenFadeAll(Color, atof(Params[2]), atof(Params[3]), atoi(Params[5]), Flags);
		}
	}
	else if (Params[0] == "glow")
	{
		REQPARAMS(6);

		CBaseEntity *pTarget = m.pScriptedEnt->RetrieveEntity(Params[1]);
		if (!pTarget)
		{
			//This is common. Don't use a verbose error here. -- UNDONE
			ALERT(at_console, "Script: %s, effect glow.  Target %s not found!!\n", m.ScriptFile.c_str(), Params[1].c_str());
			return;
		}

		Vector Color = StringToVec(Params[2]);
		float Amount = atof(Params[3]);
		float Duration = atof(Params[4]);
		float FadeDuration = atof(Params[5]);

		CEntGlow *pGlowEnt = CEntGlow::Create(pTarget, Color, Amount, Duration, FadeDuration);

		if (Duration < 0)
			pGlowEnt->SUB_Remove(); //Glow forever
	}
	//Thothie MAR2011a - decal function
	//effect decal <origin> <decal_idx>
	//see DLL_DECALLIST gDecals[] in world.cpp for list of decal indexes
	//(scratch that, DLL_DECALLIST gDecals[] is either misnnamed or not used)
	else if (Params[0] == "decal")
	{
		Vector DestPos = StringToVec(Params[1]);
		int decalidx = atoi(Params[2]);

		int message;
		message = TE_WORLDDECAL;

		//Either there are no high decals loaded, or this no workie
		if (decalidx > 255)
		{
			message = TE_WORLDDECALHIGH;
			decalidx -= 256;
		}

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(message);
		WRITE_COORD(DestPos.x);
		WRITE_COORD(DestPos.y);
		WRITE_COORD(DestPos.z);
		WRITE_BYTE(decalidx);
		MESSAGE_END();

		Print("Decal: %i @ %s\n", decalidx, Params[1].c_str());
	}
	enddbg("CScript::ScriptedEffect()");
}
