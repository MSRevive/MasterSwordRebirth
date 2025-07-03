#ifndef CANGELSCRIPT_H
#define CANGELSCRIPT_H

//==========================================================================
// CAngelScript - Base AngelScript class
// 
// Replaces CScript with AngelScript-powered functionality
// Each script file becomes an AngelScript module with its own namespace
//==========================================================================

#include "IAngelScript.h"
#include <map>
#include <memory>

// Forward declarations
class asIScriptModule;
class asIScriptObject;
class CBaseEntity;

//==========================================================================
// CAngelScript Class
//==========================================================================
class CAngelScript
{
public:
    CAngelScript();
    virtual ~CAngelScript();
    
    // Initialization
    bool Init(const char* scriptName, CBaseEntity* pOwner);
    void Cleanup();
    
    // Script loading and compilation
    bool LoadScript(const char* fileName);
    bool CompileScript(const char* scriptContent);
    bool IsValid() const { return m_pModule != nullptr; }
    
    // Event management
    bool RegisterEvent(const char* eventName, const char* functionDecl);
    ASScriptEvent* GetEvent(const char* eventName);
    bool HasEvent(const char* eventName) const;
    void RemoveEvent(const char* eventName);
    
    // Event execution
    bool ExecuteEvent(const char* eventName, ASEventParams* pParams = nullptr);
    bool ExecuteFunction(asIScriptFunction* pFunc, ASEventParams* pParams = nullptr);
    
    // Variable management
    ASScriptVar* FindVar(const char* name);
    const char* GetVar(const char* name);
    void SetVar(const char* name, const char* value);
    void SetVar(const char* name, int value);
    void SetVar(const char* name, float value);
    
    // Properties
    const char* GetScriptName() const { return m_ScriptName.c_str(); }
    CBaseEntity* GetOwner() const { return m_pOwner; }
    asIScriptModule* GetModule() const { return m_pModule; }
    
    // Script state
    bool IsRunning() const { return m_bRunning; }
    void SetRunning(bool bRunning) { m_bRunning = bRunning; }
    
    // Scope management
    ASEventScope GetScope() const { return m_Scope; }
    void SetScope(ASEventScope scope) { m_Scope = scope; }
    
    // Error handling
    const char* GetLastError() const { return m_LastError.c_str(); }
    bool HasError() const { return !m_LastError.empty(); }
    
    // Legacy compatibility
    void RunScriptEventByName(const char* eventName, msstringlist* legacyParams = nullptr);
    const char* GetReturnData() const { return m_ReturnData.c_str(); }
    void SetReturnData(const char* data) { m_ReturnData = data; }
    
protected:
    // Core properties
    std::string m_ScriptName;                           // Script file name
    CBaseEntity* m_pOwner;                              // Entity that owns this script
    asIScriptModule* m_pModule;                         // AngelScript module
    asIScriptObject* m_pScriptObject;                   // Main script object instance
    
    // Event management
    std::map<std::string, ASScriptEvent> m_Events;     // Registered events
    std::map<std::string, float> m_EventTimers;        // Event execution timers
    
    // Variable storage
    std::vector<ASScriptVar> m_Variables;              // Script variables
    
    // State
    bool m_bRunning;                                    // Is script active
    ASEventScope m_Scope;                               // Execution scope
    std::string m_LastError;                            // Last error message
    std::string m_ReturnData;                           // Data returned from events
    
    // Internal methods
    bool ParseScriptForEvents();
    bool CreateScriptObject();
    void SetError(const char* error);
    asIScriptContext* GetContext();
    void ReleaseContext(asIScriptContext* pCtx);
    
    // Legacy conversion helpers
    bool ConvertLegacyParams(msstringlist* legacyParams, ASEventParams& asParams);
};

//==========================================================================
// Global script helpers
//==========================================================================

// Find script by name in entity
CAngelScript* FindScript(CBaseEntity* pEntity, const char* scriptName);

// Execute event on all scripts of entity
bool ExecuteEventOnScripts(CBaseEntity* pEntity, const char* eventName, 
                          ASEventParams* pParams = nullptr);

// Legacy compatibility macros
#define SCRIPT_EVENT ASScriptEvent
#define scriptcmd_t void  // No longer used with AngelScript

#endif // CANGELSCRIPT_H