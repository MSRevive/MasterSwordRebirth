#include "inc_weapondefs.h"
//#include "Bodyparts.h"
//#include "Bodyparts_Human.h" //Have to include this because of the armor inclusion below

void CBodypart::Initialize(CBaseEntity *pOwner, char *ModelName, int idx)
{
	SET_MODEL(edict(), ModelName);
	pev->rendermode = 0;
	pev->renderfx = 0;
	pev->renderamt = 0;
	pev->effects = 0;
	pev->body = 0;
	pev->skin = 0;
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_FOLLOW;
	pev->flags = FL_DORMANT | FL_SKIPLOCALHOST;
	if (pOwner)
	{
		pev->aiment = pOwner->edict();
		pev->owner = pOwner->edict();
	}
	m_Idx = idx;
	m_Visible = true;
}
CBodypart *CBodypart::Duplicate(CBodypart *pExistingBodypart)
{
	CBodypart &NewBodypart = pExistingBodypart ? *pExistingBodypart : *GetClassPtr((CBodypart *)NULL);
	NewBodypart.pev->model = pev->model;
	SET_MODEL(NewBodypart.edict(), STRING(NewBodypart.pev->model));
	NewBodypart.pev->flags = pev->flags & (~FL_SKIPLOCALHOST);
	NewBodypart.pev->rendermode = pev->rendermode;
	NewBodypart.pev->renderamt = pev->renderamt;
	NewBodypart.pev->renderfx = pev->renderfx;
	NewBodypart.pev->effects = pev->effects;
	NewBodypart.pev->body = pev->body; //Don't copy armor
	NewBodypart.pev->skin = pev->skin;
	NewBodypart.pev->solid = pev->solid;
	NewBodypart.pev->movetype = pev->movetype;
	NewBodypart.pev->aiment = pev->aiment;
	NewBodypart.pev->owner = pev->owner;
	NewBodypart.m_Visible = m_Visible;
	NewBodypart.m_Idx = m_Idx;
	return &NewBodypart;
}
/*float CBodypart::TraceAttack( int iLastHitGroup, CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType ) 
{
	CMSMonster *pOwner = (CMSMonster *)MSInstance( pev->owner );
	if( !pOwner ) return flDamage;

	 for (int i = 0; i < pOwner->Gear.size(); i++) 
	{
		CGenericItem *pItemWorn = pOwner->Gear[ i ];

		//Is this armor?
		if( !pItemWorn || !pItemWorn->ArmorData )
			continue;

		//Add in protection
		flDamage = pItemWorn->Armor_Protect( iLastHitGroup, pInflictor, pAttacker, flDamage, bitsDamageType );
	}

	return flDamage;
}*/
/*void *MSQuery( int iRequest )
{ 
	CMSMonster *pOwner = (CMSMonster *)MSInstance( pev->owner );
	if( !pOwner ) return flDamage;

	 for (int i = 0; i < pOwner->Gear.size(); i++) 
	{
		CGenericItem *Item = *pOwner->Gear[ i ];
		if( FBitSet(Item.MSProperties(),ITEM_ARMOR) )
		{
			int Setting = Item.Armor_GetBody( m_Idx );
			if( Setting < 0 ) return 0;
		}

	}

	return ENT_BODYPART;
}
*/
void CBodypart::Set(int iState, void *vData)
{
	switch (iState)
	{
	case BPS_OWNER:
		pev->aiment = ((CBaseEntity *)vData)->edict();
		pev->owner = ((CBaseEntity *)vData)->edict();
		break;
	case BPS_MODEL:
		SET_MODEL(edict(), (const char *)vData);
		break;
	case BPS_BODY:
		if ((int)vData < 0)
			SetBits(pev->effects, EF_NODRAW);
		else
		{
			ClearBits(pev->effects, EF_NODRAW);
			pev->body = (int)vData;
		}
		break;
		/*		case BPS_ARMOR:
			pArmor = (CGenericItem *)vData;
			break;*/
	case BPS_TRANS:
		pev->rendermode = kRenderTransTexture;
		pev->renderamt = *(float *)vData;
		break;
	case BPS_RDRNORM:
		pev->rendermode = kRenderNormal;
		pev->renderamt = 0;
		break;
	case BPS_RDRMODE:
		pev->rendermode = (int)vData;
		break;
	case BPS_RDRFX:
		pev->renderfx = (int)vData;
		break;
	case BPS_RDRAMT:
		pev->renderamt = *(float *)vData;
		break;
	case BPS_RDRCOLOR:
		pev->rendercolor = *(Vector *)vData;
		break;
	}
}
void CBaseBody::Set(int iState, void *vData)
{
	for (int i = 0; i < Bodyparts.size(); i++)
		Bodyparts[i]->Set(iState, vData);
}
void CBaseBody::Think(CMSMonster *pOwner)
{
	for (int b = 0; b < Bodyparts.size(); b++)
	{
		int Body = 0;
		bool Visible = true;
		for (int i = 0; i < pOwner->Gear.size(); i++)
		{
			if (pOwner->Gear[i]->m_Location <= ITEMPOS_HANDS)
				continue;

			int retbody = pOwner->Gear[i]->Armor_GetBody(b);
			if (retbody > Body)
				Body = retbody;
			if (retbody < 0)
				Visible = false;
		}

		if (Bodyparts[b]->pev->body != Body || Bodyparts[b]->m_Visible != Visible)
			Bodyparts[b]->Set(BPS_BODY, Visible ? (void *)Body : (void *)-1);

		Bodyparts[b]->m_Visible = Visible;
	}
}
CBaseBody *CBaseBody::Duplicate()
{
	CBaseBody &NewBody = *msnew(CBaseBody);
	for (int i = 0; i < Bodyparts.size(); i++)
		NewBody.Bodyparts[i] = Bodyparts[i]->Duplicate();
	return &NewBody;
}
void CBaseBody::Delete()
{
	for (int i = 0; i < Bodyparts.size(); i++)
	{
		Bodyparts[i]->SUB_Remove();
		Bodyparts.erase(i--);
	}
	delete this;
}