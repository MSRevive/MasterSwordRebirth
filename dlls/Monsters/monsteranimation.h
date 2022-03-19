#ifndef INC_PLAYERANIMS
#define INC_PLAYERANIMS

class CMSMonster;

typedef enum
{
	//Master Sword Modified
	MONSTER_ANIM_WALK,
	MONSTER_ANIM_ONCE, //Plays an anim once then releases it
	MONSTER_ANIM_HOLD, //Plays an anim, then holds it
	MONSTER_ANIM_ACT,  //Plays an anim once then releases it
	MONSTER_ANIM_BREAK //Explicitly break the current anim (which sets
					   //pAnim = gAnimWalk)
	//---------------------
} MONSTER_ANIM;

class CAnimation
{
public:
	CMSMonster *m_pOwner;
	MONSTER_ANIM m_Anim;
	bool IsNewAnim;
	bool UseGait;
	int Priority;

	virtual bool CanChangeTo(MONSTER_ANIM NewAnim, void *vData = NULL) { return true; }
	virtual CAnimation *ChangeTo(MONSTER_ANIM NewAnim);
	virtual void Initialize(void *vData = NULL) { IsNewAnim = true; }
	virtual void PostAnimate() { IsNewAnim = false; }
	virtual void Animate() = 0;
	virtual bool SetAnim(msstring_ref pszSequence);
	virtual bool SetGaitAnim(msstring_ref pszSequence);
	virtual void GaitAnimate();
	virtual MONSTER_ANIM GetID() { return m_Anim; }
};

class CWalkAnim : public CAnimation
{
public:
	bool CanChangeTo(MONSTER_ANIM NewAnim, void *vData = NULL);
	void Animate();

	CWalkAnim()
	{
		m_Anim = MONSTER_ANIM_WALK;
		UseGait = false;
	}
};

/*class CAnimJump : public CAnimation {
public:
	bool CanChangeTo( MONSTER_ANIM NewAnim, void *vData = NULL );
	void Animate( );
	CAnimation *ChangeTo( MONSTER_ANIM NewAnim );

	CAnimJump( ) { m_Anim = MONSTER_ANIM_JUMP; }
};*/

//Hold never releases to Walk.
//Initialized with
//vData & (1<<0) == Hold releases to any other anim than walk when request
//vData & (1<<1) == Hold will use gait
class CAnimHold : public CAnimation
{
	bool ReleaseAnim;

public:
	bool CanChangeTo(MONSTER_ANIM NewAnim, void *vData = NULL);
	void Animate();
	void Initialize(void *vData = NULL);

	CAnimHold() { m_Anim = MONSTER_ANIM_HOLD; }
};

class CAnimOnce : public CAnimation
{
public:
	bool CanChangeTo(MONSTER_ANIM NewAnim, void *vData = NULL);
	void Animate();
	void Initialize(void *vData = NULL);

	CAnimOnce() { m_Anim = MONSTER_ANIM_ONCE; }
};

class CAnimAct : public CAnimation
{
	bool ReleaseAnim;

public:
	bool CanChangeTo(MONSTER_ANIM NewAnim, void *vData = NULL);
	void Animate();
	void Initialize(void *vData = NULL);

	CAnimAct() { m_Anim = MONSTER_ANIM_ACT; }
};
extern CWalkAnim gAnimWalk;
//extern CAnimJump gAnimJump;
extern CAnimHold gAnimHold;
extern CAnimOnce gAnimOnce;
extern CAnimAct gAnimAct;

#endif