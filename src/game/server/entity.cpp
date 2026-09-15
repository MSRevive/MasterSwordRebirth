#include "Platform.h"
#include "msdllheaders.h"
#include "msitemdefs.h"

//MS-specific entity functions

/*void CBaseEntity::StruckSound( CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, TraceResult *ptr, int bitsDamageType )
{
	//Struck sound for the world.  Makes no sound, but runs "hitwall" in the item's script
	if( !pInflictor ) return;

	CBaseEntity *pSoundEnt = GetClassPtr( (CBaseEntity *)NULL );
	pSoundEnt->pev->origin = ptr->vecEndPos;
	pSoundEnt->pev->movetype = MOVETYPE_FLY;
	UTIL_SetOrigin( pSoundEnt->pev, pSoundEnt->pev->origin );
	pSoundEnt->pev->flags |= FL_KILLME;

	//pInflicter->CounterEffect( this, CE_HITWORLD, pSoundEnt );
}*/
void CBaseEntity::CounterEffect(CBaseEntity *pInflictor, int iEffect, void *pExtraData)
{
	if (iEffect == CE_HITWORLD)
		return;

	//Let the object know it hit a wall
	pInflictor->CounterEffect(this, CE_HITWORLD, NULL);
}

void CBaseEntity::SetFollow(CBaseEntity *pTarget, int Flags)
{
	if (pTarget)
	{
		//Special follow type.  Follows and rotates with the entity while offset by a certain amount
		float Distance = (pev->origin - pTarget->pev->origin).Length();
		Vector RelAngles = UTIL_VecToAngles(pev->origin - pTarget->pev->origin) - pTarget->pev->angles;
		RelAngles.x *= -1;
		UTIL_MakeVectorsPrivate(RelAngles, pev->vuser1, NULL, NULL);
		pev->vuser1 *= Distance;
		pev->angles -= pTarget->pev->angles;
		pev->angles.x *= -1;
		pev->owner = pTarget->edict();
		SetBits(pev->playerclass, ENT_EFFECT_FOLLOW_ROTATE | Flags);
		m_AttachToEnt = pTarget;
	}
	else
	{
		CBaseEntity *pOwner = m_AttachToEnt.Entity();
		if (pOwner && pOwner->IsAlive())
			pev->angles += pOwner->pev->angles; //Update my angles

		pev->vuser1 = g_vecZero;
		pev->owner = NULL;
		ClearBits(pev->playerclass, ENT_EFFECT_FOLLOW_ROTATE);
		m_AttachToEnt = NULL;
	}
}

bool CBaseEntity::CanDamage(CBaseEntity *pOther)
{
	return (pOther->pev->takedamage > DAMAGE_NO) &&	   //The entity can take damage
		   (!FBitSet(pOther->pev->flags, FL_GODMODE)); //The entity is not invulnerable;
}

void CBaseEntity::SUB_FadeOut(float FadeDuration)
{
	m_FadeDuration = FadeDuration;
	m_TimeFadeStart = gpGlobals->time;
	SetThink(&CBaseEntity::Think_FadeOut);
	pev->nextthink = gpGlobals->time + 0.1f;
}

void CBaseEntity::Think_FadeOut(void)
{
	float Duration = gpGlobals->time - m_TimeFadeStart;
	if (Duration >= m_FadeDuration)
	{
		pev->renderamt = 0;
		SUB_Remove();
		return;
	}

	float FadeRatio = Duration / m_FadeDuration;

	FadeRatio = V_max(FadeRatio, 0.0f);
	FadeRatio = V_min(FadeRatio, 1.0f);

	pev->renderamt = 255 * FadeRatio;
	pev->nextthink = gpGlobals->time + 0.1f;
}
