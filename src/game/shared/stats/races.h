#ifndef INC_RACES
#define INC_RACES

#include "sharedutil.h"

enum relationship_e
{
	RELATIONSHIP_NM = -5, // (NEMESIS)  A monster Will ALWAYS attack its nemsis, no matter what
	RELATIONSHIP_HT = -4, // (HATE)	will attack this character instead of any visible DISLIKEd characters
	RELATIONSHIP_DL = -3, // (DISLIKE) will attack.
	RELATIONSHIP_WA = -2, // (WARY) Only attacks if attacked first.
	RELATIONSHIP_FR = -1, // (FEAR)	will run
	RELATIONSHIP_NE = 0,  // (NO RELATIONSHIP) NEUTRAL NPC
	RELATIONSHIP_AL = 1,  // (ALLY) pals. Good alternative to R_NO when applicable.
	RELATIONSHIP_NO = 2,  // default, AKA no race.
};

struct race_t
{
	msstring Name;
	msstringlist Enemies;
	msstringlist Allies;
	msstringlist Wary;
};

class CRaceManager
{
public:
	//Relationship of source race to target race
	static relationship_e Relationship(msstring_ref pszSourceRace, msstring_ref pszTargetRace);
	static void AddRace(race_t &Race);
	static void DeleteAllRaces();
	static race_t *GetRacePtr(msstring_ref pszName);
	static bool RelationshipContains(msstringlist &RaceList, msstring_ref pszTargetRace);
	static mslist<race_t> Races;
};

#endif //INC_RACES