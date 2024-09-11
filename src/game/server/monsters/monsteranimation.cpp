#include "msdllheaders.h"
#include "weapons/weapons.h"

//Shorten things up a lot
#define m_Activity m_pOwner->m_Activity
#define m_fSequenceFinished m_pOwner->m_fSequenceFinished
#define LookupSequence m_pOwner->LookupSequence

#undef Wielded
#undef CHWielded
#define CHWielded ((m_pPlayer->Hand[m_pPlayer->iCurrentHand]) ? m_pPlayer->Hand[m_pPlayer->iCurrentHand]->Wielded() : (m_pPlayer->PlayerHands ? m_pPlayer->PlayerHands->Wielded() : FALSE))

CAnimation *CAnimation::ChangeTo(MONSTER_ANIM NewAnim)
{
	m_pOwner->m_pLastAnimHandler = this;
	switch (NewAnim)
	{
	case MONSTER_ANIM_WALK:
	case MONSTER_ANIM_BREAK:
		m_pOwner->m_IdealActivity = ACT_WALK;
		return &gAnimWalk;
	case MONSTER_ANIM_HOLD:
		m_pOwner->m_IdealActivity = ACT_RESET;
		return &gAnimHold;
	case MONSTER_ANIM_ONCE:
		m_pOwner->m_IdealActivity = ACT_RESET;
		return &gAnimOnce;
	}
	return this;
}
bool CAnimation ::SetAnim(msstring_ref pszSequence)
{
	if (!pszSequence || !pszSequence[0])
		return false;

	int animDesired = LookupSequence(pszSequence);

	if (animDesired <= -1)
	{
		if (pszSequence && strlen(pszSequence))
			ALERT(at_console, "ERROR: Monster: %s - LookupSequence for sequence \"%s\" FAILED!\n", m_pOwner->DisplayName(), pszSequence);

		return false;
	}

	// Reset to first frame of desired animation
	m_pOwner->pev->sequence = animDesired;
	m_pOwner->pev->frame = 0;
	m_pOwner->ResetSequenceInfo();

	return true;
}
bool CAnimation ::SetGaitAnim(msstring_ref pszSequence)
{
	int animDesired = LookupSequence(pszSequence);
	if (animDesired <= -1)
	{
		m_pOwner->pev->gaitsequence = 0;
		return false;
	}

	m_pOwner->pev->gaitsequence = animDesired;
	return true;
}
void CAnimation ::GaitAnimate()
{
	if (!m_pOwner->IsPlayer())
		return;

	CBasePlayer *m_pPlayer = (CBasePlayer *)m_pOwner;
	float speed = m_pPlayer->pev->velocity.Length2D();
	//ALERT( at_console, "speed: %f walkspeed: %f", speed, WalkSpeed );
	bool bCustomLegs = false;

	char *m_szAnimLegs = m_pPlayer->m_szAnimLegs;

	if (strlen(m_szAnimLegs) > 0)
	{
		bCustomLegs = true;
		//ALERT( at_console, "Custom legs\n" );
	}

	int NewGait = -1;
	if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
	{
		if (speed == 0)
		{
			if (bCustomLegs)
				NewGait = LookupSequence(UTIL_VarArgs("crouchidle_", m_szAnimLegs));

			if (NewGait < 0)
				NewGait = LookupSequence("crouchidle");
		}
		else
		{
			if (bCustomLegs)
				NewGait = LookupSequence(UTIL_VarArgs("crouchmove_", m_szAnimLegs));

			if (NewGait < 0)
				NewGait = LookupSequence("crouch");
		}
	}
	else if (speed > m_pOwner->WalkSpeed(FALSE) + 1)
	{
		if (bCustomLegs)
			NewGait = LookupSequence(UTIL_VarArgs("run_%s", m_szAnimLegs));
		if (NewGait < 0)
			NewGait = LookupSequence("run");
	}
	else if (speed > 0)
	{
		if (bCustomLegs)
			NewGait = LookupSequence(UTIL_VarArgs("walk_%s", m_szAnimLegs));
		if (NewGait < 0)
			NewGait = LookupSequence("walk");
	}
	else
	{
		if (bCustomLegs)
			NewGait = LookupSequence(UTIL_VarArgs("stand_%s", m_szAnimLegs));
		//		if( bCustomLegs ) NewGait = 0;
		if (NewGait < 0)
			NewGait = LookupSequence("stand");
		//else NewGait = 0;
	}

	if (NewGait == -1)
	{
		m_pPlayer->pev->gaitsequence = 0;
		if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		{
			if (!speed)
				m_pPlayer->pev->gaitsequence = m_pOwner->LookupActivity(ACT_CROUCH);
			else
				m_pPlayer->pev->gaitsequence = m_pOwner->LookupActivity(ACT_CROUCHIDLE);
		}
	}
	else
		m_pPlayer->pev->gaitsequence = NewGait;
}

CWalkAnim gAnimWalk;

bool CWalkAnim ::CanChangeTo(MONSTER_ANIM NewAnim, void *vData)
{
	return true;
}
void CWalkAnim ::Animate()
{
	msstring_ref TorsoAnimName = m_pOwner->pszCurrentAnimName;
	msstring_ref LegsAnimName = NULL;

	if (m_pOwner->IsPlayer())
	{
		CBasePlayer *pPlayer = (CBasePlayer *)m_pOwner;
		TorsoAnimName = pPlayer->m_szAnimTorso;
		LegsAnimName = pPlayer->m_szAnimLegs;
	}

	int animDesired = LookupSequence(TorsoAnimName);

	if (animDesired != m_pOwner->pev->sequence) //Continue playing the same uninterrupted animation until told otherwise
		SetAnim(TorsoAnimName);

	if (LegsAnimName)
		SetGaitAnim(LegsAnimName);
}

CAnimHold gAnimHold; //Never submits to MONSTER_ANIM_WALK
					 //Submits to anything else if ReleaseAnim == TRUE
					 //Only uses gait if specified with flags
void CAnimHold ::Initialize(void *vData)
{
	ReleaseAnim = TRUE;
	UseGait = false; //defaults
	int Flags = (int)vData;
	if (Flags & (1 << 0))
		ReleaseAnim = FALSE; //Release to any anim besides walk
	if (Flags & (1 << 1))
		UseGait = TRUE; //Use gait
	IsNewAnim = TRUE;
}
bool CAnimHold ::CanChangeTo(MONSTER_ANIM NewAnim, void *vData)
{
	if (NewAnim == MONSTER_ANIM_WALK)
		return false;
	if (!ReleaseAnim)
		return false;
	return true;
}
void CAnimHold ::Animate()
{

	if (IsNewAnim)
		SetAnim(m_pOwner->pszCurrentAnimName);

	if (UseGait)
		GaitAnimate();
	else
		m_pOwner->pev->gaitsequence = 0;
}

CAnimOnce gAnimOnce; //Doesn't submit until it's finished playing
					 //it's sequence
void CAnimOnce ::Initialize(void *vData)
{
	UseGait = vData ? true : false;

	m_pOwner->SetScriptVar(VAR_NPC_ANIM_TORSO, m_pOwner->pszCurrentAnimName);
	if (m_pOwner->IsPlayer())
	{
		CBasePlayer *pPlayer = (CBasePlayer *)m_pOwner;
		strncpy(pPlayer->m_szAnimTorso, m_pOwner->pszCurrentAnimName, 32);
		pPlayer->m_szAnimLegs[0] = 0;
	}

	CAnimation::Initialize(vData);
}
bool CAnimOnce ::CanChangeTo(MONSTER_ANIM NewAnim, void *vData)
{
	return m_fSequenceFinished ? true : false;
}
void CAnimOnce ::Animate()
{
	if (m_pOwner->IsPlayer())
	{
		CBasePlayer *pPlayer = (CBasePlayer *)m_pOwner;
		if (IsNewAnim)
			SetAnim(pPlayer->m_szAnimTorso);
		SetGaitAnim(pPlayer->m_szAnimLegs); //Keep setting the gait, because it can change while playing anim

		return;
	}

	if (IsNewAnim)
		SetAnim(m_pOwner->pszCurrentAnimName);

	if (UseGait)
		GaitAnimate();
	else
		m_pOwner->pev->gaitsequence = 0;
}

/*					//stays in the last pose
void CAnimAct :: Initialize( void *vData ) {
	if( vData ) ReleaseAnim = FALSE;
	else ReleaseAnim = TRUE;
	IsNewAnim = TRUE;
}
bool CAnimAct :: CanChangeTo( MONSTER_ANIM NewAnim, void *vData ) {
	if( m_fSequenceFinished && ReleaseAnim ) return TRUE;
	return FALSE;
}
void CAnimAct :: Animate( ) {
	if( !IsNewAnim ) return;
	int animDesired;
	float speed = m_pOwner->pev->velocity.Length2D();

	animDesired = m_pOwner->LookupActivity( m_Activity );
	m_pOwner->pev->gaitsequence = 0;

	if( animDesired == -1 ) {
		ALERT( at_console, "ERROR: Monster: %s - LookupActivity for Activity \"%i\" FAILED!\n", m_pOwner->DisplayName(), m_Activity );
		animDesired = 0;
	}

	if( m_pOwner->pev->sequence != animDesired ) {

		// Reset to first frame of desired animation
		m_pOwner->pev->sequence		= animDesired;
		m_pOwner->pev->frame			= 0;
		m_pOwner->ResetSequenceInfo( );
	}
}
*/