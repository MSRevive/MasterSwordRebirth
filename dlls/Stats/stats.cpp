#include "../MSShared/sharedutil.h"
#include "statdefs.h"
#include "Stats.h"
#include <math.h>
#include <string.h>

statinfo_t NatStatList[6] =
{
	"Strength",
	"Agility",
	"Concentration",
	"Awareness",
	"Fitness",
	"Wisdom",
};

skillstatinfo_t SkillStatList[9] =
{
	"Swordsmanship", "swordsmanship", STATPROP_TOTAL,
	"Martial Arts", "martialarts", STATPROP_TOTAL,
	"Small Arms", "smallarms", STATPROP_TOTAL,
	"Axe Handling", "axehandling", STATPROP_TOTAL,
	"Blunt Arms", "bluntarms", STATPROP_TOTAL,
	"Archery", "archery", STATPROP_TOTAL,
	//	"Shield handling",
	//	"Two-handed weapons",
	//	"Dual weapons",
	"Spell Casting", "spellcasting", STAT_MAGIC_TOTAL,
	"Parry", "parry", 1,
	"Pole Arms", "polearms", STATPROP_TOTAL, //MiB JUL2010_02 - Pole Arms!
	//	"Spell Preparation",
	//	"Swimming",
	//	"Pickpocket", true,
};

char *SkillTypeList[] =
{
	"Proficiency",
	"Balance",
	"Power"
};

char *SpellTypeList[] =
{
	"Fire",
	"Ice",
	"Lightning",
	"unused", //Thothie: was summoning
	"unused", //Thothie: was protection
	"Divination",
	"Affliction",
};

int GetSkillStatByName(const char *pszName) //Index lookup by name (Skill stats only)
{
	for (int i = 0; i < SKILL_MAX_STATS; i++)
		if (!stricmp(pszName, SkillStatList[i].DllName))
			return SKILL_FIRSTSKILL + i;
	return -1;
}
const char *GetSkillName(int Skill) //Name lookup by index (Any stat)
{
	if (Skill < 0 || Skill >= STATS_TOTAL)
		return "(Invalid Skill)";

	if (Skill < SKILL_FIRSTSKILL)
		return NatStatList[Skill].Name;

	return SkillStatList[Skill - SKILL_FIRSTSKILL].Name;
}
int GetSubSkillByName(const char *pszName)
{
	if (!stricmp(pszName, "prof")) //alias for proficiency
		return 0;
	for (int i = 0; i < STATPROP_TOTAL; i++)
		if (!stricmp(pszName, SkillTypeList[i]))
			return i;
	for (int i = 0; i < STAT_MAGIC_TOTAL; i++)
		if (!stricmp(pszName, SpellTypeList[i]))
			return i;
	return -1;
}
int GetNatStatByName(const char *pszName)
{
	for (int i = 0; i < NATURAL_MAX_STATS; i++)
		if (!stricmp(pszName, NatStatList[i].Name))
			return i;
	return -1;
}
//Converts stat.prop into valid indices
void GetStatIndices(const char *Name, int &Stat, int &Prop)
{
	msstring FullName = Name;

	msstring StatName = FullName.thru_char(".");
	msstring PropName = FullName.substr(StatName.len() + 1);
	if (StatName.len())
		Stat = GetSkillStatByName(StatName);
	if (PropName.len())
		Prop = GetSubSkillByName(PropName);
}

CSubStat::~CSubStat()
{
	//Value.UnRegister( );
	//Exp.UnRegister( );
}
CSubStat &CSubStat::operator=(const CSubStat &Other)
{
#ifdef MEM_ENCRYPT
	Value.Set(Other.Value.Get());
	Exp.Set(Other.Exp.Get());
#else
	Value = Other.Value;
	Exp = Other.Exp;
#endif
	return *this;
}
bool CSubStat::Changed()
{
#ifdef MEM_ENCRYPT
	return Value.m_Changed || Exp.m_Changed;
#else
	return (Value != m_OldValue) || (Exp != m_OldExp);
#endif
}

int CStat::operator=(int Equals)
{
	int iAdd = int(Equals / (float)m_SubStats.size());
	int iExtra = Equals % m_SubStats.size(), i = 0;

	for (i = 0; i < (signed)m_SubStats.size(); i++)
		m_SubStats[i].Value = iAdd;

	for (iExtra; iExtra > 0; iExtra--)
	{
		int iLowestStat = 0;
		for (i = 0; i < (signed)m_SubStats.size(); i++)
			if (m_SubStats[i].Value < m_SubStats[iLowestStat].Value)
				iLowestStat = i;
		m_SubStats[iLowestStat].Value++;
	}

	return Value();
}
int CStat::operator+=(int Add)
{
	for (Add; abs(Add) > 0; Add -= Add / abs(Add))
	{
		int iLowestStat = 0, i;
		for (i = 0; i < (signed)m_SubStats.size(); i++)
			if (m_SubStats[i].Value < m_SubStats[iLowestStat].Value)
				iLowestStat = i;
		m_SubStats[iLowestStat].Value += Add / abs(Add);
	}
	return Value();
}
int CStat::Value()
{
	int Total = 0;
	int iSubStats = m_SubStats.size();
	for (int i = 0; i < iSubStats; i++)
		Total += m_SubStats[i].Value;

	int iVal = (Total / iSubStats) + ((Total % iSubStats) ? 1 : 0);
	return iVal;
}
int CStat::Value(int StatProperty)
{
	if (StatProperty >= (signed)m_SubStats.size())
		return -1;

	return m_SubStats[StatProperty].Value;
}
void CStat::OutDate()
{
//Makes sure an update will be sent next frame
#ifdef MEM_ENCRYPT
	for (int i = 0; i < m_SubStats.size(); i++)
	{
		m_SubStats[i].Value.m_Changed = true;
		m_SubStats[i].Exp.m_Changed = true;
	}
#else
	for (int i = 0; i < m_SubStats.size(); i++)
	{
		m_SubStats[i].m_OldValue = !m_SubStats[i].Value;
		m_SubStats[i].m_OldExp = !m_SubStats[i].Exp;
	}
#endif
}
void CStat::Update()
{
	//Updates the stat to current - no updates sent

#ifdef MEM_ENCRYPT
	for (int i = 0; i < m_SubStats.size(); i++)
	{
		m_SubStats[i].Value.m_Changed = false;
		m_SubStats[i].Exp.m_Changed = false;
	}
#else
	for (int i = 0; i < m_SubStats.size(); i++)
	{
		m_SubStats[i].m_OldValue = m_SubStats[i].Value;
		m_SubStats[i].m_OldExp = m_SubStats[i].Exp;
	}
#endif
}
bool CStat::Changed()
{
	//Check if the Changed bit is set on any substats
	for (int i = 0; i < m_SubStats.size(); i++)
		if (m_SubStats[i].Changed())
			return true;
	return false;
}

bool CStat::operator!=(const CStat &Other)
{
	//Just check the substats
	for (int i = 0; i < m_SubStats.size(); i++)
	{
		if (i >= (signed)Other.m_SubStats.size())
			break;
		if (m_SubStats[i].Value != Other.m_SubStats[i].Value)
			return true;
		if (m_SubStats[i].Exp != Other.m_SubStats[i].Exp)
			return true;
	}
	return false;
}

void CStat::InitStatList(statlist &Stats)
{
	//Due to memory encryption, these must be allocated all at once.  It's faster too
	Stats.reserve_once(STATS_TOTAL, STATS_TOTAL);
	for (int i = 0; i < STATS_TOTAL; i++)
	{
		msstring_ref Name = (i < NATURAL_MAX_STATS) ? NatStatList[i].Name : SkillStatList[i - NATURAL_MAX_STATS].DllName;
		CStat::skilltype_e Type = (i < NATURAL_MAX_STATS) ? CStat::STAT_NAT : CStat::STAT_SKILL;
		CStat &Stat = Stats[i];
		Stat.m_Name = Name;
		Stat.m_Type = Type;
		int iSubStats = (Stat.m_Type == CStat::STAT_NAT) ? 1 : SkillStatList[i - NATURAL_MAX_STATS].StatCount;
		Stat.m_SubStats.reserve_once(iSubStats, iSubStats);
	}
}
