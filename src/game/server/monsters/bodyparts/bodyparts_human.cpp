#include "msdllheaders.h"
#include "monsters/msmonster.h"
#include "player/modeldefs.h"
#include "bodyparts_human.h"
#include "monsters.h"

extern char * ModelListHumanMale[HUMAN_BODYPARTS];
extern char* ModelListHumanFemale[HUMAN_BODYPARTS];

CHumanBodypart *CHumanBodypart::Duplicate()
{
	CHumanBodypart *pNewBodypart = GetClassPtr((CHumanBodypart *)NULL);
	return (CHumanBodypart *)CBodypart::Duplicate(pNewBodypart);
}

void CHumanBody::Initialize(CBaseEntity *pOwner, void *pvData)
{
	for (int i = 0; i < HUMAN_BODYPARTS; i++)
	{
		CHumanBodypart& HumanBodypart = *(CHumanBodypart*)GetClassPtr((CHumanBodypart*)NULL);

		int Gender = ((CMSMonster*)pOwner)->m_Gender;
		if (pvData)
			Gender = (int)pvData;

		HumanBodypart.Initialize(pOwner, ((Gender > 0) ? ModelListHumanFemale[i] : ModelListHumanMale[i]), i);
		Bodyparts.add(&HumanBodypart);
	}
}

CBaseBody *CHumanBody::Duplicate()
{
	CHumanBody &NewBody = *msnew(CHumanBody);
	for (int i = 0; i < Bodyparts.size(); i++)
		NewBody.Bodyparts.add(Bodyparts[i]->Duplicate());

	return &NewBody;
}

int HitGroupToBodyPart(int HitGroup)
{
	switch (HitGroup)
	{
	case HITGROUP_HEAD:
		return HBP_HEAD;

	case HITGROUP_CHEST:
	case HITGROUP_STOMACH:
		return HBP_CHEST;

	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
		return HBP_ARMS;

	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		return HBP_LEGS;

	default:
		return HBP_CHEST;
	}
}