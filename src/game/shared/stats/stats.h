#ifndef STATS_H
#define STATS_H

typedef unsigned long ulong;

class CSubStat
{
public:
	CSubStat(int iValue, int iExp)
	{
		Value = OldValue = iValue;
		Exp = OldExp = iExp;
	}

	CSubStat() { Clear(); }

	~CSubStat();

	void Clear()
	{
		Value = OldValue = 0;
		Exp = OldExp = 0;
	}

	CSubStat &operator=(const CSubStat &Other);

	int Value; // Level of the skill
	int OldValue;

	ulong Exp; // Total Experience in this skill
	ulong OldExp;	
};

class CStat
{
public:
	mslist<CSubStat> m_SubStats; //For regular stats, there are three - Speed, balance, power
								 //For spellcasting, there is more
								 //For parry, there is only one
	string_i m_Name;
	bool bNeedsUpdate;
	enum skilltype_e
	{
		STAT_NAT,
		STAT_SKILL
	} m_Type; //Stat type

	CStat() { bNeedsUpdate = true; }
	CStat(msstring_ref Name, skilltype_e Type)
	{
		m_Name = Name;
		m_Type = Type;
		bNeedsUpdate = true;
	}
	bool operator==(CStat &CompareStat);
	operator int() { return Value(); }
	int operator=(int Equals);
	int operator+=(int Add);
	int Value();
	int Value(int StatProperty);
	bool operator!=(const CStat &Other);
	bool Changed(); //Has changed since OutDate called
	void OutDate(); //Makes sure a change is sent next frame
	void Update();	//Sets status to current - No updates sent

	CSubStat* GetSubStat(int index)
	{
		if ((index < 0) || (index >= m_SubStats.size()))
			return NULL;
		return &m_SubStats[index];
	}

	static void InitStatList(mslist<CStat> &Stats);
};

typedef mslist<CStat> statlist;

//A caching structure for CStat.  Allows you to do numerous lookups without calling
//Value() each time.  Used in TitleManager::GetPlayerTitle()
struct skillcache_t
{
	int Skill;
	int Value;
};

struct statinfo_t
{
	char *Name;
};

struct skillstatinfo_t
{
	const char *Name;
	const char *DllName;
	int StatCount;
};

extern statinfo_t NatStatList[6];
extern skillstatinfo_t SkillStatList[9];
extern char *SkillTypeList[3];
extern char *SpellTypeList[5];

#endif // STATS_H