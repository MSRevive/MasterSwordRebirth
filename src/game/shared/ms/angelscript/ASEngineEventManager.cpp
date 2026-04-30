//==========================================================================
// AngelScript Engine Event Manager Implementation
//==========================================================================

#include "ASEngineEventManager.h"
#include "CAngelScriptManager.h"
#include "../mslogger.h"
#include <asbind20/asbind.hpp>
#include <angelscript.h>
#include <algorithm>
#include <cmath> //required for vector.h

// Include vector.h directly to avoid mathlib.h macro conflicts
// We need the vec_t typedef but not the DotProduct macro
typedef float vec_t;
#include "hl/vector.h"

// Static instance
ASEngineEventManager* ASEngineEventManager::s_pInstance = nullptr;

// Static recursion depth for PLAYER_SAY_TEXT events
int ASEngineEventManager::s_nSayTextEventDepth = 0;

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
    // Allow reinitialization if the engine pointer has changed (level change scenario)
    if (m_bInitialized && m_pEngine == pEngine)
    {
        MS_ANGEL_INFO("ASEngineEventManager::Initialize: Already initialized with this engine (%p)", pEngine);
        return true;
    }
        
    if (!pEngine)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::Initialize: NULL engine pointer");
        return false;
    }
    
    // CRITICAL: If we're reinitializing with a new engine OR we have stale handlers, clear everything
    if (m_bInitialized && m_pEngine != pEngine)
    {
        MS_ANGEL_INFO("ASEngineEventManager::Initialize: ENGINE CHANGE DETECTED!");
        MS_ANGEL_INFO("  Old engine: %p, New engine: %p", m_pEngine, pEngine);
        MS_ANGEL_INFO("  Performing aggressive cleanup of old handlers...");
        Destroy();
    }
    else if (!m_bInitialized && !m_EventHandlers.empty())
    {
        // Safety check: If not initialized but handlers exist, something went wrong
        MS_ANGEL_ERROR("ASEngineEventManager::Initialize: INCONSISTENT STATE - handlers exist but not initialized!");
        MS_ANGEL_ERROR("  This indicates improper shutdown. Forcing cleanup...");
        Destroy();
    }
    
    m_pEngine = pEngine;
    m_bInitialized = true;
    
    MS_ANGEL_INFO("ASEngineEventManager initialized successfully with engine %p (handlers: %zu)", pEngine, m_EventHandlers.size());
    return true;
}

//==========================================================================
// Destroy
//==========================================================================
void ASEngineEventManager::Destroy()
{
    size_t handlerCount = m_EventHandlers.size();
    MS_ANGEL_INFO("ASEngineEventManager: Starting destruction process (initialized=%d, handlers=%zu)...", 
                  m_bInitialized, handlerCount);
        
    // CRITICAL: Store the old engine pointer before clearing
    asIScriptEngine* pOldEngine = m_pEngine;
    
    // Always clear handlers even if not marked as initialized
    // This handles cases where Destroy is called multiple times
    
    // Release all function references safely
    if (!m_EventHandlers.empty())
    {
        MS_ANGEL_INFO("ASEngineEventManager: Releasing %zu event handlers", handlerCount);
        for (auto& handler : m_EventHandlers)
        {
            if (handler.pFunction)
            {
                MS_ANGEL_INFO("ASEngineEventManager: Releasing function %s from module %s (engine: %p)", 
                             handler.pFunction->GetName(), handler.moduleName.c_str(), pOldEngine);
                try
                {
                    // Verify the function's engine matches before releasing
                    asIScriptModule* pFuncModule = handler.pFunction->GetModule();
                    asIScriptEngine* pFuncEngine = pFuncModule ? pFuncModule->GetEngine() : nullptr;
                    
                    if (pFuncEngine != pOldEngine && pOldEngine != nullptr)
                    {
                        MS_ANGEL_WARN("ASEngineEventManager: Handler function engine mismatch during cleanup! (handler: %p, expected: %p)", 
                                     pFuncEngine, pOldEngine);
                    }
                    
                    handler.pFunction->Release();
                }
                catch (...)
                {
                    MS_ANGEL_ERROR("ASEngineEventManager: Exception while releasing function %s", 
                                  handler.pFunction->GetName());
                }
                handler.pFunction = nullptr; // Ensure pointer is nulled
            }
        }
    }
    
    // Clear the handlers list completely
    m_EventHandlers.clear();
    
    // Mark as not initialized
    m_bInitialized = false;
    
    // Null the engine pointer to prevent further access
    m_pEngine = nullptr;
    
    MS_ANGEL_INFO("ASEngineEventManager destroyed (%zu handlers cleared, engine %p unbound)", handlerCount, pOldEngine);
}

//==========================================================================
// Register Event Handler
//==========================================================================
bool ASEngineEventManager::RegisterEventHandler(EngineEventType eventType, asIScriptFunction* pFunction, const char* szModuleName)
{
    if (!m_bInitialized || !pFunction || !szModuleName)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::RegisterEventHandler: Invalid parameters (initialized=%d, function=%p, module=%s)", 
                       m_bInitialized, pFunction, szModuleName ? szModuleName : "NULL");
        return false;
    }
    
    if (!m_pEngine)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::RegisterEventHandler: Event manager has no engine pointer!");
        return false;
    }
    
    // CRITICAL: Validate that the function belongs to the current engine
    asIScriptModule* pFuncModule = pFunction->GetModule();
    if (!pFuncModule)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::RegisterEventHandler: Function '%s' has no module - cannot register", 
                      pFunction->GetName());
        return false;
    }
    
    asIScriptEngine* pFuncEngine = pFuncModule->GetEngine();
    if (!pFuncEngine)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::RegisterEventHandler: Function module has no engine - cannot register");
        return false;
    }
    
    if (pFuncEngine != m_pEngine)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::RegisterEventHandler: CRITICAL ENGINE MISMATCH!");
        MS_ANGEL_ERROR("  Function '%s' from module '%s' belongs to engine %p", pFunction->GetName(), szModuleName, pFuncEngine);
        MS_ANGEL_ERROR("  But EventManager is using engine %p", m_pEngine);
        MS_ANGEL_ERROR("  This indicates a stale function pointer from an old engine!");
        MS_ANGEL_ERROR("  REGISTRATION REJECTED - Handler will NOT be registered");
        return false;
    }
    
    MS_ANGEL_INFO("ASEngineEventManager::RegisterEventHandler: Validated engine match for '%s' (engine: %p)", 
                 pFunction->GetName(), pFuncEngine);
    
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
// Fire Player Say Text Event
//==========================================================================
void ASEngineEventManager::FirePlayerSayTextEvent(const char* szPlayerName, const char* szSteamID, const char* szText)
{
    // Create RAII depth guard for recursion protection
    SayTextEventDepthGuard depthGuard;
    
    // Check for recursion overflow before any other processing
    if (depthGuard.IsMaxDepthExceeded())
    {
        MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: Recursion depth exceeded (%d > %d) - blocking event to prevent infinite loop", 
                       depthGuard.GetCurrentDepth(), MAX_SAY_TEXT_EVENT_DEPTH);
        MS_ANGEL_ERROR("  Player: %s, Text: '%s'", 
                       szPlayerName ? szPlayerName : "NULL", 
                       szText ? szText : "NULL");
        return;
    }
    
    // Log recursion depth for debugging (only at depth > 1 to avoid spam)
    if (depthGuard.GetCurrentDepth() > 1)
    {
        MS_ANGEL_INFO("ASEngineEventManager::FirePlayerSayTextEvent: Nested call detected (depth=%d) for player %s", 
                      depthGuard.GetCurrentDepth(), szPlayerName ? szPlayerName : "NULL");
    }
    
    if (!m_bInitialized)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: Manager not initialized");
        return;
    }
    
    // Enhanced null pointer validation with detailed logging
    if (!szPlayerName)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: szPlayerName is NULL");
        szPlayerName = "Unknown Player";
    }
    
    if (!szSteamID)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: szSteamID is NULL");
        szSteamID = "Unknown SteamID";
    }
    
    if (!szText)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: szText is NULL - blocking event");
        return; // Don't fire event with null text as it will crash
    }
    
    // Additional safety checks for valid string pointers
    std::string safePlayerName;
    std::string safeSteamID;
    std::string safeText;
    
    // Safe string construction with exception handling
    try
    {
        // Validate and safely copy player name
        if (szPlayerName && strlen(szPlayerName) > 0 && strlen(szPlayerName) < 256)
        {
            safePlayerName = std::string(szPlayerName);
        }
        else
        {
            safePlayerName = "Unknown Player";
            if (szPlayerName)
            {
                MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: Invalid player name length (%zu)", 
                              szPlayerName ? strlen(szPlayerName) : 0);
            }
        }
        
        // Validate and safely copy Steam ID
        if (szSteamID && strlen(szSteamID) > 0 && strlen(szSteamID) < 128)
        {
            safeSteamID = std::string(szSteamID);
        }
        else
        {
            safeSteamID = "Unknown SteamID";
            if (szSteamID)
            {
                MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: Invalid Steam ID length (%zu)", 
                              szSteamID ? strlen(szSteamID) : 0);
            }
        }
        
        // Validate and safely copy text (most critical)
        if (szText && strlen(szText) > 0 && strlen(szText) < 512)
        {
            safeText = std::string(szText);
        }
        else
        {
            MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: Invalid text length (%zu) - blocking event", 
                          szText ? strlen(szText) : 0);
            return; // Don't fire event with invalid text
        }
    }
    catch (const std::exception& e)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: Exception during string construction: %s", e.what());
        return; // Don't fire event if string construction fails
    }
    catch (...)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: Unknown exception during string construction");
        return; // Don't fire event if string construction fails
    }
    
    // Create parameter vector with safe strings
    std::vector<std::string> params;
    try
    {
        params.reserve(3); // Pre-allocate to avoid reallocation
        params.push_back(safePlayerName);
        params.push_back(safeSteamID);
        params.push_back(safeText);
    }
    catch (const std::exception& e)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: Exception during parameter creation: %s", e.what());
        return;
    }
    catch (...)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: Unknown exception during parameter creation");
        return;
    }
    
    MS_ANGEL_INFO("ASEngineEventManager: Firing PlayerSayText event for %s: '%s'", 
                 safePlayerName.c_str(), safeText.c_str());
    
    // Dispatch with additional exception handling
    try
    {
        DispatchEvent(EngineEventType::PLAYER_SAY_TEXT, params);
    }
    catch (const std::exception& e)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: Exception during event dispatch: %s", e.what());
    }
    catch (...)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::FirePlayerSayTextEvent: Unknown exception during event dispatch");
    }
}

//==========================================================================
// Helper function to determine if parameter should be passed by reference
//==========================================================================
bool ASEngineEventManager::IsParameterByReference(asIScriptFunction* pFunction, int paramIndex)
{
    if (!pFunction || (unsigned int)paramIndex >= (unsigned int)pFunction->GetParamCount())
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
    // Enhanced safety checks to prevent memory corruption
    if (!m_bInitialized || !m_pEngine)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::DispatchEvent: Event manager not properly initialized (initialized=%d, engine=%p)", 
                       m_bInitialized, m_pEngine);
        return;
    }
    
    // Verify the AngelScript manager is still valid
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (!pManager || !pManager->IsInitialized())
    {
        MS_ANGEL_ERROR("ASEngineEventManager::DispatchEvent: AngelScript manager is invalid or not initialized");
        return;
    }
        
    MS_ANGEL_INFO("ASEngineEventManager::DispatchEvent - Event: %s (type=%d), Parameters: %zu", 
                  GetEventTypeName(eventType), static_cast<int>(eventType), parameters.size());
    
    // Log parameters
    for (size_t i = 0; i < parameters.size(); i++)
    {
        MS_ANGEL_INFO("  Parameter[%zu]: '%s'", i, parameters[i].c_str());
    }
    
    // Debug: Log all registered handlers first
    MS_ANGEL_INFO("ASEngineEventManager: Total registered handlers: %zu", m_EventHandlers.size());
    for (size_t i = 0; i < m_EventHandlers.size(); i++)
    {
        const auto& h = m_EventHandlers[i];
        MS_ANGEL_INFO("  Handler[%zu]: %s -> %s (type=%d)", i, 
                     h.pFunction ? h.pFunction->GetName() : "NULL",
                     GetEventTypeName(h.eventType), static_cast<int>(h.eventType));
    }
    
    int handlerCount = 0;
    int executedCount = 0;
    std::vector<size_t> staleHandlerIndices;  // Track indices of stale handlers to remove
    
    // Find all handlers for this event type
    for (size_t handlerIdx = 0; handlerIdx < m_EventHandlers.size(); handlerIdx++)
    {
        const auto& handler = m_EventHandlers[handlerIdx];
        MS_ANGEL_INFO("ASEngineEventManager: Checking handler %s (type=%d) against event %s (type=%d)",
                     handler.pFunction ? handler.pFunction->GetName() : "NULL",
                     static_cast<int>(handler.eventType),
                     GetEventTypeName(eventType),
                     static_cast<int>(eventType));
        
        if (handler.eventType != eventType)
        {
            MS_ANGEL_INFO("  -> SKIPPED: Event type mismatch");
            continue;
        }
        
        MS_ANGEL_INFO("  -> SELECTED: Event type matches");
            
        handlerCount++;
        
        MS_ANGEL_INFO("ASEngineEventManager: Executing handler #%d for %s", 
                      handlerCount, GetEventTypeName(eventType));
                      
        // Validate function pointer before use to prevent memory corruption
        if (!handler.pFunction)
        {
            MS_ANGEL_ERROR("ASEngineEventManager: Function pointer is NULL for %s", GetEventTypeName(eventType));
            staleHandlerIndices.push_back(handlerIdx);
            continue;
        }
        
        // Additional validation: check if function is still part of a valid module
        asIScriptModule* pModule = handler.pFunction->GetModule();
        if (!pModule)
        {
            MS_ANGEL_ERROR("ASEngineEventManager: Function module is NULL for %s::%s - STALE FUNCTION POINTER DETECTED", 
                          handler.moduleName.c_str(), handler.pFunction->GetName());
            MS_ANGEL_ERROR("  This indicates the module was unloaded but event handlers weren't unregistered");
            staleHandlerIndices.push_back(handlerIdx);
            continue;
        }
        
        // Verify the module is still registered with the engine
        asIScriptModule* pEngineModule = m_pEngine->GetModule(handler.moduleName.c_str());
        if (!pEngineModule || pEngineModule != pModule)
        {
            MS_ANGEL_ERROR("ASEngineEventManager: Module '%s' is not registered with engine - STALE FUNCTION POINTER", 
                          handler.moduleName.c_str());
            MS_ANGEL_ERROR("  Expected module: %p, Found module: %p", pModule, pEngineModule);
            staleHandlerIndices.push_back(handlerIdx);
            continue;
        }
        
        // Verify the module name matches to detect stale references
        const char* actualModuleName = pModule->GetName();
        if (!actualModuleName || handler.moduleName != actualModuleName)
        {
            MS_ANGEL_ERROR("ASEngineEventManager: Module name mismatch for %s (expected: %s, actual: %s) - STALE REFERENCE", 
                          handler.pFunction->GetName(), handler.moduleName.c_str(), 
                          actualModuleName ? actualModuleName : "NULL");
            staleHandlerIndices.push_back(handlerIdx);
            continue;
        }
        
        // Verify the function is still valid within its module
        asIScriptFunction* pVerifyFunc = pModule->GetFunctionByName(handler.pFunction->GetName());
        if (!pVerifyFunc || pVerifyFunc != handler.pFunction)
        {
            MS_ANGEL_ERROR("ASEngineEventManager: Function '%s' not found in module '%s' - STALE FUNCTION POINTER", 
                          handler.pFunction->GetName(), handler.moduleName.c_str());
            MS_ANGEL_ERROR("  Expected function: %p, Found function: %p", handler.pFunction, pVerifyFunc);
            staleHandlerIndices.push_back(handlerIdx);
            continue;
        }
        
        MS_ANGEL_INFO("  Function: %s (Module: %s)", 
                      handler.pFunction->GetName(), handler.moduleName.c_str());
        
        // Acquire context from the manager's pool
        asIScriptContext* pContext = AcquireContext();
        if (!pContext)
        {
            MS_ANGEL_ERROR("ASEngineEventManager: Failed to acquire script context for %s", GetEventTypeName(eventType));
            continue;
        }
        
        // FINAL VALIDATION: Verify the function belongs to the same engine as the context
        asIScriptEngine* pContextEngine = pContext->GetEngine();
        asIScriptEngine* pFunctionEngine = handler.pFunction->GetModule() ? handler.pFunction->GetModule()->GetEngine() : nullptr;
        
        if (!pFunctionEngine || pContextEngine != pFunctionEngine)
        {
            MS_ANGEL_ERROR("ASEngineEventManager: CRITICAL - Function/Context engine mismatch!");
            MS_ANGEL_ERROR("  Context engine: %p", pContextEngine);
            MS_ANGEL_ERROR("  Function engine: %p", pFunctionEngine);
            MS_ANGEL_ERROR("  Function: %s from module %s", handler.pFunction->GetName(), handler.moduleName.c_str());
            MS_ANGEL_ERROR("  This handler must be removed - it's from an old engine!");
            ReleaseContext(pContext);
            staleHandlerIndices.push_back(handlerIdx);
            continue;
        }
        
        // Prepare the function call with additional error handling
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
                
            case EngineEventType::PLAYER_SAY_TEXT:
                if (parameters.size() >= 3)
                {
                    // Set string parameters for player name, steam ID, and text
                    if (IsParameterByReference(handler.pFunction, 0))
                        pContext->SetArgAddress(0, const_cast<std::string*>(&parameters[0]));
                    else
                        pContext->SetArgObject(0, const_cast<std::string*>(&parameters[0]));
                        
                    if (IsParameterByReference(handler.pFunction, 1))
                        pContext->SetArgAddress(1, const_cast<std::string*>(&parameters[1]));
                    else
                        pContext->SetArgObject(1, const_cast<std::string*>(&parameters[1]));
                        
                    if (IsParameterByReference(handler.pFunction, 2))
                        pContext->SetArgAddress(2, const_cast<std::string*>(&parameters[2]));
                    else
                        pContext->SetArgObject(2, const_cast<std::string*>(&parameters[2]));
                }
                else
                {
                    MS_ANGEL_ERROR("ASEngineEventManager: Insufficient parameters for %s (need 3, got %zu)", 
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
    
    // Clean up any stale handlers detected during execution
    if (!staleHandlerIndices.empty())
    {
        MS_ANGEL_WARN("ASEngineEventManager: Removing %zu stale handlers detected during event dispatch", staleHandlerIndices.size());
        
        // Sort indices in descending order to remove from back to front (avoids index shifting issues)
        std::sort(staleHandlerIndices.begin(), staleHandlerIndices.end(), std::greater<size_t>());
        
        for (size_t idx : staleHandlerIndices)
        {
            if (idx < m_EventHandlers.size())
            {
                auto& handler = m_EventHandlers[idx];
                MS_ANGEL_INFO("  Removing stale handler: %s from module %s", 
                             handler.pFunction ? handler.pFunction->GetName() : "NULL",
                             handler.moduleName.c_str());
                
                // Release the function pointer if it exists
                if (handler.pFunction)
                {
                    handler.pFunction->Release();
                    handler.pFunction = nullptr;
                }
                
                // Remove from vector
                m_EventHandlers.erase(m_EventHandlers.begin() + idx);
            }
        }
        
        MS_ANGEL_INFO("ASEngineEventManager: Stale handlers removed. Remaining handlers: %zu", m_EventHandlers.size());
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
    // Enhanced safety checks to prevent memory corruption
    if (!m_bInitialized || !m_pEngine)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::AcquireContext: Event manager not properly initialized (initialized=%d, engine=%p)", 
                       m_bInitialized, m_pEngine);
        return nullptr;
    }
        
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (!pManager)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::AcquireContext: AngelScript manager instance is null");
        return nullptr;
    }
    
    // Verify the manager is still initialized
    if (!pManager->IsInitialized())
    {
        MS_ANGEL_ERROR("ASEngineEventManager::AcquireContext: AngelScript manager is not initialized");
        return nullptr;
    }
    
    // Verify the engine is still valid
    asIScriptEngine* pCurrentEngine = pManager->GetEngine();
    if (!pCurrentEngine || pCurrentEngine != m_pEngine)
    {
        MS_ANGEL_ERROR("ASEngineEventManager::AcquireContext: Engine mismatch or invalid (current=%p, expected=%p)", 
                       pCurrentEngine, m_pEngine);
        return nullptr;
    }
        
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
        case EngineEventType::PLAYER_SAY_TEXT:   return "PlayerSayText";
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
        else if (eventName == "OnEnginePlayerSayText" || eventName == "PlayerSayText")
            return EngineEventType::PLAYER_SAY_TEXT;
        
        // Log unknown event name and return an invalid value
        MS_ANGEL_ERROR("StringToEventType: Unknown event name '%s' - registration will fail", eventName.c_str());
        return static_cast<EngineEventType>(-1); // Return invalid event type instead of defaulting
    }

    // Register engine event (called from AngelScript)
    void RegisterEngineEvent_Generic(asIScriptGeneric* gen)
    {
        if (!gen)
        {
            MS_ANGEL_ERROR("RegisterEngineEvent_Generic: NULL generic interface!");
            return;
        }
            
        // Get the string parameter
        std::string* eventName = static_cast<std::string*>(gen->GetArgObject(0));
        if (!eventName)
        {
            MS_ANGEL_ERROR("RegisterEngineEvent_Generic: NULL event name!");
            return;
        }
            
        // Get the function reference parameter
        asIScriptFunction** pFuncPtr = static_cast<asIScriptFunction**>(gen->GetArgAddress(1));
        if (!pFuncPtr || !*pFuncPtr)
        {
            MS_ANGEL_ERROR("RegisterEngineEvent_Generic: NULL function pointer for event '%s'!", eventName->c_str());
            return;
        }
            
        asIScriptFunction* pFunction = *pFuncPtr;
        
        ASEngineEventManager* pManager = ASEngineEventManager::Instance();
        if (!pManager)
        {
            MS_ANGEL_ERROR("RegisterEngineEvent_Generic: Event manager instance not available!");
            return;
        }
            
        EngineEventType eventType = StringToEventType(*eventName);
        
        // Validate that we got a valid event type
        if (static_cast<int>(eventType) == -1)
        {
            MS_ANGEL_ERROR("RegisterEngineEvent_Generic: Invalid event name '%s' - registration failed", eventName->c_str());
            return;
        }
        
        // Get the module name from the function
        asIScriptModule* pModule = pFunction->GetModule();
        const char* szModuleName = pModule ? pModule->GetName() : "Unknown";
        
        MS_ANGEL_INFO("RegisterEngineEvent_Generic: Attempting to register '%s' -> %s (module: %s)", 
                     eventName->c_str(), pManager->GetEventTypeName(eventType), szModuleName);
        
        bool result = pManager->RegisterEventHandler(eventType, pFunction, szModuleName);
        
        if (result)
        {
            MS_ANGEL_INFO("RegisterEngineEvent_Generic: SUCCESS - Handler '%s' registered for event '%s'", 
                         pFunction->GetName(), eventName->c_str());
        }
        else
        {
            MS_ANGEL_ERROR("RegisterEngineEvent_Generic: FAILED - Handler '%s' could not be registered for event '%s'!", 
                          pFunction->GetName(), eventName->c_str());
            MS_ANGEL_ERROR("  Module: %s, Event Type: %s", szModuleName, pManager->GetEventTypeName(eventType));
        }
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