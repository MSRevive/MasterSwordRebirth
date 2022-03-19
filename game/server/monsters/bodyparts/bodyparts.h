enum
{ //Bodypart states
	BPS_OWNER = 0,
	BPS_MODEL,
	BPS_BODY,
	BPS_ARMOR,
	BPS_TRANS,
	BPS_RDRNORM,
	BPS_RDRMODE,
	BPS_RDRAMT,
	BPS_RDRFX,
	BPS_RDRCOLOR,
};
//In general
//0 = Head, 1 = Chest, 2 = Left Arm, 3 = Right Arm, 4 = Left Leg, 5 = Right Leg

class CGenericItem;

class CBodypart : public CBaseEntity
{
public:
	virtual void Initialize(CBaseEntity *pOwner, char *ModelName, int idx);
	//virtual float TraceAttack( int iLastHitGroup, CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType );
	virtual CBodypart *Duplicate(CBodypart *pExistingBodypart = NULL);
	virtual void Set(int iState, void *vData);
	//virtual int MSProperties( ) { return ENT_BODYPART; }
	//virtual void *MSQuery( int iRequest );

	bool m_Visible;
	int m_Idx;
};

class CBaseBody
{
public:
	mslist<CBodypart *> Bodyparts;

	virtual void Spawn() {}
	virtual void Initialize(CBaseEntity *pOwner, void *pvData = NULL) {}
	CBodypart *Bodypart(int iBodypart)
	{
		if (iBodypart < 0 || iBodypart >= (signed)Bodyparts.size())
			return NULL;
		return Bodyparts[iBodypart];
	}
	virtual void Set(int iState, void *vData);
	virtual void Think(class CMSMonster *pOwner);
	virtual CBaseBody *Duplicate();
	virtual void Delete();
};
