#ifndef IANGELSCRIPT_H
#define IANGELSCRIPT_H

//==========================================================================
// IAngelScript - Interface for AngelScript-powered entities
// 
// This replaces the old IScripted interface with modern AngelScript support
// Maintains compatibility with existing systems while enabling new features
//==========================================================================

#include <vector>
#include <string>
#include "stackstring.h"

// Forward declarations
class CAngelScript;
class CBaseEntity;
class asIScriptContext;
class asIScriptFunction;

// Event parameter structure
struct ASEventParams
{
    std::vector<std::string> params;
    
    void Add(const char* param) { params.push_back(param); }
    void Add(const std::string& param) { params.push_back(param); }
    void Add(int value) { params.push_back(std::to_string(value)); }
    void Add(float value) { params.push_back(std::to_string(value)); }
    
    size_t Count() const { return params.size(); }
    const char* Get(size_t index) const 
    { 
        return (index < params.size()) ? params[index].c_str() : "";
    }
};

// Event scope enumeration (same as original)
enum ASEventScope
{
    AS_EVENTSCOPE_SERVER,
    AS_EVENTSCOPE_CLIENT,
    AS_EVENTSCOPE_SHARED
};

// Script variable structure
struct ASScriptVar
{
    std::string name;
    std::string value;
    
    ASScriptVar() {}
    ASScriptVar(const char* n, const char* v) : name(n), value(v) {}
};

// AngelScript event structure
struct ASScriptEvent
{
    std::string name;                    // Event name
    asIScriptFunction* pFunction;        // AngelScript function pointer
    ASEventScope scope;                  // Execution scope
    float fNextExecutionTime;            // For delayed/repeated execution
    float fRepeatDelay;                  // Repeat interval
    std::vector<float> timedExecutions;  // Scheduled execution times
    
    ASScriptEvent() : pFunction(nullptr), scope(AS_EVENTSCOPE_SERVER), 
                      fNextExecutionTime(0), fRepeatDelay(0) {}
};

// Timed event for scheduling future execution
struct ASTimedEvent
{
    std::string eventName;               // Name of event to execute
    float fExecuteTime;                  // When to execute (game time)
    ASEventParams params;                // Parameters to pass to event
    CAngelScript* pTargetScript;         // Specific script target (nullptr for all)
    bool bRepeat;                        // Whether this is a repeating event
    float fRepeatDelay;                  // Delay between repeats
    
    ASTimedEvent() : fExecuteTime(0), pTargetScript(nullptr), bRepeat(false), fRepeatDelay(0) {}
    ASTimedEvent(const char* name, float execTime) : eventName(name), fExecuteTime(execTime), 
                                                     pTargetScript(nullptr), bRepeat(false), fRepeatDelay(0) {}
};

//==========================================================================
// IAngelScript Interface
//==========================================================================
class IAngelScript
{
public:
    IAngelScript();
    virtual ~IAngelScript();
    
    // Script management
    virtual CAngelScript* Script_Add(const char* scriptName, CBaseEntity* pEntity);
    virtual CAngelScript* Script_Get(const char* scriptName);
    virtual void Script_Remove(int idx);
    virtual void Script_RemoveAll();
    
    // Script execution
    virtual void RunScriptEvents(bool fOnlyRunNamedEvents = false);
    virtual bool Script_ExecuteEvent(const char* eventName, ASEventParams* pParams = nullptr);
    virtual bool Script_ExecuteFunction(asIScriptFunction* pFunc, ASEventParams* pParams = nullptr);
    
    // Event management
    virtual void CallScriptEvent(const char* eventName, ASEventParams* pParams = nullptr);
    virtual void CallScriptEventTimed(const char* eventName, float delay);
    virtual void CallScriptEventTimed(const char* eventName, float delay, ASEventParams* pParams);
    virtual void CallScriptEventRepeating(const char* eventName, float delay, float repeatDelay, ASEventParams* pParams = nullptr);
    virtual bool HasScriptEvent(const char* eventName);
    virtual void CancelTimedEvents(const char* eventName = nullptr);  // Cancel specific or all timed events
    
    // Variable management
    virtual const char* GetScriptVar(const char* varName);
    virtual void SetScriptVar(const char* varName, const char* value);
    virtual void SetScriptVar(const char* varName, int value);
    virtual void SetScriptVar(const char* varName, float value);
    
    // Client synchronization
    virtual void Script_InitHUD(class CBasePlayer* pPlayer);
    virtual void Script_SendCmd(const char* scriptName, const char* msgType, 
                                const char* msgTarget, ASEventParams* pParams);
    
    // Utility
    virtual void Deactivate();
    virtual void Think();  // Called each frame for maintenance
    
    // Legacy compatibility helpers
    virtual const char* GetFirstScriptVar(const char* varName);
    virtual bool GetScriptVar(msstring& parserName, msstringlist& params, 
                             CAngelScript* pBaseScript, msstring& returnVal);
    
protected:
    std::vector<CAngelScript*> m_Scripts;     // Active scripts
    std::string m_ReturnData;                 // Data returned from events
    std::vector<ASTimedEvent> m_TimedEvents;  // Pending timed events
    
    // Internal helpers
    virtual asIScriptContext* PrepareContext(asIScriptFunction* pFunc);
    virtual void UnprepareContext(asIScriptContext* pCtx);
    virtual bool SetEventParameters(asIScriptContext* pCtx, ASEventParams* pParams);
    
    // Timing helpers
    virtual float GetCurrentTime();           // Get current game time (shared code compatible)
    virtual void ProcessTimedEvents();        // Process pending timed events
    virtual void ScheduleEvent(const char* eventName, float delay, ASEventParams* pParams = nullptr, 
                              CAngelScript* pTargetScript = nullptr);
    virtual void ScheduleRepeatingEvent(const char* eventName, float delay, float repeatDelay, 
                                       ASEventParams* pParams = nullptr, CAngelScript* pTargetScript = nullptr);
};

// Conversion helpers for legacy code
// NOTE: These macros are also defined in iscript.h with conditional compilation
// to avoid redefinition conflicts during the transition period
#define EVENTSCOPE_SERVER AS_EVENTSCOPE_SERVER
#define EVENTSCOPE_CLIENT AS_EVENTSCOPE_CLIENT
#define EVENTSCOPE_SHARED AS_EVENTSCOPE_SHARED

#endif // IANGELSCRIPT_H