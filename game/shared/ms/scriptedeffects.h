//Scripted effects

//Flags
#define SCRIPTEFFECT_NORMAL (0)
#define SCRIPTEFFECT_PLAYERACTION (1 << 0)
#define SCRIPTEFFECT_NOSTACK (1 << 1)

//Interface for monsters, items, etc.
/*class IScriptedEffect
{
public:
	
	~IScriptedEffect( ) { foreach( i, m_Effects.size() ) { delete m_Effects[i]; m_Effects.erase( i ); i--; } }

	mslist<CScriptedEffect *> m_Effects;
};*/

struct globalscripteffect_t
{
	globalscripteffect_t() {}
	globalscripteffect_t(string_i Name, string_i Scriptname, int Flags)
	{
		m_Name = Name;
		m_ScriptName = Scriptname;
		m_Flags = Flags;
	}

	string_i m_Name, m_ScriptName;
	int m_Flags;
};

class CGlobalScriptedEffects
{
public:
	static mslist<globalscripteffect_t> Effects;

	static void RegisterEffect(globalscripteffect_t &Effect);
	static CScript *ApplyEffect(msstring_ref ScriptName, IScripted *pScriptTarget, CBaseEntity *pTarget, msstringlist *Parameters = NULL);
	static void DeleteEffects();
};
