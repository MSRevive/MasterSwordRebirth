#ifndef INC_RACES
#define INC_RACES

#include "../MSShared/sharedutil.h"

enum relationship_e
{
	RELATIONSHIP_NM = -4, // (NEMESIS)  A monster Will ALWAYS attack its nemsis, no matter what
	RELATIONSHIP_HT = -3, // (HATE)	will attack this character instead of any visible DISLIKEd characters
	RELATIONSHIP_DL = -2, // (DISLIKE) will attack
	RELATIONSHIP_FR = -1, // (FEAR)	will run
	RELATIONSHIP_NO = 0,  // (NO RELATIONSHIP) disregard
	RELATIONSHIP_AL = 1,  // (ALLY) pals. Good alternative to R_NO when applicable.
};

struct race_t
{
	msstring Name;
	msstringlist Enemies;
	msstringlist Allies;
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