#include "IAngelScript.h"
#include "CAngelScript.h"
#include "CAngelScriptManager.h"
#include "angelscript.h"
#include "stackstring.h"
#include "global.h"
#include <ctime>

// Include for CBasePlayer if needed
#ifdef GAME_DLL
#include "player.h"
#endif

//==========================================================================
// Constructor
//==========================================================================
IAngelScript::IAngelScript()
{
    // Initialize with empty state
    m_Scripts.clear();
    m_ReturnData.clear();
}

//==========================================================================
// Destructor
//==========================================================================
IAngelScript::~IAngelScript()
{
    // Clean up all scripts
    Script_RemoveAll();
}

//==========================================================================
// Add a new script
//==========================================================================
CAngelScript* IAngelScript::Script_Add(const char* scriptName, CBaseEntity* pEntity)
{
    if (!scriptName || !scriptName[0])
        return nullptr;
        
    // Check if script already exists
    CAngelScript* pExisting = Script_Get(scriptName);
    if (pExisting)
        return pExisting;
        
    // Create new script
    CAngelScript* pScript = new CAngelScript();
    if (!pScript->Init(scriptName, pEntity))
    {
        delete pScript;
        return nullptr;
    }
    
    // Add to list
    m_Scripts.push_back(pScript);
    
    return pScript;
}

//==========================================================================
// Get script by name
//==========================================================================
CAngelScript* IAngelScript::Script_Get(const char* scriptName)
{
    if (!scriptName || !scriptName[0])
        return nullptr;
        
    for (size_t i = 0; i < m_Scripts.size(); i++)
    {
        if (m_Scripts[i] && strcmp(m_Scripts[i]->GetScriptName(), scriptName) == 0)
            return m_Scripts[i];
    }
    
    return nullptr;
}

//==========================================================================
// Remove script by index
//==========================================================================
void IAngelScript::Script_Remove(int idx)
{
    if (idx < 0 || idx >= (int)m_Scripts.size())
        return;
        
    CAngelScript* pScript = m_Scripts[idx];
    if (pScript)
    {
        pScript->Cleanup();
        delete pScript;
    }
    
    m_Scripts.erase(m_Scripts.begin() + idx);
}

//==========================================================================
// Remove all scripts
//==========================================================================
void IAngelScript::Script_RemoveAll()
{
    for (size_t i = 0; i < m_Scripts.size(); i++)
    {
        if (m_Scripts[i])
        {
            m_Scripts[i]->Cleanup();
            delete m_Scripts[i];
        }
    }
    m_Scripts.clear();
}

//==========================================================================
// Run all script events
//==========================================================================
void IAngelScript::RunScriptEvents(bool fOnlyRunNamedEvents)
{
    // Execute scheduled events for all scripts
    for (size_t i = 0; i < m_Scripts.size(); i++)
    {
        CAngelScript* pScript = m_Scripts[i];
        if (!pScript || !pScript->IsRunning())
            continue;
            
        // Check for auto-run events if not only running named events
        if (!fOnlyRunNamedEvents)
        {
            // Execute think event if present
            pScript->ExecuteEvent("GameThink");
        }
    }
}

//==========================================================================
// Execute event by name
//==========================================================================
bool IAngelScript::Script_ExecuteEvent(const char* eventName, ASEventParams* pParams)
{
    if (!eventName || !eventName[0])
        return false;
        
    bool bExecuted = false;
    
    // Clear return data before executing
    m_ReturnData.clear();
    
    // Execute on all scripts
    for (size_t i = 0; i < m_Scripts.size(); i++)
    {
        CAngelScript* pScript = m_Scripts[i];
        if (!pScript || !pScript->IsRunning())
            continue;
            
        if (pScript->ExecuteEvent(eventName, pParams))
        {
            bExecuted = true;
            
            // Capture return data from first successful execution
            if (m_ReturnData.empty())
                m_ReturnData = pScript->GetReturnData();
        }
    }
    
    return bExecuted;
}

//==========================================================================
// Execute specific function
//==========================================================================
bool IAngelScript::Script_ExecuteFunction(asIScriptFunction* pFunc, ASEventParams* pParams)
{
    if (!pFunc)
        return false;
        
    // Get a context
    asIScriptContext* pCtx = PrepareContext(pFunc);
    if (!pCtx)
        return false;
        
    // Set parameters
    if (!SetEventParameters(pCtx, pParams))
    {
        UnprepareContext(pCtx);
        return false;
    }
    
    // Execute
    int r = pCtx->Execute();
    bool bSuccess = (r == asEXECUTION_FINISHED);
    
    // Clean up
    UnprepareContext(pCtx);
    
    return bSuccess;
}

//==========================================================================
// Call script event immediately
//==========================================================================
void IAngelScript::CallScriptEvent(const char* eventName, ASEventParams* pParams)
{
    Script_ExecuteEvent(eventName, pParams);
}

//==========================================================================
// Call script event with delay
//==========================================================================
void IAngelScript::CallScriptEventTimed(const char* eventName, float delay)
{
    if (!eventName || !eventName[0] || delay < 0)
        return;
        
    // Schedule the event for future execution
    ScheduleEvent(eventName, delay);
}

//==========================================================================
// Call script event with delay and parameters
//==========================================================================
void IAngelScript::CallScriptEventTimed(const char* eventName, float delay, ASEventParams* pParams)
{
    if (!eventName || !eventName[0] || delay < 0)
        return;
        
    // Schedule the event for future execution with parameters
    ScheduleEvent(eventName, delay, pParams);
}

//==========================================================================
// Call script event repeatedly with delay
//==========================================================================
void IAngelScript::CallScriptEventRepeating(const char* eventName, float delay, float repeatDelay, ASEventParams* pParams)
{
    if (!eventName || !eventName[0] || delay < 0 || repeatDelay <= 0)
        return;
        
    // Schedule the repeating event
    ScheduleRepeatingEvent(eventName, delay, repeatDelay, pParams);
}

//==========================================================================
// Check if any script has event
//==========================================================================
bool IAngelScript::HasScriptEvent(const char* eventName)
{
    if (!eventName || !eventName[0])
        return false;
        
    for (size_t i = 0; i < m_Scripts.size(); i++)
    {
        CAngelScript* pScript = m_Scripts[i];
        if (pScript && pScript->HasEvent(eventName))
            return true;
    }
    
    return false;
}

//==========================================================================
// Cancel timed events
//==========================================================================
void IAngelScript::CancelTimedEvents(const char* eventName)
{
    if (!eventName || !eventName[0])
    {
        // Cancel all timed events
        m_TimedEvents.clear();
        return;
    }
    
    // Cancel specific event by name
    for (int i = (int)m_TimedEvents.size() - 1; i >= 0; i--)
    {
        if (m_TimedEvents[i].eventName == eventName)
        {
            m_TimedEvents.erase(m_TimedEvents.begin() + i);
        }
    }
}

//==========================================================================
// Get script variable
//==========================================================================
const char* IAngelScript::GetScriptVar(const char* varName)
{
    if (!varName || !varName[0])
        return "";
        
    // Check all scripts for variable
    for (size_t i = 0; i < m_Scripts.size(); i++)
    {
        CAngelScript* pScript = m_Scripts[i];
        if (!pScript)
            continue;
            
        const char* value = pScript->GetVar(varName);
        if (value && value[0])
            return value;
    }
    
    return "";
}

//==========================================================================
// Set script variable (string)
//==========================================================================
void IAngelScript::SetScriptVar(const char* varName, const char* value)
{
    if (!varName || !varName[0])
        return;
        
    // Set on all scripts
    for (size_t i = 0; i < m_Scripts.size(); i++)
    {
        CAngelScript* pScript = m_Scripts[i];
        if (pScript)
            pScript->SetVar(varName, value);
    }
}

//==========================================================================
// Set script variable (int)
//==========================================================================
void IAngelScript::SetScriptVar(const char* varName, int value)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", value);
    SetScriptVar(varName, buffer);
}

//==========================================================================
// Set script variable (float)
//==========================================================================
void IAngelScript::SetScriptVar(const char* varName, float value)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.2f", value);
    SetScriptVar(varName, buffer);
}

//==========================================================================
// Initialize HUD for player
//==========================================================================
void IAngelScript::Script_InitHUD(CBasePlayer* pPlayer)
{
    // Send script initialization commands to client
    // This will be implemented when we handle client-server synchronization
}

//==========================================================================
// Send command to client
//==========================================================================
void IAngelScript::Script_SendCmd(const char* scriptName, const char* msgType, 
                                  const char* msgTarget, ASEventParams* pParams)
{
    // This will be implemented when we handle client-server synchronization
}

//==========================================================================
// Deactivate all scripts
//==========================================================================
void IAngelScript::Deactivate()
{
    Script_RemoveAll();
    m_ReturnData.clear();
    m_TimedEvents.clear();  // Clear any pending timed events
}

//==========================================================================
// Think - maintenance
//==========================================================================
void IAngelScript::Think()
{
    // Process timed events
    ProcessTimedEvents();
}

//==========================================================================
// Legacy compatibility - get first script var
//==========================================================================
const char* IAngelScript::GetFirstScriptVar(const char* varName)
{
    if (m_Scripts.empty())
        return "";
        
    CAngelScript* pScript = m_Scripts[0];
    return pScript ? pScript->GetVar(varName) : "";
}

//==========================================================================
// Legacy compatibility - complex var getter
//==========================================================================
bool IAngelScript::GetScriptVar(msstring& parserName, msstringlist& params, 
                                CAngelScript* pBaseScript, msstring& returnVal)
{
    // This is for legacy script variable parsing
    // Will be implemented based on legacy requirements
    return false;
}

//==========================================================================
// Prepare context for execution
//==========================================================================
asIScriptContext* IAngelScript::PrepareContext(asIScriptFunction* pFunc)
{
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (!pManager)
        return nullptr;
        
    asIScriptContext* pCtx = pManager->AcquireContext();
    if (!pCtx)
        return nullptr;
        
    int r = pCtx->Prepare(pFunc);
    if (r < 0)
    {
        pManager->ReleaseContext(pCtx);
        return nullptr;
    }
    
    return pCtx;
}

//==========================================================================
// Release context after execution
//==========================================================================
void IAngelScript::UnprepareContext(asIScriptContext* pCtx)
{
    if (!pCtx)
        return;
        
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (pManager)
        pManager->ReleaseContext(pCtx);
}

//==========================================================================
// Set event parameters on context
//==========================================================================
bool IAngelScript::SetEventParameters(asIScriptContext* pCtx, ASEventParams* pParams)
{
    if (!pCtx)
        return false;
        
    // If no parameters, we're done
    if (!pParams || pParams->Count() == 0)
        return true;
        
    // Set parameters based on function signature
    asIScriptFunction* pFunc = pCtx->GetFunction();
    // Validate pointer is in a reasonable range (not an error code)
    if (!pFunc || reinterpret_cast<uintptr_t>(pFunc) <= 0x10000)
        return false;
        
    int paramCount = pFunc->GetParamCount();
    for (int i = 0; i < paramCount && i < (int)pParams->Count(); i++)
    {
        // For now, pass all parameters as strings
        // Later we can add type detection
        int r = pCtx->SetArgObject(i, (void*)pParams->Get(i));
        if (r < 0)
            return false;
    }
    
    return true;
}

//==========================================================================
// Get current game time (shared code compatible)
//==========================================================================
float IAngelScript::GetCurrentTime()
{
    // For shared code, we need to handle timing differently than server/client
    // Use a simple clock-based timing system that works across all contexts
    static clock_t startTime = clock();
    clock_t currentTime = clock();
    return (float)(currentTime - startTime) / CLOCKS_PER_SEC;
}

//==========================================================================
// Schedule an event for future execution
//==========================================================================
void IAngelScript::ScheduleEvent(const char* eventName, float delay, ASEventParams* pParams, CAngelScript* pTargetScript)
{
    if (!eventName || !eventName[0] || delay < 0)
        return;
        
    ASTimedEvent timedEvent;
    timedEvent.eventName = eventName;
    timedEvent.fExecuteTime = GetCurrentTime() + delay;
    timedEvent.pTargetScript = pTargetScript;
    timedEvent.bRepeat = false;
    timedEvent.fRepeatDelay = 0;
    
    // Copy parameters if provided
    if (pParams)
    {
        for (size_t i = 0; i < pParams->Count(); i++)
        {
            timedEvent.params.Add(pParams->Get(i));
        }
    }
    
    m_TimedEvents.push_back(timedEvent);
}

//==========================================================================
// Schedule a repeating event
//==========================================================================
void IAngelScript::ScheduleRepeatingEvent(const char* eventName, float delay, float repeatDelay, 
                                         ASEventParams* pParams, CAngelScript* pTargetScript)
{
    if (!eventName || !eventName[0] || delay < 0 || repeatDelay <= 0)
        return;
        
    ASTimedEvent timedEvent;
    timedEvent.eventName = eventName;
    timedEvent.fExecuteTime = GetCurrentTime() + delay;
    timedEvent.pTargetScript = pTargetScript;
    timedEvent.bRepeat = true;
    timedEvent.fRepeatDelay = repeatDelay;
    
    // Copy parameters if provided
    if (pParams)
    {
        for (size_t i = 0; i < pParams->Count(); i++)
        {
            timedEvent.params.Add(pParams->Get(i));
        }
    }
    
    m_TimedEvents.push_back(timedEvent);
}

//==========================================================================
// Process pending timed events
//==========================================================================
void IAngelScript::ProcessTimedEvents()
{
    if (m_TimedEvents.empty())
        return;
        
    float currentTime = GetCurrentTime();
    
    // Process events in reverse order so we can safely remove them
    for (int i = (int)m_TimedEvents.size() - 1; i >= 0; i--)
    {
        ASTimedEvent& timedEvent = m_TimedEvents[i];
        
        // Check if this event is ready to execute
        if (currentTime >= timedEvent.fExecuteTime)
        {
            // Execute the event
            if (timedEvent.pTargetScript)
            {
                // Execute on specific script
                if (timedEvent.params.Count() > 0)
                    timedEvent.pTargetScript->ExecuteEvent(timedEvent.eventName.c_str(), &timedEvent.params);
                else
                    timedEvent.pTargetScript->ExecuteEvent(timedEvent.eventName.c_str());
            }
            else
            {
                // Execute on all scripts
                if (timedEvent.params.Count() > 0)
                    Script_ExecuteEvent(timedEvent.eventName.c_str(), &timedEvent.params);
                else
                    Script_ExecuteEvent(timedEvent.eventName.c_str());
            }
            
            // Handle repeating events
            if (timedEvent.bRepeat)
            {
                // Schedule next execution
                timedEvent.fExecuteTime = currentTime + timedEvent.fRepeatDelay;
            }
            else
            {
                // Remove one-time event
                m_TimedEvents.erase(m_TimedEvents.begin() + i);
            }
        }
    }
}