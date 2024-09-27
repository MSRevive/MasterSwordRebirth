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

extern const char* ModelListHumanMale[HUMAN_BODYPARTS];
extern const char* ModelListHumanFemale[HUMAN_BODYPARTS];

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