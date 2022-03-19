/***
*
*	Copyright (c) 2000, Kenneth "Dogg" Early.
*	
*	Email: kene@maverickdev.com or
*		   kearly@crosswinds.net
*
****/
//
//	NPC Script: Basically a scripted_sequence, but for MS
//  ����������
#include "msdllheaders.h"
#include "Monsters/msmonster.h"
#include "script.h"

enum
{
	SCRIPT_MOVE = 0,
	SCRIPT_PLAYANIM,
	SCRIPT_RUNEVENT,
	SCRIPT_MOVE_PLAYANIM,
	SCRIPT_MOVE_RUNEVENT,
};

class NPCScript : public CBaseEntity
{
	void Act(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void PlayAnim();
	void RunScriptEvent();
	void MoveThink();
	void AnimateThink();
	void Finish(bool fEarlyBreak)
	{
		m_EarlyBreak = true;
		Finish();
	}
	void Finish();
	void FireTarget();

	int m_iType;
	float m_flFireDelay;
	string_t m_sMoveAnim, m_sActionAnim, m_sFireWhenDone, m_sFireOnBreak, m_sEventName;
	entityinfo_t m_NPC;
	bool m_fStopAI;
	bool m_fShouldExpire; //duplicates of this npcscript have this set to true
	dest_t m_MoveDest;
	bool m_EarlyBreak;

	//Overridden
	void Spawn();
	void KeyValue(KeyValueData *pkvd);
};

LINK_ENTITY_TO_CLASS(ms_npcscript, NPCScript);
LINK_ENTITY_TO_CLASS(mstrig_act, NPCScript);

void NPCScript ::Spawn()
{
	SetBits(pev->effects, EF_NODRAW);
	SetUse(&NPCScript::Act);
	m_fShouldExpire = false;
}
void NPCScript ::Act(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	CMSMonster *pMonster = NULL;

	if (!m_fShouldExpire)
	{
		//Print("->!m_fShouldExpire");
		if (pActivator && pActivator->IsMSMonster() && (!pev->target || FStrEq(STRING(pActivator->pev->targetname), STRING(pev->target))))
		{
			//Print("->pActivator");
			pMonster = (CMSMonster *)pActivator;
		}
		else
		{
			pMonster = (CMSMonster *)UTIL_FindEntityByTargetname(NULL, STRING(pev->target));
			//Print("->UTIL_FindEntityByTargetname %s %s", STRING(pev->target), pMonster->DisplayName() );
			if (!pMonster || !FBitSet(pMonster->pev->flags, FL_MONSTER))
				return;
		}

		if (!pMonster->IsAlive())
		{
			//Print("->IsAlive()1");
			return;
		}

		//If busy, create a new npc_script and let the monster use that one...
		if (m_NPC.Entity())
		{
			//Print("->m_NPC.Entity()");
			//Calling monster must have the correct targetname
			if (pActivator && pActivator->MyMonsterPointer() &&
				FStrEq(STRING(pActivator->pev->targetname), STRING(pev->target)))
			{
				//Print("->Duplicating");
				NPCScript *pNewScript = GetClassPtr((NPCScript *)NULL);
				pNewScript->pev->origin = pev->origin;
				pNewScript->pev->angles = pev->angles;
				pNewScript->pev->target = pev->target;
				pNewScript->m_iType = m_iType;
				pNewScript->m_flFireDelay = m_flFireDelay;
				pNewScript->m_sMoveAnim = m_sMoveAnim;
				pNewScript->m_sActionAnim = m_sActionAnim;
				pNewScript->m_sFireWhenDone = m_sFireWhenDone;
				pNewScript->m_sEventName = m_sEventName;
				pNewScript->m_fStopAI = m_fStopAI;
				pNewScript->m_fShouldExpire = true;
				pNewScript->Act(pActivator, pCaller, useType, value);
			}
			return;
		}
	}
	else
	{
		//Print("->(CMSMonster *)pActivator");
		pMonster = (CMSMonster *)pActivator;
	}

	//Monster must be alive
	//Print("->IsAlive()2");
	if (!pMonster->IsAlive())
		return;
	//Don't stop an attacking monster or
	//a monster already in a script
	//Print("->MONSTERSTATE_SCRIPT");
	if (pMonster->m_MonsterState == MONSTERSTATE_SCRIPT)
		return;
	//Print("->m_fStopAI");
	if (!m_fStopAI && pMonster->m_hEnemy != NULL)
		return;

	m_NPC = pMonster;

	//Thothie NOV2007a - attempting to fix buggy ms_npcscript behavior
	//Print("->m_iType==SCRIPT_RUNEVENT");
	if (m_iType == SCRIPT_RUNEVENT)
	{
		RunScriptEvent();
		//DEC2007a note: something was preventing me from running monster scripts on helena/helena
		//hence all these crazy Print debugs
		//pMonster->m_MonsterState = MONSTERSTATE_SCRIPT;
		return;
	}

	if (m_iType == SCRIPT_MOVE ||
		m_iType == SCRIPT_MOVE_PLAYANIM ||
		m_iType == SCRIPT_MOVE_RUNEVENT)
	{
		m_MoveDest.Origin = pev->origin;
		m_MoveDest.Proximity = pMonster->GetDefaultMoveProximity();

		pMonster->m_MoveDest = m_MoveDest;
		pMonster->SetConditions(MONSTER_HASMOVEDEST);
		if (m_fStopAI)
			pMonster->SetConditions(MONSTER_NOAI);
		pMonster->m_Activity = ACT_WALK;
		pMonster->m_hEnemy = this;
		if (m_sMoveAnim)
		{
			//Thothie JUN2007b - this is causing monsters to studder if players move just out of atk range
			//- if you dance near such a monster, he can never attack, as this break stops his atk anim
			//pMonster->SetAnimation( MONSTER_ANIM_BREAK );
			pMonster->SetAnimation(MONSTER_ANIM_WALK, (char *)STRING(m_sMoveAnim));
		}
		SetThink(&NPCScript::MoveThink);
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else if (m_iType == SCRIPT_PLAYANIM)
	{
		PlayAnim();
	}
	else if (m_iType == SCRIPT_RUNEVENT)
	{
		RunScriptEvent();
	}
	else
		return;

	pMonster->m_MonsterState = MONSTERSTATE_SCRIPT;
}
void NPCScript ::PlayAnim()
{
	CMSMonster *pMonster = (CMSMonster *)m_NPC.Entity();
	if (!pMonster || !pMonster->IsAlive())
	{
		Finish();
		return;
	}

	pMonster->m_Activity = ACT_IDLE;
	pMonster->pev->angles = pev->angles;
	pMonster->pev->sequence = 0;
	if (m_sActionAnim)
	{
		pMonster->SetAnimation(MONSTER_ANIM_BREAK);
		pMonster->SetAnimation(MONSTER_ANIM_ONCE, (char *)STRING(m_sActionAnim));
	}
	pMonster->pev->frame = 0;			   //In case the previous AND current anim is sequence 0
	pMonster->m_fSequenceFinished = FALSE; //ditto...
	SetThink(&NPCScript::AnimateThink);
	pev->nextthink = gpGlobals->time + 0.1;
}
void NPCScript ::RunScriptEvent()
{
	CMSMonster *pMonster = (CMSMonster *)m_NPC.Entity();
	if (!pMonster || !pMonster->IsAlive())
	{
		Finish();
		return;
	}

	pMonster->CallScriptEvent(STRING(m_sEventName));

	//SetThink( Finish );
	//pev->nextthink = gpGlobals->time + 0.1;
	Finish();
}
void NPCScript ::MoveThink()
{
	CMSMonster *pMonster = (CMSMonster *)m_NPC.Entity();
	if (!pMonster || !pMonster->IsAlive())
	{
		Finish(true);
		return;
	}

	if (pMonster->m_MoveDest != m_MoveDest)
	{
		Finish(true);
		return;
	}

	pMonster->m_NextNodeTime = pMonster->m_NodeCancelTime = gpGlobals->time + 2.0;
	pev->nextthink = gpGlobals->time + 0.1;

	if (!pMonster->HasConditions(MONSTER_HASMOVEDEST))
	{
		//if( fShouldExpire ) ALERT( at_console, "!!NO MONSTER_HASMOVEDEST\n" );

		if (pMonster->m_IdleAnim.len())
			pMonster->SetAnimation(MONSTER_ANIM_WALK, pMonster->m_IdleAnim);
		else
		{
			pMonster->m_pAnimHandler = NULL;
			pMonster->SetActivity(ACT_IDLE);
		}
		pMonster->pev->angles = pev->angles;

		switch (m_iType)
		{
		case SCRIPT_MOVE_PLAYANIM:
			PlayAnim();
			break;
		case SCRIPT_MOVE_RUNEVENT:
			RunScriptEvent();
			break;
		default:
			Finish();
			break;
		}
	}
}
void NPCScript ::AnimateThink()
{
	CMSMonster *pMonster = (CMSMonster *)m_NPC.Entity();
	if (!pMonster)
	{
		Finish();
		return;
	}

	pev->nextthink = gpGlobals->time + 0.1;

	if (pMonster->m_fSequenceFinished)
	{
		Vector vBoneVec, vBoneAngle;
		pMonster->GetBonePosition(0, vBoneVec, vBoneAngle);
		if ((pev->origin - vBoneVec).Length() > 8.0)
		{
			pMonster->pev->origin.x = vBoneVec.x;
			pMonster->pev->origin.y = vBoneVec.y;
			pMonster->pev->origin.z += 1;
			DROP_TO_FLOOR(pMonster->edict());
			UTIL_SetOrigin(pMonster->pev, pMonster->pev->origin);
			pMonster->pev->effects |= EF_NOINTERP;
		}

		Finish();
	}
}
void NPCScript ::Finish()
{
	if (m_flFireDelay && !m_EarlyBreak)
	{
		SetThink(&NPCScript::FireTarget);
		pev->nextthink = gpGlobals->time + m_flFireDelay;
	}
	else
		FireTarget();
}
void NPCScript ::FireTarget()
{
	CMSMonster *pMonster = (CMSMonster *)m_NPC.Entity();

	if (pMonster)
	{
		pMonster->ClearConditions(MONSTER_NOAI);
		pMonster->m_MonsterState = MONSTERSTATE_NONE;
		if (pMonster->m_hEnemy == this)
			pMonster->m_hEnemy = NULL;
		m_NPC = NULL;
	}

	SetThink(NULL);

	string_t FireEvent = m_EarlyBreak ? m_sFireOnBreak : m_sFireWhenDone;
	if (FireEvent)
		FireTargets(STRING(FireEvent), pMonster, this, USE_TOGGLE, 0);

	if (m_fShouldExpire)
		DelayedRemove();
}
void NPCScript ::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "type"))
	{
		m_iType = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "moveanim"))
	{
		m_sMoveAnim = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "actionanim"))
	{
		m_sActionAnim = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "eventname"))
	{
		m_sEventName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "firewhendone"))
	{
		m_sFireWhenDone = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fireonbreak"))
	{
		m_sFireOnBreak = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "firedelay"))
	{
		m_flFireDelay = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "stopai"))
	{
		m_fStopAI = atoi(pkvd->szValue) ? true : false;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity ::KeyValue(pkvd);
}
