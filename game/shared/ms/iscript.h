#ifndef ISCRIPT_H // MiB MAR2015_01 [LOCAL_PANEL] - Made this include-
#define ISCRIPT_H

/*
	Dogg - Interface for a scripted entity
*/
class CScript;
class CBaseEntity;

struct scriptvar_t
{
	scriptvar_t() {}
	scriptvar_t(msstring_ref name, msstring_ref value)
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

	scriptvar_t *FindVar(msstring_ref Name);
	msstring_ref GetVar(msstring_ref Name);
	scriptvar_t *SetVar(msstring_ref Name, msstring_ref Value);
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
	scriptcmdname_t(msstring_ref Name)
	{
		m_Name = Name;
		m_Conditional = false;
	}
	scriptcmdname_t(msstring_ref Name, bool Conditional)
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
struct scriptcmd_t
{
	scriptcmd_t() { init(); }
	scriptcmd_t(msstring_ref Name, bool Conditional = false)
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
		mslist<scriptcmd_t> m_Cmds; //The commands of this command list
		~cmdlist_t()
		{
		}
	};
	cmdlist_t m_IfCmds;			  //If this is a conditional command, here is the group of comamnds to be executed if true
	mslist<cmdlist_t> m_ElseCmds; //If this is a conditional command, here is the group of comamnds to be executed if false
	~scriptcmd_t()
	{
	}
};
typedef scriptcmd_t::cmdlist_t scriptcmd_list;

enum eventscope_e
{
	EVENTSCOPE_SERVER,
	EVENTSCOPE_CLIENT,
	EVENTSCOPE_SHARED,
};

//The basic 'event' of a script.  Contains a list of commands
struct SCRIPT_EVENT : public IVariables
{
	string_i Name;				   //Event name
	scriptcmd_list Commands;	   //The comamnds in this event
	msstringlist *Params;		   //The parameters passed to this event (Can be NULL)
	float fNextExecutionTime,	   //Next time to be run with repeatdelay
		fRepeatDelay;			   //Repeat every x seconds
	eventscope_e Scope;			   //Scope (delete events that aren't in the right scope... client events on server and vica versa)
	mslist<float> TimedExecutions; //Clock times in the future that this event should be executed
	bool bFullStop;				   //MiB DEC2014_07 - "exitevent" command (exit.rtf)

	void SetLocal(msstring_ref Name, msstring_ref Value) { SetVar(Name, Value); }
	msstring_ref GetLocal(msstring_ref Name);
	~SCRIPT_EVENT()
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
	virtual CScript *Script_Add(string_i ScriptName, CBaseEntity *pEntity); //Adds a new script to the list
	virtual CScript *Script_Get(string_i ScriptName);
	virtual void Script_Remove(int idx);																						   //Removes a script
	virtual void Script_InitHUD(class CBasePlayer *pPlayer);																	   //Called when a player joins the game
	virtual void Script_Setup() {}																								   //Ties m_pScriptCommands to a global somewhere
	virtual int Script_ParseLine(CScript *Script, msstring_ref pszCommandLine, scriptcmd_t &Cmd);								   //Parses a line of script text and returns the command type
	virtual void RunScriptEvents(bool fOnlyRunNamedEvents = false);																   //Runs all events
	virtual bool Script_ExecuteEvent(CScript *Script, SCRIPT_EVENT &Event, msstringlist *Parameters = NULL) { return false; }	   //Runs an event
	virtual bool Script_SetupEvent(CScript *Script, SCRIPT_EVENT &Event) { return true; }										   //Set up variables for the event to use
	virtual bool Script_ExecuteCmds(CScript *Script, SCRIPT_EVENT &Event, scriptcmd_list &Cmds) { return false; }				   //Runs all commands in an event
	virtual bool Script_ExecuteCmd(CScript *Script, SCRIPT_EVENT &Event, scriptcmd_t &Cmd, msstringlist &Params) { return false; } //Runs a single command
	virtual void CallScriptEventTimed(msstring_ref EventName, float Delay);														   //Call all events with name after a delay
	virtual void CallScriptEvent(msstring_ref EventName, msstringlist *Parameters = NULL);										   //Call all events with name right now
	virtual bool GetScriptVar(msstring &ParserName, msstringlist &Params, CScript *BaseScript, msstring &Return) { return false; } //Get script var from derived class
	virtual msstring_ref GetFirstScriptVar(msstring_ref EventName);																   //Get script var from first script
	virtual void SetScriptVar(msstring_ref VarName, msstring_ref Value);														   //Set var in first script
	virtual void SetScriptVar(msstring_ref VarName, int iValue);																   //Set var in first script
	virtual void SetScriptVar(msstring_ref VarName, float flValue);																   //Set var in first script
	virtual void Deactivate();																									   //Deallocate resources
	//virtual void Script_Use( CBaseEntity *pActivator, CBaseEntity *pCaller, int useType, float value );
	IScripted();

	scriptcmdname_list *m_pScriptCommands; //Master list of commands for this Scripted Ent
	mslist<CScript *> m_Scripts;		   //List of scripts
	msstring m_ReturnData;				   //Data returned from an event.  Reset at next CallScriptEvent()
};
#define SKIP_STR " \t" //Whitespace characters

#define DEFAULT_SCRIPT_ID -1
#define PLAYER_SCRIPT_ID -2

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

//Definitions for functions like GetProp(), CLGetCurrentTempEntProp()
//Returns different types of data as a string.  Requires static msstring Return
#define RETURN_TRUE \
	{               \
		return "1"; \
	}
#define RETURN_FALSE \
	{                \
		return "0";  \
	}
#define RETURN_FLOAT_PRECISION(a) \
	{                             \
		_snprintf(Return, MSSTRING_SIZE, "%f", a); \
		return Return;            \
	}
#define RETURN_FLOAT(a)             \
	{                               \
		_snprintf(Return, MSSTRING_SIZE, "%.2f", a); \
		return Return;              \
	}
#define RETURN_INT(a)             \
	{                             \
		_snprintf(Return, MSSTRING_SIZE, "%i", a); \
		return Return;            \
	}
#define RETURN_VECTOR(a)                                    \
	{                                                       \
		_snprintf(Return, MSSTRING_SIZE, "(%.2f,%.2f,%.2f)", a.x, a.y, a.z); \
		return Return;                                      \
	}
#define RETURN_POSITION(name, position)              \
	{                                                \
		if (Prop == name)                            \
			return (Return = VecToString(position)); \
		else if (Prop == name ".x")                  \
			RETURN_FLOAT(position.x)                 \
		else if (Prop == name ".y")                  \
			RETURN_FLOAT(position.y)                 \
		else if (Prop == name ".z")                  \
			RETURN_FLOAT(position.z)                 \
	}
#define RETURN_ANGLE(name, angles)                 \
	{                                              \
		if (Prop == name)                          \
			return (Return = VecToString(angles)); \
		else if (Prop == name ".pitch")            \
			RETURN_FLOAT(angles.x)                 \
		else if (Prop == name ".yaw")              \
			RETURN_FLOAT(angles.y)                 \
		else if (Prop == name ".roll")             \
			RETURN_FLOAT(angles.z)                 \
	}
#define RETURN_NOTHING_STR "�NA�"
#define RETURN_NOTHING return RETURN_NOTHING_STR
#define RETURN_ZERO return "0"

#endif // MiB MAR2015_01 [LOCAL_PANEL] - Made this include-
