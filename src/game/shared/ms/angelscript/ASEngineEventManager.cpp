//==========================================================================
// AngelScript Engine Event Manager Implementation
//==========================================================================

#include "ASEngineEventManager.h"
#include "CAngelScriptManager.h"
#include "../mslogger.h"
#include <asbind20/asbind.hpp>
#include <angelscript.h>
#include <algorithm>

// Include vector.h directly to avoid mathlib.h macro conflicts
// We need the vec_t typedef but not the DotProduct macro
typedef float vec_t;
#include "../../../server/hl/vector.h"

// Static instance
ASEngineEventManager* ASEngineEventManager::s_pInstance = nullptr;

//==========================================================================
// Constructor
//==========================================================================
ASEngineEventManager::ASEngineEventManager()
    : m_pEngine(nullptr)
    , m_bInitialized(false)
{
}

//==========================================================================
// Destructor
//==========================================================================
ASEngineEventManager::~ASEngineEventManager()
{
    Destroy();
}

//==========================================================================
// Singleton Access
//==========================================================================
ASEngineEventManager* ASEngineEventManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new ASEngineEventManager();
    }
    return s_pInstance;
}

//==========================================================================
// Shutdown Singleton
//==========================================================================
void ASEngineEventManager::Shutdown()
{
    if (s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

//==========================================================================
// Initialize
//==========================================================================
bool ASEngineEventManager::Initialize(asIScriptEngine* pEngine)
{
    if (m_bInitialized)
        return true;
        
    if (!pEngine)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::Initialize: NULL engine pointer");
        return false;
    }
    
    m_pEngine = pEngine;
    m_bInitialized = true;
    
    MS_ANGEL_INFO("ASEngineEventManager initialized successfully");
    return true;
}

//==========================================================================
// Destroy
//==========================================================================
void ASEngineEventManager::Destroy()
{
    if (!m_bInitialized)
        return;
        
    // Release all function references
    for (auto& handler : m_EventHandlers)
    {
        if (handler.pFunction)
        {
            handler.pFunction->Release();
        }
    }
    
    m_EventHandlers.clear();
    m_pEngine = nullptr;
    m_bInitialized = false;
    
    MS_ANGEL_INFO("ASEngineEventManager destroyed");
}

//==========================================================================
// Register Event Handler
//==========================================================================
bool ASEngineEventManager::RegisterEventHandler(EngineEventType eventType, asIScriptFunction* pFunction, const char* szModuleName)
{
    if (!m_bInitialized || !pFunction || !szModuleName)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::RegisterEventHandler: Invalid parameters");
        return false;
    }
    
    // Check if handler already exists for this module and event type
    for (const auto& handler : m_EventHandlers)
    {
        if (handler.eventType == eventType && handler.moduleName == szModuleName)
        {
            MS_ANGEL_INFO("ASEngineEventManager: Replacing existing handler for %s in module %s", 
                         GetEventTypeName(eventType), szModuleName);
            // Release old function and remove it
            const_cast<EngineEventHandler&>(handler).pFunction->Release();
            m_EventHandlers.erase(std::remove_if(m_EventHandlers.begin(), m_EventHandlers.end(),
                [eventType, szModuleName](const EngineEventHandler& h) {
                    return h.eventType == eventType && h.moduleName == szModuleName;
                }), m_EventHandlers.end());
            break;
        }
    }
    
    // Add reference to the function to keep it alive
    pFunction->AddRef();
    
    // Add new handler
    m_EventHandlers.emplace_back(eventType, pFunction, szModuleName);
    
    MS_ANGEL_INFO("ASEngineEventManager: Registered handler for %s in module %s", 
                 GetEventTypeName(eventType), szModuleName);
    return true;
}

//==========================================================================
// Unregister Event Handler
//==========================================================================
void ASEngineEventManager::UnregisterEventHandler(EngineEventType eventType, const char* szModuleName)
{
    if (!m_bInitialized || !szModuleName)
        return;
        
    auto it = std::remove_if(m_EventHandlers.begin(), m_EventHandlers.end(),
        [eventType, szModuleName](EngineEventHandler& handler) {
            if (handler.eventType == eventType && handler.moduleName == szModuleName)
            {
                handler.pFunction->Release();
                return true;
            }
            return false;
        });
    
    if (it != m_EventHandlers.end())
    {
        m_EventHandlers.erase(it, m_EventHandlers.end());
        MS_ANGEL_INFO("ASEngineEventManager: Unregistered handler for %s in module %s", 
                     GetEventTypeName(eventType), szModuleName);
    }
}

//==========================================================================
// Unregister All Handlers for Module
//==========================================================================
void ASEngineEventManager::UnregisterAllHandlers(const char* szModuleName)
{
    if (!m_bInitialized || !szModuleName)
        return;
        
    auto it = std::remove_if(m_EventHandlers.begin(), m_EventHandlers.end(),
        [szModuleName](EngineEventHandler& handler) {
            if (handler.moduleName == szModuleName)
            {
                handler.pFunction->Release();
                return true;
            }
            return false;
        });
    
    if (it != m_EventHandlers.end())
    {
        m_EventHandlers.erase(it, m_EventHandlers.end());
        MS_ANGEL_INFO("ASEngineEventManager: Unregistered all handlers for module %s", szModuleName);
    }
}

//==========================================================================
// Fire Player Connect Event
//==========================================================================
void ASEngineEventManager::FirePlayerConnectEvent(const char* szPlayerName, const char* szSteamID)
{
    if (!m_bInitialized)
        return;
        
    std::vector<std::string> params;
    params.push_back(szPlayerName ? szPlayerName : "");
    params.push_back(szSteamID ? szSteamID : "");
    
    MS_ANGEL_INFO("ASEngineEventManager: Firing PlayerConnect event for %s", szPlayerName ? szPlayerName : "Unknown");
    DispatchEvent(EngineEventType::PLAYER_CONNECT, params);
}

//==========================================================================
// Fire Player Disconnect Event
//==========================================================================
void ASEngineEventManager::FirePlayerDisconnectEvent(const char* szPlayerName, const char* szSteamID)
{
    if (!m_bInitialized)
        return;
        
    std::vector<std::string> params;
    params.push_back(szPlayerName ? szPlayerName : "");
    params.push_back(szSteamID ? szSteamID : "");
    
    MS_ANGEL_INFO("ASEngineEventManager: Firing PlayerDisconnect event for %s", szPlayerName ? szPlayerName : "Unknown");
    DispatchEvent(EngineEventType::PLAYER_DISCONNECT, params);
}

//==========================================================================
// Fire Monster Killed Event
//==========================================================================
void ASEngineEventManager::FireMonsterKilledEvent(const char* szMonsterName, const char* szKillerName, float fPosX, float fPosY, float fPosZ)
{
    if (!m_bInitialized)
        return;
        
    std::vector<std::string> params;
    params.push_back(szMonsterName ? szMonsterName : "");
    params.push_back(szKillerName ? szKillerName : "");
    params.push_back(std::to_string(fPosX));
    params.push_back(std::to_string(fPosY));
    params.push_back(std::to_string(fPosZ));
    
    MS_ANGEL_INFO("ASEngineEventManager: Firing MonsterKilled event - %s killed by %s at (%.1f, %.1f, %.1f)", 
                 szMonsterName ? szMonsterName : "Unknown", szKillerName ? szKillerName : "Unknown", fPosX, fPosY, fPosZ);
    DispatchEvent(EngineEventType::MONSTER_KILLED, params);
}

//==========================================================================
// Fire Treasure Spawned Event
//==========================================================================
void ASEngineEventManager::FireTreasureSpawnedEvent(const char* szTreasureName, float fPosX, float fPosY, float fPosZ)
{
    if (!m_bInitialized)
        return;
        
    std::vector<std::string> params;
    params.push_back(szTreasureName ? szTreasureName : "");
    params.push_back(std::to_string(fPosX));
    params.push_back(std::to_string(fPosY));
    params.push_back(std::to_string(fPosZ));
    
    MS_ANGEL_INFO("ASEngineEventManager: Firing TreasureSpawned event for %s", szTreasureName ? szTreasureName : "Unknown");
    DispatchEvent(EngineEventType::TREASURE_SPAWNED, params);
}

//==========================================================================
// Fire Player Spawned Event
//==========================================================================
void ASEngineEventManager::FirePlayerSpawnedEvent(const char* szPlayerName)
{
    if (!m_bInitialized)
        return;
        
    std::vector<std::string> params;
    params.push_back(szPlayerName ? szPlayerName : "");
    
    MS_ANGEL_INFO("ASEngineEventManager: Firing PlayerSpawned event for %s", szPlayerName ? szPlayerName : "Unknown");
    DispatchEvent(EngineEventType::PLAYER_SPAWNED, params);
}

//==========================================================================
// Helper function to determine if parameter should be passed by reference
//==========================================================================
bool ASEngineEventManager::IsParameterByReference(asIScriptFunction* pFunction, int paramIndex)
{
    if (!pFunction || paramIndex >= pFunction->GetParamCount())
        return false;
        
    int typeId;
    asDWORD flags;
    const char* name;
    const char* defaultArg;
    
    int r = pFunction->GetParam(paramIndex, &typeId, &flags, &name, &defaultArg);
    if (r < 0)
        return false;
    
    // Check if parameter has reference flag
    return (flags & asTM_INREF) != 0;
}

//==========================================================================
// Dispatch Event to Handlers
//==========================================================================
void ASEngineEventManager::DispatchEvent(EngineEventType eventType, const std::vector<std::string>& parameters)
{
    if (!m_bInitialized)
        return;
        
    MS_ANGEL_INFO("ASEngineEventManager::DispatchEvent - Event: %s, Parameters: %zu", 
                  GetEventTypeName(eventType), parameters.size());
    
    // Log parameters
    for (size_t i = 0; i < parameters.size(); i++)
    {
        MS_ANGEL_INFO("  Parameter[%zu]: '%s'", i, parameters[i].c_str());
    }
    
    int handlerCount = 0;
    int executedCount = 0;
    
    // Find all handlers for this event type
    for (const auto& handler : m_EventHandlers)
    {
        if (handler.eventType != eventType)
            continue;
            
        handlerCount++;
        
        MS_ANGEL_INFO("ASEngineEventManager: Executing handler #%d for %s", 
                      handlerCount, GetEventTypeName(eventType));
        MS_ANGEL_INFO("  Function: %s (Module: %s)", 
                      handler.pFunction->GetName(), handler.moduleName.c_str());
        
        // Acquire context from the manager's pool
        asIScriptContext* pContext = AcquireContext();
        if (!pContext)
        {
            MS_ANGEL_ERROR("ASEngineEventManager: Failed to acquire script context for %s", GetEventTypeName(eventType));
            continue;
        }
        
        // Prepare the function call
        int r = pContext->Prepare(handler.pFunction);
        if (r < 0)
        {
            const char* errorStr = "";
            switch(r)
            {
                case -1: errorStr = "asERROR"; break;
                case -2: errorStr = "asCONTEXT_ACTIVE"; break;
                case -3: errorStr = "asCONTEXT_NOT_FINISHED"; break;
                case -4: errorStr = "asCONTEXT_NOT_PREPARED"; break;
                case -5: errorStr = "asINVALID_ARG"; break;
                case -6: errorStr = "asNO_FUNCTION"; break;
                default: errorStr = "UNKNOWN"; break;
            }
            MS_ANGEL_ERROR("ASEngineEventManager: Failed to prepare function call for %s (error: %d - %s)", GetEventTypeName(eventType), r, errorStr);
            
            // Additional debugging info
            if (handler.pFunction)
            {
                MS_ANGEL_ERROR("Function details: Name='%s', ParamCount=%d, Module='%s'", 
                              handler.pFunction->GetName() ? handler.pFunction->GetName() : "NULL",
                              handler.pFunction->GetParamCount(),
                              handler.moduleName.c_str());
            }
            else
            {
                MS_ANGEL_ERROR("Function pointer is NULL!");
            }
            
            ReleaseContext(pContext);
            continue;
        }
        
        // Create persistent storage for Vector objects
        std::vector<Vector> vectorStorage;
        bool parameterError = false;
        
        // Set parameters based on event type
        switch (eventType)
        {
            case EngineEventType::PLAYER_CONNECT:
            case EngineEventType::PLAYER_DISCONNECT:
                if (parameters.size() >= 2)
                {
                    // Set string parameters based on their declaration type
                    if (IsParameterByReference(handler.pFunction, 0))
                        pContext->SetArgAddress(0, const_cast<std::string*>(&parameters[0]));
                    else
                        pContext->SetArgObject(0, const_cast<std::string*>(&parameters[0]));
                        
                    if (IsParameterByReference(handler.pFunction, 1))
                        pContext->SetArgAddress(1, const_cast<std::string*>(&parameters[1]));
                    else
                        pContext->SetArgObject(1, const_cast<std::string*>(&parameters[1]));
                }
                else
                {
                    MS_ANGEL_ERROR("ASEngineEventManager: Insufficient parameters for %s (need 2, got %zu)", 
                                  GetEventTypeName(eventType), parameters.size());
                    parameterError = true;
                }
                break;
                
            case EngineEventType::MONSTER_KILLED:
                if (parameters.size() >= 5)
                {
                    // Set string parameters
                    if (IsParameterByReference(handler.pFunction, 0))
                        pContext->SetArgAddress(0, const_cast<std::string*>(&parameters[0]));
                    else
                        pContext->SetArgObject(0, const_cast<std::string*>(&parameters[0]));
                        
                    if (IsParameterByReference(handler.pFunction, 1))
                        pContext->SetArgAddress(1, const_cast<std::string*>(&parameters[1]));
                    else
                        pContext->SetArgObject(1, const_cast<std::string*>(&parameters[1]));
                    
                    // Create Vector3 for the death position and store it persistently
                    try
                    {
                        vectorStorage.emplace_back(std::stof(parameters[2]), std::stof(parameters[3]), std::stof(parameters[4]));
                        
                        if (IsParameterByReference(handler.pFunction, 2))
                            pContext->SetArgAddress(2, &vectorStorage.back());
                        else
                            pContext->SetArgObject(2, &vectorStorage.back());
                    }
                    catch (const std::exception& e)
                    {
                        MS_ANGEL_ERROR("ASEngineEventManager: Failed to parse Vector3 coordinates for %s: %s", 
                                      GetEventTypeName(eventType), e.what());
                        parameterError = true;
                    }
                }
                else
                {
                    MS_ANGEL_ERROR("ASEngineEventManager: Insufficient parameters for %s (need 5, got %zu)", 
                                  GetEventTypeName(eventType), parameters.size());
                    parameterError = true;
                }
                break;
                
            case EngineEventType::TREASURE_SPAWNED:
                if (parameters.size() >= 4)
                {
                    // Set string parameter
                    if (IsParameterByReference(handler.pFunction, 0))
                        pContext->SetArgAddress(0, const_cast<std::string*>(&parameters[0]));
                    else
                        pContext->SetArgObject(0, const_cast<std::string*>(&parameters[0]));
                    
                    // Create Vector3 for the position and store it persistently
                    try
                    {
                        vectorStorage.emplace_back(std::stof(parameters[1]), std::stof(parameters[2]), std::stof(parameters[3]));
                        
                        if (IsParameterByReference(handler.pFunction, 1))
                            pContext->SetArgAddress(1, &vectorStorage.back());
                        else
                            pContext->SetArgObject(1, &vectorStorage.back());
                    }
                    catch (const std::exception& e)
                    {
                        MS_ANGEL_ERROR("ASEngineEventManager: Failed to parse Vector3 coordinates for %s: %s", 
                                      GetEventTypeName(eventType), e.what());
                        parameterError = true;
                    }
                }
                else
                {
                    MS_ANGEL_ERROR("ASEngineEventManager: Insufficient parameters for %s (need 4, got %zu)", 
                                  GetEventTypeName(eventType), parameters.size());
                    parameterError = true;
                }
                break;
                
            case EngineEventType::PLAYER_SPAWNED:
                if (parameters.size() >= 1)
                {
                    // Set string parameter
                    if (IsParameterByReference(handler.pFunction, 0))
                        pContext->SetArgAddress(0, const_cast<std::string*>(&parameters[0]));
                    else
                        pContext->SetArgObject(0, const_cast<std::string*>(&parameters[0]));
                }
                else
                {
                    MS_ANGEL_ERROR("ASEngineEventManager: Insufficient parameters for %s (need 1, got %zu)", 
                                  GetEventTypeName(eventType), parameters.size());
                    parameterError = true;
                }
                break;
        }
        
        // Only execute if no parameter errors occurred
        if (!parameterError)
        {
            // Execute the function
            r = pContext->Execute();
            if (r != asEXECUTION_FINISHED)
            {
                if (r == asEXECUTION_EXCEPTION)
                {
                    MS_ANGEL_ERROR("ASEngineEventManager: Exception in %s handler: %s", 
                                  GetEventTypeName(eventType), pContext->GetExceptionString());
                }
                else
                {
                    MS_ANGEL_ERROR("ASEngineEventManager: Failed to execute %s handler (result: %d)", 
                                  GetEventTypeName(eventType), r);
                }
            }
            else
            {
                executedCount++;
                MS_ANGEL_INFO("ASEngineEventManager: Successfully executed handler %s::%s", 
                             handler.moduleName.c_str(), handler.pFunction->GetName());
            }
        }
        
        // Release the context back to the pool
        ReleaseContext(pContext);
    }
    
    if (handlerCount > 0)
    {
        MS_ANGEL_INFO("ASEngineEventManager: Dispatched %s event to %d handlers (%d executed successfully)", 
                     GetEventTypeName(eventType), handlerCount, executedCount);
    }
    else
    {
        MS_ANGEL_INFO("ASEngineEventManager: No handlers registered for %s event", 
                     GetEventTypeName(eventType));
    }
}

//==========================================================================
// Acquire Context from Manager Pool
//==========================================================================
asIScriptContext* ASEngineEventManager::AcquireContext()
{
    if (!m_bInitialized)
        return nullptr;
        
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (!pManager)
        return nullptr;
        
    return pManager->AcquireContext();
}

//==========================================================================
// Release Context to Manager Pool
//==========================================================================
void ASEngineEventManager::ReleaseContext(asIScriptContext* pContext)
{
    if (!pContext)
        return;
        
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (pManager)
    {
        pManager->ReleaseContext(pContext);
    }
}

//==========================================================================
// Get Event Type Name
//==========================================================================
const char* ASEngineEventManager::GetEventTypeName(EngineEventType eventType) const
{
    switch (eventType)
    {
        case EngineEventType::PLAYER_CONNECT:    return "PlayerConnect";
        case EngineEventType::PLAYER_DISCONNECT: return "PlayerDisconnect";
        case EngineEventType::MONSTER_KILLED:    return "MonsterKilled";
        case EngineEventType::TREASURE_SPAWNED:  return "TreasureSpawned";
        case EngineEventType::PLAYER_SPAWNED:    return "PlayerSpawned";
        default: return "Unknown";
    }
}

//==========================================================================
// Log Event Handlers
//==========================================================================
void ASEngineEventManager::LogEventHandlers()
{
    if (!m_bInitialized)
        return;
        
    MS_ANGEL_INFO("ASEngineEventManager: Registered Event Handlers (%zu total):", m_EventHandlers.size());
    
    for (const auto& handler : m_EventHandlers)
    {
        MS_ANGEL_INFO("  - %s: %s (%s)", 
                     GetEventTypeName(handler.eventType),
                     handler.pFunction ? handler.pFunction->GetName() : "NULL",
                     handler.moduleName.c_str());
    }
}

//==========================================================================
// Get Handler Count for Event Type
//==========================================================================
int ASEngineEventManager::GetHandlerCount(EngineEventType eventType) const
{
    if (!m_bInitialized)
        return 0;
        
    int count = 0;
    for (const auto& handler : m_EventHandlers)
    {
        if (handler.eventType == eventType)
            count++;
    }
    return count;
}

//==========================================================================
// AngelScript Registration Functions
//==========================================================================
namespace ASEngineEvents
{
    // Helper to convert string to event type
    EngineEventType StringToEventType(const std::string& eventName)
    {
        if (eventName == "OnEnginePlayerConnect" || eventName == "PlayerConnect")
            return EngineEventType::PLAYER_CONNECT;
        else if (eventName == "OnEnginePlayerDisconnect" || eventName == "PlayerDisconnect")
            return EngineEventType::PLAYER_DISCONNECT;
        else if (eventName == "OnEngineMonsterKilled" || eventName == "MonsterKilled")
            return EngineEventType::MONSTER_KILLED;
        else if (eventName == "OnEngineTreasureSpawned" || eventName == "TreasureSpawned")
            return EngineEventType::TREASURE_SPAWNED;
        else if (eventName == "OnEnginePlayerSpawned" || eventName == "PlayerSpawned")
            return EngineEventType::PLAYER_SPAWNED;
        
        return EngineEventType::PLAYER_CONNECT; // Default fallback
    }

    // Register engine event (called from AngelScript)
    void RegisterEngineEvent_Generic(asIScriptGeneric* gen)
    {
        if (!gen)
            return;
            
        // Get the string parameter
        std::string* eventName = static_cast<std::string*>(gen->GetArgObject(0));
        if (!eventName)
            return;
            
        // Get the function reference parameter
        asIScriptFunction** pFuncPtr = static_cast<asIScriptFunction**>(gen->GetArgAddress(1));
        if (!pFuncPtr || !*pFuncPtr)
            return;
            
        asIScriptFunction* pFunction = *pFuncPtr;
        
        ASEngineEventManager* pManager = ASEngineEventManager::Instance();
        if (!pManager)
            return;
            
        EngineEventType eventType = StringToEventType(*eventName);
        
        // Get the module name from the function
        asIScriptModule* pModule = pFunction->GetModule();
        const char* szModuleName = pModule ? pModule->GetName() : "Unknown";
        
        pManager->RegisterEventHandler(eventType, pFunction, szModuleName);
    }

    // Unregister engine event (called from AngelScript)
    void UnregisterEngineEvent(const std::string& eventName)
    {
        ASEngineEventManager* pManager = ASEngineEventManager::Instance();
        if (!pManager)
            return;
            
        EngineEventType eventType = StringToEventType(eventName);
        
        // Note: This version unregisters from all modules - could be enhanced
        // to support per-module unregistration if needed
        MS_ANGEL_INFO("ASEngineEvents: Unregistering all handlers for %s", eventName.c_str());
    }

    // Log engine event handlers (called from AngelScript)
    void LogEngineEventHandlers()
    {
        ASEngineEventManager* pManager = ASEngineEventManager::Instance();
        if (pManager)
        {
            pManager->LogEventHandlers();
        }
    }

    // Register all engine event functions with AngelScript
    bool RegisterEngineEventFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine)
            return false;
            
        // Register utility functions that scripts can call
        int r = pEngine->RegisterGlobalFunction("void RegisterEngineEvent(const string &in, ?&in)", 
                                               asFUNCTION(RegisterEngineEvent_Generic), asCALL_GENERIC);
        if (r < 0)
        {
            MS_ANGEL_ERROR("ASEngineEvents: Failed to register RegisterEngineEvent function");
            return false;
        }
        
        r = pEngine->RegisterGlobalFunction("void UnregisterEngineEvent(const string &in)", 
                                           asFUNCTION(UnregisterEngineEvent), asCALL_CDECL);
        if (r < 0)
        {
            MS_ANGEL_ERROR("ASEngineEvents: Failed to register UnregisterEngineEvent function");
            return false;
        }
        
        r = pEngine->RegisterGlobalFunction("void LogEngineEventHandlers()", 
                                           asFUNCTION(LogEngineEventHandlers), asCALL_CDECL);
        if (r < 0)
        {
            MS_ANGEL_ERROR("ASEngineEvents: Failed to register LogEngineEventHandlers function");
            return false;
        }
        
        MS_ANGEL_INFO("ASEngineEvents: Successfully registered engine event functions");
        return true;
    }
}