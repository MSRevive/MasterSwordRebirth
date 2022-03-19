#include "bodyparts.h"

enum
{
	//Human bodyparts
	HBP_LEGS = 0,
	HBP_HEAD,
	HBP_CHEST,
	HBP_ARMS,
	HUMAN_BODYPARTS
};
extern char *ModelList[HUMAN_BODYPARTS][2];

class CHumanBodypart : public CBodypart
{
public:
	CHumanBodypart *Duplicate();
};

class CHumanBody : public CBaseBody
{
public:
	void Initialize(CBaseEntity *pOwner, void *pvData = NULL);
	CBaseBody *Duplicate();
};

int HitGroupToBodyPart(int HitGroup);