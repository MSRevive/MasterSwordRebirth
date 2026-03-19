#ifndef ISCRIPT_H // MiB MAR2015_01 [LOCAL_PANEL] - Made this include-
#define ISCRIPT_H

/*
	Dogg - Interface for a scripted entity
*/
class CScript;
class CBaseEntity;
#include <stdio.h>
#include "sharedutil.h"
#include "../../../public/archtypes.h"  // For ulong typedef

#ifndef VECTOR_H
#include "..\shared\hl\vector.h"
#endif

// Add Windows-specific ulong definition if not already defined
#ifdef _WIN32
#ifndef ulong
typedef unsigned long ulong;
#endif
#endif


struct scriptvar_t
{
	scriptvar_t() {}
	scriptvar_t(const char* name, const char* value)
	{
		Name = name;
		Value = value;
	}
	msstring Name;
	msstring Value;
	~scriptvar_t()
	{
	}
};

class IVariables
{
public:
	mslist<scriptvar_t> m_Variables; //Local script variables

	scriptvar_t *FindVar(const char* Name);
	const char* GetVar(const char* Name);
	scriptvar_t *SetVar(const char* Name, const char* Value);
	~IVariables()
	{
	}
};

//A script command name
struct scriptcmdname_t
{
	scriptcmdname_t() 
	{ 
		m_Conditional = false; 
	}
	scriptcmdname_t(const char* Name)
	{
		m_Name = Name;
		m_Conditional = false;
	}
	scriptcmdname_t(const char* Name, bool Conditional)
	{
		m_Name = Name;
		m_Conditional = Conditional;
	}

	msstring m_Name;
	bool m_Conditional;
};
typedef mslist<scriptcmdname_t> scriptcmdname_list;

//A script command with parameters
//If conditional, contains a list of sub-commands to be run when true
struct LegacyScriptCmd
{
	LegacyScriptCmd() { init(); }
	LegacyScriptCmd(const char* Name, bool Conditional = false)
	{
		init();
		m_Params.add(Name);
		m_Conditional = Conditional;
	}
	inline void init() 
	{ 
		m_Conditional = m_AddingElseCmds = m_NewConditional = false; 
	}
		
	inline msstring &Name()
	{
		static msstring NoName;
		return m_Params.size() ? m_Params[0] : NoName;
	}
	inline int Params() 
	{ 
		return m_Params.size() - 1; 
	}

	bool m_Conditional;	   //Whether this is a conditional command or normal command
	bool m_AddingElseCmds; //Whether I'm currently adding child cmds (executed when true) or else cmds (executed when false)
	bool m_NewConditional; //Whether this a new conditional, or a legacy conditional, which breaks all event execution on failure
	msstringlist m_Params; //m_Param[0] is the Command

	struct cmdlist_t
	{
		bool m_SingleCmd;			//Whether this list is a { } block or a single command at the end of a conditional
		mslist<LegacyScriptCmd> m_Cmds; //The commands of this command list
		~cmdlist_t()
		{
		}
	};
	cmdlist_t m_IfCmds;			  //If this is a conditional command, here is the group of comamnds to be executed if true
	mslist<cmdlist_t> m_ElseCmds; //If this is a conditional command, here is the group of comamnds to be executed if false
	~LegacyScriptCmd()
	{
	}
};
typedef LegacyScriptCmd::cmdlist_t legacy_scriptcmd_list;

enum LegacyEventScope
{
	LEGACY_EVENTSCOPE_SERVER,
	LEGACY_EVENTSCOPE_CLIENT,
	LEGACY_EVENTSCOPE_SHARED,
};

//The basic 'event' of a script.  Contains a list of commands
struct LegacyScriptEvent : public IVariables
{
	msstring Name;				   //Event name
	legacy_scriptcmd_list Commands;	   //The comamnds in this event
	msstringlist *Params;		   //The parameters passed to this event (Can be NULL)
	float fNextExecutionTime,	   //Next time to be run with repeatdelay
		fRepeatDelay;			   //Repeat every x seconds
	LegacyEventScope Scope;			   //Scope (delete events that aren't in the right scope... client events on server and vica versa)
	mslist<float> TimedExecutions; //Clock times in the future that this event should be executed
	bool bFullStop;				   //MiB DEC2014_07 - "exitevent" command (exit.rtf)

	void SetLocal(const char* Name, const char* Value) { SetVar(Name, Value); }
	const char* GetLocal(const char* Name);
	~LegacyScriptEvent()
	{
	}
};

struct scriptsendcmd_t
{
	msstring ScriptName, //Name of the script to load on client
		MsgType,		 //Who to send the command to
		MsgTarget;		 //Who to send the command to
	msstringlist Params; //Parameters sent to client
	ulong UniqueID;
};

//Interface
class IScripted
{
public:
	virtual CScript *Script_Add(msstring ScriptName, CBaseEntity *pEntity); //Adds a new script to the list
	virtual CScript *Script_Get(msstring ScriptName);
	virtual void Script_Remove(int idx);																						   //Removes a script
	virtual void Script_InitHUD(class CBasePlayer *pPlayer);																	   //Called when a player joins the game
	virtual void Script_Setup() {}																								   //Ties m_pScriptCommands to a global somewhere
	virtual int Script_ParseLine(CScript *Script, const char* pszCommandLine, LegacyScriptCmd &Cmd);								   //Parses a line of script text and returns the command type
	virtual void RunScriptEvents(bool fOnlyRunNamedEvents = false);																   //Runs all events
	virtual bool Script_ExecuteEvent(CScript *Script, LegacyScriptEvent &Event, msstringlist *Parameters = NULL) { return false; }	   //Runs an event
	virtual bool Script_SetupEvent(CScript *Script, LegacyScriptEvent &Event) { return true; }										   //Set up variables for the event to use
	virtual bool Script_ExecuteCmds(CScript *Script, LegacyScriptEvent &Event, legacy_scriptcmd_list &Cmds) { return false; }				   //Runs all commands in an event
	virtual bool Script_ExecuteCmd(CScript *Script, LegacyScriptEvent &Event, LegacyScriptCmd &Cmd, msstringlist &Params) { return false; } //Runs a single command
	virtual void CallScriptEventTimed(const char* EventName, float Delay);														   //Call all events with name after a delay
	virtual void CallScriptEvent(const char* EventName, msstringlist *Parameters = NULL);										   //Call all events with name right now
	virtual bool GetScriptVar(msstring &ParserName, msstringlist &Params, CScript *BaseScript, msstring &Return) { return false; } //Get script var from derived class
	virtual const char* GetFirstScriptVar(const char* EventName);																   //Get script var from first script
	virtual void SetScriptVar(const char* VarName, const char* Value);														   //Set var in first script
	virtual void SetScriptVar(const char* VarName, int iValue);																   //Set var in first script
	virtual void SetScriptVar(const char* VarName, float flValue);																   //Set var in first script
	virtual void Deactivate();																									   //Deallocate resources
	//virtual void Script_Use( CBaseEntity *pActivator, CBaseEntity *pCaller, int useType, float value );
	
	IScripted();
	virtual ~IScripted()
	{
	}

	scriptcmdname_list *m_pScriptCommands; //Master list of commands for this Scripted Ent
	mslist<CScript *> m_Scripts;		   //List of scripts
	msstring m_ReturnData;				   //Data returned from an event.  Reset at next CallScriptEvent()
};
constexpr const char* SKIP_STR = " \t"; //Whitespace characters

constexpr int DEFAULT_SCRIPT_ID = -1;
constexpr int PLAYER_SCRIPT_ID = -2;

// MiB 30NOV_2014 - Template structure for script function pointers
// Keeps track of whether or not it's a conditional command and how many times it's been referenced (for testing and curiosity)
// Template parameter is the function pointer signature Script.h has two examples (at the time of this writing) of how to
//		make a child of this.
template <typename P>
struct scriptcmdbase_t
{
private:
	bool m_Conditional;
	P pFunc;

	void Init(P func, bool Conditional)
	{
		m_Conditional = Conditional;
		pFunc = func;
	}

public:
	scriptcmdbase_t() { Init(NULL, false); }
	scriptcmdbase_t(P func) { Init(func, false); }
	scriptcmdbase_t(P func, bool Conditional) { Init(func, Conditional); }

	bool GetConditional() const { return m_Conditional; }
	P GetFunc() const { return pFunc; }
};

constexpr const char* RETURN_NOTHING_STR = "-NA-";
inline const char* RETURN_NOTHING() {
	return RETURN_NOTHING_STR;
}

inline const char* RETURN_ZERO() {
	return "0";
}

inline const char* RETURN_true(){
	return "1";
}

inline const char* RETURN_false() {
	return "0";
}

inline char * RETURN_FLOAT_PRECISION(const float &prfl) {
	msstring Return;
	_snprintf(Return, MSSTRING_SIZE, "%f", prfl); 
	return Return;
}

inline char* RETURN_FLOAT(const float& fl) {
	msstring Return;
	_snprintf(Return, MSSTRING_SIZE, "%.2f", fl);
	return Return;
}

inline char* RETURN_INT(const int& i) {
	msstring Return;
	_snprintf(Return, MSSTRING_SIZE, "%i", i);
	return Return;
}

inline char* RETURN_VECTOR(const Vector& vec){
	msstring Return;
	_snprintf(Return, MSSTRING_SIZE, "(%.2f,%.2f,%.2f)",vec.x, vec.y, vec.z);
	return Return;
}


inline const char* VecToString(const Vector& Vec, bool bAs2D)
{
	msstring Return;
	if (bAs2D)
		_snprintf(Return, MSSTRING_SIZE, "(%.2f,%.2f)", Vec.x, Vec.y);
	else
		_snprintf(Return, MSSTRING_SIZE, "(%.2f,%.2f,%.2f)", Vec.x, Vec.y, Vec.z);
	return Return;
}


inline const char* RETURN_POSITION(const char* prop, const char * name, const Vector& position, bool as2d = false) {

	msstring NameExt = name;
	msstring Prop = prop;

	if (Prop == name)	
		return VecToString(position, as2d); 

	NameExt = name;
	NameExt += ".x";

	if (Prop == NameExt) return RETURN_FLOAT(position.x);


	NameExt = name;
	NameExt += ".y";

	if (Prop == NameExt) return RETURN_FLOAT(position.y);

	NameExt = name;
	NameExt += ".y";

	if (Prop == NameExt) return RETURN_FLOAT(position.z);

	return RETURN_NOTHING();

}


inline const char* RETURN_ANGLE(const char* prop, const char* name, const Vector& angles, bool as2d = false) {

	msstring NameExt = name;
	msstring Prop = prop;

	if (Prop == name) {
		return VecToString(angles, as2d);
	}
	

	NameExt = name;
	NameExt += ".pitch";


	if (Prop == NameExt) {
		return RETURN_FLOAT(angles.x);
	}

	NameExt = name;
	NameExt += ".yaw";

	if (Prop == NameExt) {
		return RETURN_FLOAT(angles.y);
	}

	NameExt = name;
	NameExt += ".roll";


	if (Prop == NameExt) {
		return RETURN_FLOAT(angles.z);
	}

	return RETURN_NOTHING();
}


// Legacy compatibility typedefs
typedef LegacyScriptCmd scriptcmd_t;
typedef LegacyScriptEvent SCRIPT_EVENT;
typedef legacy_scriptcmd_list scriptcmd_list;
typedef LegacyEventScope eventscope_e;

// Legacy EVENTSCOPE macros - only define if not already defined by AngelScript headers
#ifndef EVENTSCOPE_SERVER

constexpr eventscope_e EVENTSCOPE_SERVER = LEGACY_EVENTSCOPE_SERVER;
constexpr eventscope_e EVENTSCOPE_CLIENT = LEGACY_EVENTSCOPE_CLIENT;
constexpr eventscope_e EVENTSCOPE_SHARED = LEGACY_EVENTSCOPE_SHARED;

#endif

#endif // MiB MAR2015_01 [LOCAL_PANEL] - Made this include-
