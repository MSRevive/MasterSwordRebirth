
#ifndef BODYPARTS_HUMAN_H
#include "bodyparts.h"
#define BODYPARTS_HUMAN_H
enum
{
	//Human bodyparts
	HBP_LEGS = 0,
	HBP_HEAD,
	HBP_CHEST,
	HBP_ARMS,
	HUMAN_BODYPARTS
};

class CHumanBodypart : public CBodypart
{
public:
	CHumanBodypart *Duplicate();
};

class CHumanBody : public CBaseBody
{
public:
	void Initialize(CBaseEntity *pOwner, void *pvData = nullptr);
	CBaseBody *Duplicate();
};

int HitGroupToBodyPart(int HitGroup);
#endif