/*

	Statdefs.h - All stat definitions
				 This is in its own file so that any changes affect the minimal amount of cpp modules
*/
//This is in its own file because

#ifndef STATDEFS_H
#define STATDEFS_H

enum
{
	NATURAL_STR = 0, //How much you can carry
					 //Stamina regeneration
	NATURAL_DEX,	 //How fast you can move,
					 //How well you handle multiple weapons
	NATURAL_CON,	 //How focused you are
	NATURAL_AWR,	 //Being 'aware' of your surroundings
	NATURAL_FIT,	 //How much health you have
	NATURAL_WIS,	 //Makes spells take less stamina/mana

	NATURAL_MAX_STATS
};

enum
{
	SKILL_FIRSTSKILL = NATURAL_MAX_STATS,	//Skills are divided by 3
	SKILL_SWORDSMANSHIP = SKILL_FIRSTSKILL, //1/3 = Speed
	SKILL_MARTIALARTS,						//1/3 = Balance (Accuracy)
	SKILL_SMALLARMS,						//1/3 = Power (Damage)
	SKILL_AXEHANDLING,
	SKILL_BLUNTARMS,
	SKILL_ARCHERY,
	//SKILL_SHIELDHANDLING,
	//SKILL_TWOHANDED,			//This skill limits who can use two-handed weapons
	SKILL_SPELLCASTING, //This skill has many substats
	SKILL_PARRY,
	SKILL_POLEARMS,						   // MiB JUL2010_02 - Pole Arms!
	SKILL_MAX_ATTACK = SKILL_POLEARMS + 1, //Max number of attack skills (MiB JUL2010_02 - Again)
										   //	SKILL_SPELLPREPARING,
										   //	SKILL_SWIMMING,
										   //	SKILL_THEFT,

	SKILL_LASTSKILL = SKILL_POLEARMS, //MiB JUL2010_02 - And again..
	SKILL_MAX_STATS = (SKILL_LASTSKILL + 1) - NATURAL_MAX_STATS,
	STATS_TOTAL = (NATURAL_MAX_STATS + SKILL_MAX_STATS)
};

//#define SKILL_SPELLPREPARING SKILL_THEFT
//#define SKILL_SPELLCASTING SKILL_THEFT
//#define SKILL_SWIMMING SKILL_THEFT

enum
{
	STATPROP_SKILL = 0,
	STATPROP_BALANCE,
	STATPROP_POWER,
	STATPROP_TOTAL,

	STAT_MAGIC_FIRE = 0, //Magic.  These override the above properties for spellcasting
	STAT_MAGIC_ICE,
	STAT_MAGIC_LIGHTNING,
	STAT_MAGIC_SUMMONING,
	STAT_MAGIC_PROTECTION,
	STAT_MAGIC_DIVINATION,
	STAT_MAGIC_AFFLICTION,
	STAT_MAGIC_TOTAL,
	STATPROP_ALL_TOTAL = STAT_MAGIC_TOTAL,
};

int GetSkillStatByName(const char *pszName);				 //Returns -1 if not found
int GetSubSkillByName(const char *pszName);					 //Returns -1 if not found
int GetNatStatByName(const char *pszName);					 //Returns -1 if not found
void GetStatIndices(const char *Name, int &Stat, int &Prop); //Converts stat.prop into valid indices
const char *GetSkillName(int Skill);						 //Looks any skill

#define STAT_MAX_VALUE 300.0
#define STATPROP_MAX_VALUE 100.0
	//level cap

//For MSMonster::m_PlayerDamage
struct playerdamage_t
{
	float dmg[SKILL_MAX_ATTACK][STATPROP_ALL_TOTAL]; //Record damage done with with each combat skill
	float dmgInTotal; //MiB Mar2008a - For keeping track of all damage a player has done, regardless of what skill it was
	char msId[32]; // MiB MAR2019_22 [SLOT_EXP] - Keep track of the steam id / slot to avoid passing exp to the wrong character

	// MiB MAR2019_22 [SLOT_EXP] - Added because this is done in multiple places and shouldn't be written out...
	void Clear()
	{
		for(int i = 0; i < SKILL_MAX_ATTACK; i++)
		{
			for(int j = 0; j < STATPROP_ALL_TOTAL; j++)
			{
				dmg[i][j] = 0;
			}
		}
		dmgInTotal = 0;
		msId[0] = 0;
	}
};

#endif //STATDEFS_H
