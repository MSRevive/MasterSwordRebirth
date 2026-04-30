#include "CAngelScriptManager.h"
#include <cstdio>  // for printf
#include <cstring> // for memset
#include <cstdlib> // for malloc, free
#include <ctime>   // for clock, CLOCKS_PER_SEC
#include <new>     // for placement new
#include <vector>  // for std::vector
#include "../mslogger.h"  // Include MSLogger for unified logging
#include "cvardef.h"        // For cvar_t definition
#include "ASEngineEventManager.h"  // Include for event manager cleanup

// Only include AngelScript if we're compiling with it enabled
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244) // conversion warnings from AngelScript
#pragma warning(disable: 4996) // deprecated function warnings
#endif

#include "angelscript.h"
#include "ASBindings.h"
#include "ASCoroutines.h"
#include "ASObjectPool.h"
#include "ASDebugger.h"
#include "ASEngineInterface.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Static instance
CAngelScriptManager* CAngelScriptManager::s_pInstance = nullptr;

// Helper function to check if debug mode is enabled
static bool IsDebugModeEnabled()
{
        return ASEngineProvider::GetCvarString("as_debug_mode") == "1";
}

//==========================================================================
// Constructor
//==========================================================================
CAngelScriptManager::CAngelScriptManager()
    : m_pEngine(nullptr)
    , m_bInitialized(false)
    , m_nMemoryUsed(0)
    , m_nMemoryLimit(268435456) // 256MB default limit
    , m_pDebugger(nullptr)
{
}

//==========================================================================
// Destructor
//==========================================================================
CAngelScriptManager::~CAngelScriptManager()
{
    Destroy();
}

//==========================================================================
// Singleton access
//==========================================================================
CAngelScriptManager* CAngelScriptManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new CAngelScriptManager();
    }
    return s_pInstance;
}

//==========================================================================
// Shutdown singleton
//==========================================================================
void CAngelScriptManager::Shutdown()
{
    if (s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

//==========================================================================
// AngelScript message callback
//==========================================================================
void ASMessageCallback(const asSMessageInfo* msg, void* param)
{
    // Use MSLogger for proper categorized logging
    if (msg->type == asMSGTYPE_ERROR)
    {
        // Log to both ANGELSCRIPT and GENERAL logs for visibility
        MS_ANGEL_ERROR("%s (%d, %d): %s",
                       msg->section ? msg->section : "Unknown",
                       msg->row, msg->col, msg->message);
        MS_ERROR("[COMPILATION ERROR] %s (%d, %d): %s",
                 msg->section ? msg->section : "Unknown",
                 msg->row, msg->col, msg->message);
    }
    else if (msg->type == asMSGTYPE_WARNING)
    {
        MS_ANGEL_INFO("WARNING: %s (%d, %d): %s",
                      msg->section ? msg->section : "Unknown",
                      msg->row, msg->col, msg->message);
        MS_INFO("[COMPILATION WARNING] %s (%d, %d): %s",
                msg->section ? msg->section : "Unknown",
                msg->row, msg->col, msg->message);
    }
    else
    {
        MS_ANGEL_INFO("%s (%d, %d): %s",
                      msg->section ? msg->section : "Unknown",
                      msg->row, msg->col, msg->message);
        MS_INFO("[COMPILATION INFO] %s (%d, %d): %s",
                msg->section ? msg->section : "Unknown",
                msg->row, msg->col, msg->message);
    }
}

//==========================================================================
// Initialize AngelScript engine
//==========================================================================
bool CAngelScriptManager::Initialize()
{
    if (m_bInitialized)
        return true;
        
    // Create the AngelScript engine
    m_pEngine = asCreateScriptEngine();
    if (!m_pEngine)
    {
        LogMessage("Failed to create AngelScript engine", 1);
        return false;
    }
    
    
    // Set the message callback
    int r = m_pEngine->SetMessageCallback(asFUNCTION(ASMessageCallback), 0, asCALL_CDECL);
    if (r < 0)
    {
        LogMessage("Failed to set AngelScript message callback", 1);
        m_pEngine->ShutDownAndRelease();
        m_pEngine = nullptr;
        return false;
    }
    
    // Configure engine properties for 32-bit constraints and class support
    r = m_pEngine->SetEngineProperty(asEP_MAX_STACK_SIZE, 1024*1024); // 1MB stack
    if (r < 0)
    {
        LogMessage("Failed to set AngelScript stack size", 1);
    }
    
    // Enable class support and modern syntax features
    r = m_pEngine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);
    if (r < 0)
    {
        LogMessage("Failed to enable unsafe references", 1);
    }
    
    r = m_pEngine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, true);
    if (r < 0)
    {
        LogMessage("Failed to enable bytecode optimization", 1);
    }
    
    r = m_pEngine->SetEngineProperty(asEP_COPY_SCRIPT_SECTIONS, true);
    if (r < 0)
    {
        LogMessage("Failed to enable script section copying", 1);
    }
    
    // Set up memory allocation hooks
    r = asSetGlobalMemoryFunctions(ASMalloc, ASFree);
    if (r < 0)
    {
        LogMessage("Failed to set AngelScript memory functions", 1);
    }
    
    // Initialize debugger if debug mode is enabled
    if (IsDebugModeEnabled())
    {
        LogMessage("Initializing AngelScript Debugger...");
        m_pDebugger = new ASDebugger();
        if (m_pDebugger->Initialize(m_pEngine))
        {
            LogMessage("AngelScript Debugger initialized successfully");
            
            // Enable stepping for debugging
            m_pDebugger->EnableStepping(true);
            
            // Add a breakpoint in GameMaster constructor for debugging
            m_pDebugger->AddBreakpoint("GameMaster", 156); // Constructor line
            m_pDebugger->AddBreakpoint("GameMaster", 496); // OnEnginePlayerConnect line
            
            LogMessage("Debugger breakpoints set for GameMaster");
        }
        else
        {
            LogMessage("Failed to initialize AngelScript Debugger", 1);
            delete m_pDebugger;
            m_pDebugger = nullptr;
        }
    }
    
    // Register all bindings through integration layer
    if (!ASBindings::RegisterAll(m_pEngine))
    {
        LogMessage("Failed to register AngelScript bindings", 1);
        m_pEngine->ShutDownAndRelease();
        m_pEngine = nullptr;
        return false;
    }
    
    // Initialize the event manager with the new engine
    ASEngineEventManager* pEventManager = ASEngineEventManager::Instance();
    if (pEventManager)
    {
        if (pEventManager->Initialize(m_pEngine))
        {
            LogMessage("AngelScript Event Manager initialized successfully");
        }
        else
        {
            LogMessage("Failed to initialize AngelScript Event Manager", 1);
            // Don't fail the entire initialization for event manager issues
        }
    }
    
    m_bInitialized = true;
    LogMessage("AngelScript Manager initialized successfully");
    return true;
}

//==========================================================================
// Destroy AngelScript engine
//==========================================================================
void CAngelScriptManager::Destroy()
{
    if (!m_bInitialized)
        return;
    
    MS_ANGEL_INFO("=== CAngelScriptManager::Destroy - Starting cleanup ===");
        
    // Destroy the event manager first to clear all event handlers
    // This prevents dangling function references when the engine is destroyed
    ASEngineEventManager* pEventManager = ASEngineEventManager::Instance();
    if (pEventManager)
    {
        MS_ANGEL_INFO("Destroying AngelScript Event Manager...");
        pEventManager->Destroy();
        MS_ANGEL_INFO("AngelScript Event Manager destroyed");
    }
    
    // CRITICAL: Force garbage collection before releasing contexts and engine
    // This helps clean up circular references and global objects
    if (m_pEngine)
    {
        MS_ANGEL_INFO("Running full garbage collection cycle...");
        
        // Get GC statistics before cleanup
        asUINT currentSize, totalDestroyed, totalDetected;
        m_pEngine->GetGCStatistics(&currentSize, &totalDestroyed, &totalDetected);
        MS_ANGEL_INFO("GC Statistics before cleanup: CurrentSize=%u, TotalDestroyed=%u, TotalDetected=%u", 
                      currentSize, totalDestroyed, totalDetected);
        
        // Run multiple GC cycles to handle circular references
        // Each cycle may detect new objects that can be destroyed
        const int maxGCCycles = 5;
        for (int cycle = 0; cycle < maxGCCycles; cycle++)
        {
            // Start incremental GC
            int r = m_pEngine->GarbageCollect(asGC_FULL_CYCLE | asGC_DESTROY_GARBAGE);
            
            // Get statistics after this cycle
            m_pEngine->GetGCStatistics(&currentSize, &totalDestroyed, &totalDetected);
            MS_ANGEL_INFO("GC Cycle %d/%d: CurrentSize=%u, TotalDestroyed=%u, TotalDetected=%u", 
                          cycle + 1, maxGCCycles, currentSize, totalDestroyed, totalDetected);
            
            // If no objects remain, we're done
            if (currentSize == 0)
            {
                MS_ANGEL_INFO("All GC objects cleaned up after %d cycles", cycle + 1);
                break;
            }
            
            // If this is the last cycle and objects still remain, log a warning
            if (cycle == maxGCCycles - 1 && currentSize > 0)
            {
                MS_ANGEL_ERROR("WARNING: %u objects still in GC after %d cycles", currentSize, maxGCCycles);
                MS_ANGEL_ERROR("This may indicate circular references or leaked objects");
            }
        }
    }
    
    // Release all contexts in the pool AFTER garbage collection
    MS_ANGEL_INFO("Releasing %zu context(s) from pool...", m_ContextPool.size());
    for (size_t i = 0; i < m_ContextPool.size(); i++)
    {
        if (m_ContextPool[i])
        {
            m_ContextPool[i]->Release();
        }
    }
    m_ContextPool.clear();
    MS_ANGEL_INFO("Context pool cleared");
    
    // Shutdown debugger
    if (m_pDebugger)
    {
        MS_ANGEL_INFO("Shutting down debugger...");
        m_pDebugger->Shutdown();
        delete m_pDebugger;
        m_pDebugger = nullptr;
        MS_ANGEL_INFO("Debugger shut down");
    }
    
    // Shutdown optimization systems
    MS_ANGEL_INFO("Shutting down optimization systems...");
    ShutdownOptimizationSystems();
    
    // Shutdown coroutine manager
    MS_ANGEL_INFO("Shutting down coroutine manager...");
    ASCoroutineManager::Shutdown();
    
    // Clean up the engine - this will trigger final GC
    if (m_pEngine)
    {
        MS_ANGEL_INFO("Shutting down and releasing AngelScript engine...");
        m_pEngine->ShutDownAndRelease();
        m_pEngine = nullptr;
        MS_ANGEL_INFO("AngelScript engine released");
    }
    
    m_bInitialized = false;
    
    MS_ANGEL_INFO("=== AngelScript Manager destroyed successfully ===");
}

//==========================================================================
// Get script module
//==========================================================================
asIScriptModule* CAngelScriptManager::GetModule(const char* szModuleName)
{
    if (!m_bInitialized || !m_pEngine)
        return nullptr;
        
    return m_pEngine->GetModule(szModuleName);
}

//==========================================================================
// Compile script module
//==========================================================================
bool CAngelScriptManager::CompileModule(const char* szModuleName, const char* szScript)
{
    if (!m_bInitialized || !m_pEngine || !szModuleName || !szScript)
        return false;
        
    // Create a module
    asIScriptModule* pModule = m_pEngine->GetModule(szModuleName, asGM_ALWAYS_CREATE);
    if (!pModule)
    {
        LogMessage("Failed to create AngelScript module", 1);
        return false;
    }
    
    // Add the script section
    int r = pModule->AddScriptSection("script", szScript);
    if (r < 0)
    {
        LogMessage("Failed to add script section to module", 1);
        return false;
    }
    
    // Compile the module
    r = pModule->Build();
    if (r < 0)
    {
        LogMessage("Failed to compile AngelScript module", 1);
        return false;
    }
    
    return true;
}

//==========================================================================
// Remove script module
//==========================================================================
void CAngelScriptManager::RemoveModule(const char* szModuleName)
{
    if (!m_bInitialized || !m_pEngine || !szModuleName)
        return;
        
    m_pEngine->DiscardModule(szModuleName);
}

//==========================================================================
// Acquire context from pool
//==========================================================================
asIScriptContext* CAngelScriptManager::AcquireContext()
{
    if (!m_bInitialized || !m_pEngine)
        return nullptr;
        
    asIScriptContext* pContext = nullptr;
    
    // Check if we have a context available in the pool
    if (!m_ContextPool.empty())
    {
        pContext = m_ContextPool.back();
        m_ContextPool.pop_back();
        
        // CRITICAL: Validate that the pooled context belongs to the current engine
        // If engines were switched (e.g. during level change), old contexts must be discarded
        if (pContext && pContext->GetEngine() != m_pEngine)
        {
            MS_ANGEL_WARN("AcquireContext: Pooled context belongs to wrong engine! (context: %p, expected: %p)", 
                         pContext->GetEngine(), m_pEngine);
            MS_ANGEL_WARN("  Discarding stale context and creating new one");
            pContext->Release();
            pContext = nullptr;
        }
        
        if (pContext)
        {
            // Make sure the context is in a clean state
            pContext->Unprepare();
        }
    }
    
    // Create a new context if pool is empty or stale context was discarded
    if (!pContext)
    {
        pContext = m_pEngine->CreateContext();
        if (!pContext)
        {
            LogMessage("Failed to create script context", 1);
        }
        else
        {
            MS_ANGEL_INFO("AcquireContext: Created new context from engine %p", m_pEngine);
        }
    }
    
    // Set line callback for debugging if debugger is enabled
    // TEMPORARILY DISABLED: These callbacks can cause crashes when the context is in invalid states
    // TODO: Re-enable when debugger is fully implemented and tested
    /*
    if (pContext && m_pDebugger && IsDebugModeEnabled())
    {
        pContext->SetLineCallback(asFUNCTION(ASDebugger::LineCallback), m_pDebugger, asCALL_CDECL_OBJLAST);
        pContext->SetExceptionCallback(asFUNCTION(ASDebugger::ExceptionCallback), m_pDebugger, asCALL_CDECL_OBJLAST);
    }
    */
    
    return pContext;
}

//==========================================================================
// Clear context pool
//==========================================================================
void CAngelScriptManager::ClearContextPool()
{
    MS_ANGEL_INFO("ClearContextPool: Clearing %zu pooled contexts...", m_ContextPool.size());
    
    // Release all contexts in the pool
    for (size_t i = 0; i < m_ContextPool.size(); i++)
    {
        if (m_ContextPool[i])
        {
            // Unprepare to clear any stale function references
            m_ContextPool[i]->Unprepare();
            m_ContextPool[i]->Release();
        }
    }
    m_ContextPool.clear();
    
    MS_ANGEL_INFO("ClearContextPool: Context pool cleared");
}

//==========================================================================
// Clear all script modules
//==========================================================================
void CAngelScriptManager::ClearAllModules()
{
    if (!m_pEngine)
        return;
        
    asUINT moduleCount = m_pEngine->GetModuleCount();
    MS_ANGEL_INFO("ClearAllModules: Discarding %u script modules...", moduleCount);
    
    // Discard all modules (must do in reverse to avoid index shifting issues)
    for (int i = (int)moduleCount - 1; i >= 0; i--)
    {
        asIScriptModule* pModule = m_pEngine->GetModuleByIndex(i);
        if (pModule)
        {
            const char* moduleName = pModule->GetName();
            MS_ANGEL_DEBUG("  Discarding module: %s", moduleName ? moduleName : "<unnamed>");
            m_pEngine->DiscardModule(moduleName);
        }
    }
    
    MS_ANGEL_INFO("ClearAllModules: All modules cleared (remaining: %u)", m_pEngine->GetModuleCount());
}

//==========================================================================
// Prepare for level change - comprehensive cleanup
//==========================================================================
void CAngelScriptManager::PrepareForLevelChange()
{
    if (!m_bInitialized || !m_pEngine)
        return;
        
    MS_ANGEL_INFO("=== PrepareForLevelChange: Starting comprehensive AngelScript cleanup ===");
    
    // Step 1: Clear all pooled contexts
    // This prevents any stale bytecode pointers from being reused
    MS_ANGEL_INFO("Step 1: Clearing context pool...");
    ClearContextPool();
    
    // Step 2: Clear all script modules
    // This removes all compiled scripts and their entity references
    MS_ANGEL_INFO("Step 2: Clearing all script modules...");
    ClearAllModules();
    
    // Step 3: Run garbage collection to clean up any remaining script objects
    MS_ANGEL_INFO("Step 3: Running full garbage collection...");
    asUINT currentSize, totalDestroyed, totalDetected;
    m_pEngine->GetGCStatistics(&currentSize, &totalDestroyed, &totalDetected);
    MS_ANGEL_INFO("  GC before cleanup - Objects: %u, Destroyed: %u, Detected: %u", 
                  currentSize, totalDestroyed, totalDetected);
    
    // Force multiple full GC cycles to ensure everything is cleaned up
    for (unsigned int i = 0; i < 3; i++)
    {
        m_pEngine->GarbageCollect(asGC_FULL_CYCLE | asGC_DESTROY_GARBAGE);
    }
    
    m_pEngine->GetGCStatistics(&currentSize, &totalDestroyed, &totalDetected);
    MS_ANGEL_INFO("  GC after cleanup - Objects: %u, Destroyed: %u, Detected: %u", 
                  currentSize, totalDestroyed, totalDetected);
    
    if (currentSize > 0)
    {
        MS_ANGEL_WARN("  Warning: %u objects still in GC after cleanup (may be persistent global objects)", 
                        currentSize);
    }
    
    MS_ANGEL_INFO("=== PrepareForLevelChange: AngelScript cleanup complete ===");
    MS_ANGEL_INFO("  All entity references, bytecode, and script state cleared");
    MS_ANGEL_INFO("  Scripts will be reloaded when new map loads");
}

//==========================================================================
// Complete AngelScript engine reinitialization for level change
//==========================================================================
void CAngelScriptManager::ReinitializeForLevelChange()
{
    MS_ANGEL_INFO("=== ReinitializeForLevelChange: COMPLETE ENGINE TEARDOWN AND REBUILD ===");
    
    if (!m_bInitialized)
    {
        MS_ANGEL_ERROR("ReinitializeForLevelChange: Engine not initialized, cannot reinit");
        return;
    }
    
    // Step 1: Complete teardown
    MS_ANGEL_INFO("Step 1: Performing complete engine shutdown...");
    
    // Clear all contexts first
    ClearContextPool();
    
    // Clear all modules
    ClearAllModules();
    
    // Shutdown event manager to clear all event handlers before engine destruction
    ASEngineEventManager* pEventManager = ASEngineEventManager::Instance();
    if (pEventManager)
    {
        MS_ANGEL_INFO("  Destroying event manager...");
        pEventManager->Destroy();
    }
    
    // Shutdown debugger if it exists
    if (m_pDebugger)
    {
        MS_ANGEL_INFO("  Shutting down debugger...");
        m_pDebugger->Shutdown();
        delete m_pDebugger;
        m_pDebugger = nullptr;
    }
    
    // Run final garbage collection before destroying engine
    MS_ANGEL_INFO("  Running final garbage collection...");
    for (unsigned int i = 0; i < 5; i++)  // Extra cycles to be thorough
    {
        m_pEngine->GarbageCollect(asGC_FULL_CYCLE | asGC_DESTROY_GARBAGE | asGC_DETECT_GARBAGE);
    }
    
    // Get final GC stats
    asUINT currentSize, totalDestroyed, totalDetected;
    m_pEngine->GetGCStatistics(&currentSize, &totalDestroyed, &totalDetected);
    MS_ANGEL_INFO("  Final GC stats - Objects: %u, Destroyed: %u, Detected: %u", 
                  currentSize, totalDestroyed, totalDetected);
    
    // Release the engine completely
    MS_ANGEL_INFO("  Releasing AngelScript engine...");
    int refCount = m_pEngine->Release();
    MS_ANGEL_INFO("  Engine released (remaining refcount: %d)", refCount);
    m_pEngine = nullptr;
    m_bInitialized = false;
    m_nMemoryUsed = 0;
    
    MS_ANGEL_INFO("Step 2: Complete teardown finished - all AngelScript state destroyed");
    
    // Step 2: Complete reinitialization
    MS_ANGEL_INFO("Step 3: Rebuilding AngelScript engine from scratch...");
    
    if (!Initialize())
    {
        MS_ANGEL_ERROR("ReinitializeForLevelChange: CRITICAL - Failed to reinitialize AngelScript engine!");
        return;
    }
    
    MS_ANGEL_INFO("=== ReinitializeForLevelChange: ENGINE REBUILD COMPLETE ===");
    MS_ANGEL_INFO("  AngelScript engine recreated with clean state");
    MS_ANGEL_INFO("  All registrations, bindings, and systems reinitialized");
    MS_ANGEL_INFO("  Ready for script loading");
}

//==========================================================================
// Release context to pool
//==========================================================================
void CAngelScriptManager::ReleaseContext(asIScriptContext* pContext)
{
    if (!pContext)
        return;
        
    // Reset the context state
    pContext->Unprepare();
    
    // If pool isn't full, add the context back to the pool
    if (m_ContextPool.size() < MAX_CONTEXT_POOL_SIZE)
    {
        m_ContextPool.push_back(pContext);
    }
    else
    {
        // Pool is full, release the context
        pContext->Release();
    }
}

//==========================================================================
// Think - called every frame for maintenance
//==========================================================================
void CAngelScriptManager::Think()
{
    if (!m_bInitialized || !m_pEngine)
        return;
        
    // Run garbage collection step
    m_pEngine->GarbageCollect(asGC_ONE_STEP);
    
    // Update optimization systems
    float fCurrentTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;
    
    // Process coroutines
    ASCoroutineManager* pCoroutineManager = ASCoroutineManager::Instance();
    if (pCoroutineManager)
    {
        pCoroutineManager->Think(fCurrentTime);
    }
    
    // Process memory optimization systems
    ThinkOptimizationSystems();
}

//==========================================================================
// Log message with AngelScript prefix
//==========================================================================
void CAngelScriptManager::LogMessage(const char* szMessage, int nLevel)
{
    // Use MSLogger for proper categorized logging
    if (nLevel > 0)  // Error level
    {
        MS_ANGEL_ERROR("%s", szMessage);
    }
    else  // Info level
    {
        MS_ANGEL_INFO("%s", szMessage);
    }
}

//==========================================================================
// CallGlobalFunction - Execute a global AngelScript function
//==========================================================================
bool CAngelScriptManager::CallGlobalFunction(const char* szFunctionName, std::optional<std::string> szModuleName)
{
    if (!m_bInitialized || !m_pEngine || !szFunctionName)
    {
        MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunction: Invalid state or parameters");
        return false;
    }

    MS_ANGEL_INFO("=== CallGlobalFunction: Looking for '%s' in module '%s' ===", 
                  szFunctionName, szModuleName.has_value() ? szModuleName->c_str() : "ALL");

    // If module name is specified, search only in that module
    if (szModuleName.has_value())
    {
        asIScriptModule* pModule = m_pEngine->GetModule(szModuleName->c_str());
        if (!pModule)
        {
            MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunction: Module '%s' not found", szModuleName->c_str());
            return false;
        }

        asIScriptFunction* pFunction = pModule->GetFunctionByName(szFunctionName);
        if (!pFunction)
        {
            MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunction: Function '%s' not found in module '%s'", 
                          szFunctionName, szModuleName->c_str());
            
            // List all functions in the module for debugging
            MS_ANGEL_INFO("Functions in module '%s':", szModuleName->c_str());
            for (asUINT j = 0; j < pModule->GetFunctionCount(); j++)
            {
                asIScriptFunction* pFunc = pModule->GetFunctionByIndex(j);
                if (pFunc)
                {
                    MS_ANGEL_INFO("  - %s", pFunc->GetName());
                }
            }
            
            return false;
        }

        MS_ANGEL_INFO("Found function '%s' in module '%s', executing...", szFunctionName, szModuleName->c_str());
        return this->ExecuteFunction(pFunction, szFunctionName);
    }
    
    // Search all modules for the function
    bool functionFound = false;
    bool anySuccess = false;
    
    MS_ANGEL_INFO("Searching all %d modules for function '%s'...", m_pEngine->GetModuleCount(), szFunctionName);
    
    for (asUINT i = 0; i < m_pEngine->GetModuleCount(); i++)
    {
        asIScriptModule* pModule = m_pEngine->GetModuleByIndex(i);
        if (!pModule) continue;
        
        MS_ANGEL_INFO("Checking module '%s'...", pModule->GetName());
        
        asIScriptFunction* pFunction = pModule->GetFunctionByName(szFunctionName);
        if (pFunction)
        {
            functionFound = true;
            MS_ANGEL_INFO("CAngelScriptManager::CallGlobalFunction: Calling '%s' in module '%s'", 
                         szFunctionName, pModule->GetName());
            
            if (this->ExecuteFunction(pFunction, szFunctionName))
            {
                anySuccess = true;
            }
        }
    }
    
    if (!functionFound)
    {
        MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunction: Function '%s' not found in any module", 
                      szFunctionName);
        
        // List all modules and their functions for debugging
        MS_ANGEL_INFO("=== Module Summary ===");
        for (asUINT i = 0; i < m_pEngine->GetModuleCount(); i++)
        {
            asIScriptModule* pModule = m_pEngine->GetModuleByIndex(i);
            if (!pModule) continue;
            
            MS_ANGEL_INFO("Module '%s' has %d functions:", pModule->GetName(), pModule->GetFunctionCount());
            for (asUINT j = 0; j < pModule->GetFunctionCount() && j < 10; j++) // Show first 10
            {
                asIScriptFunction* pFunc = pModule->GetFunctionByIndex(j);
                if (pFunc)
                {
                    MS_ANGEL_INFO("  - %s", pFunc->GetName());
                }
            }
            if (pModule->GetFunctionCount() > 10)
            {
                MS_ANGEL_INFO("  ... and %d more functions", pModule->GetFunctionCount() - 10);
            }
        }
        
        return false;
    }
    
    return anySuccess;
}

//==========================================================================
// IsParameterByReference - Helper function to determine if parameter should be passed by reference
//==========================================================================
bool CAngelScriptManager::IsParameterByReference(asIScriptFunction* pFunction, int paramIndex)
{
    if (!pFunction || paramIndex < 0 || static_cast<asUINT>(paramIndex) >= pFunction->GetParamCount())
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
// CallGlobalFunctionWithParams - Execute a global AngelScript function with string parameters
//==========================================================================
bool CAngelScriptManager::CallGlobalFunctionWithParams(const char* szFunctionName, const std::optional<std::vector<std::string>>& params, std::optional<std::string> szModuleName)
{
    if (!m_bInitialized || !m_pEngine || !szFunctionName)
    {
        MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Invalid state or parameters");
        return false;
    }

    const int paramCount = params.has_value() ? (int)params->size() : 0;
    
    // Skip verbose logging for frequently called functions to reduce spam
    bool isFrequentFunction = (strcmp(szFunctionName, "GameThink") == 0);
    
    if (!isFrequentFunction)
    {
        MS_ANGEL_INFO("=== CallGlobalFunctionWithParams: Looking for '%s' in module '%s' with %d params ===", 
                      szFunctionName, szModuleName.has_value() ? szModuleName->c_str() : "ALL", paramCount);
    }

    // If module name is specified, search only in that module
    if (szModuleName.has_value())
    {
        asIScriptModule* pModule = m_pEngine->GetModule(szModuleName->c_str());
        if (!pModule)
        {
            MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Module '%s' not found", szModuleName->c_str());
            return false;
        }

        asIScriptFunction* pFunction = pModule->GetFunctionByName(szFunctionName);
        if (!pFunction)
        {
            MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Function '%s' not found in module '%s'", 
                          szFunctionName, szModuleName->c_str());
            return false;
        }

        // Get context
        asIScriptContext* pContext = AcquireContext();
        if (!pContext)
        {
            MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Failed to acquire context");
            return false;
        }

        // Prepare function
        int r = pContext->Prepare(pFunction);
        if (r < 0)
        {
            MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Failed to prepare function");
            ReleaseContext(pContext);
            return false;
        }

        // Set string arguments - check if parameter is passed by reference
        if (params.has_value())
        {
            for (size_t i = 0; i < params->size() && i < (size_t)pFunction->GetParamCount(); i++)
            {
                if (IsParameterByReference(pFunction, i))
                    r = pContext->SetArgAddress(i, const_cast<std::string*>(&(*params)[i]));
                else
                    r = pContext->SetArgObject(i, const_cast<std::string*>(&(*params)[i]));
                    
                if (r < 0)
                {
                    MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Failed to set argument %d", (int)i);
                    ReleaseContext(pContext);
                    return false;
                }
            }
        }

        // Execute
        r = pContext->Execute();
        ReleaseContext(pContext);

        if (r != asEXECUTION_FINISHED)
        {
            if (r == asEXECUTION_EXCEPTION)
            {
                MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Exception in function '%s': %s", 
                              szFunctionName, pContext->GetExceptionString());
            }
            else
            {
                MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Failed to execute function '%s' (result: %d)", 
                              szFunctionName, r);
            }
            return false;
        }

        // Only log success for non-frequent functions
        if (!isFrequentFunction)
        {
            MS_ANGEL_INFO("Successfully called function '%s' with parameters", szFunctionName);
        }
        return true;
    }
    
    // Search all modules for the function
    bool functionFound = false;
    bool anySuccess = false;
    
    for (asUINT i = 0; i < m_pEngine->GetModuleCount(); i++)
    {
        asIScriptModule* pModule = m_pEngine->GetModuleByIndex(i);
        if (!pModule) continue;
        
        asIScriptFunction* pFunction = pModule->GetFunctionByName(szFunctionName);
        if (pFunction)
        {
            functionFound = true;
            
            // Only log for non-frequent functions to reduce spam
            if (!isFrequentFunction)
            {
                MS_ANGEL_INFO("CAngelScriptManager::CallGlobalFunctionWithParams: Calling '%s' in module '%s'", 
                             szFunctionName, pModule->GetName());
            }
            
            // Get context
            asIScriptContext* pContext = AcquireContext();
            if (!pContext)
            {
                MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Failed to acquire context");
                continue;
            }

            // Prepare function
            int r = pContext->Prepare(pFunction);
            if (r < 0)
            {
                MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Failed to prepare function");
                ReleaseContext(pContext);
                continue;
            }

            // Set string arguments - check if parameter is passed by reference
            bool argsOk = true;
            if (params.has_value())
            {
                for (size_t j = 0; j < params->size() && j < (size_t)pFunction->GetParamCount(); j++)
                {
                    if (IsParameterByReference(pFunction, j))
                        r = pContext->SetArgAddress(j, const_cast<std::string*>(&(*params)[j]));
                    else
                        r = pContext->SetArgObject(j, const_cast<std::string*>(&(*params)[j]));
                        
                    if (r < 0)
                    {
                        MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Failed to set argument %d", (int)j);
                        argsOk = false;
                        break;
                    }
                }
            }

            if (!argsOk)
            {
                ReleaseContext(pContext);
                continue;
            }

            // Execute
            r = pContext->Execute();
            ReleaseContext(pContext);

            if (r == asEXECUTION_FINISHED)
            {
                anySuccess = true;
                //MS_ANGEL_INFO("Successfully called function '%s' with parameters", szFunctionName);
            }
            else
            {
                if (r == asEXECUTION_EXCEPTION)
                {
                    MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Exception in function '%s': %s", 
                                  szFunctionName, pContext->GetExceptionString());
                }
                else
                {
                    MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Failed to execute function '%s' (result: %d)", 
                                  szFunctionName, r);
                }
            }
        }
    }
    
    if (!functionFound)
    {
        MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunctionWithParams: Function '%s' not found in any module", 
                      szFunctionName);
        return false;
    }
    
    return anySuccess;
}

//==========================================================================
// ExecuteFunction - Helper to execute a function with proper context handling
//==========================================================================
bool CAngelScriptManager::ExecuteFunction(asIScriptFunction* pFunction, const char* szFunctionName)
{
    if (!pFunction)
    {
        MS_ANGEL_ERROR("CAngelScriptManager::ExecuteFunction: NULL function pointer");
        return false;
    }
    
    // Validate the function before execution
    asIScriptModule* pModule = pFunction->GetModule();
    if (!pModule)
    {
        MS_ANGEL_ERROR("CAngelScriptManager::ExecuteFunction: Function '%s' has NULL module - STALE FUNCTION POINTER", 
                      szFunctionName);
        MS_ANGEL_ERROR("  This function was likely from a module that has been unloaded");
        return false;
    }
    
    // Verify the module is still registered with the engine
    const char* moduleName = pModule->GetName();
    if (!moduleName)
    {
        MS_ANGEL_ERROR("CAngelScriptManager::ExecuteFunction: Module has NULL name for function '%s'", 
                      szFunctionName);
        return false;
    }
    
    asIScriptModule* pEngineModule = m_pEngine->GetModule(moduleName);
    if (!pEngineModule || pEngineModule != pModule)
    {
        MS_ANGEL_ERROR("CAngelScriptManager::ExecuteFunction: Module '%s' not registered with engine - STALE FUNCTION POINTER", 
                      moduleName);
        MS_ANGEL_ERROR("  Function: '%s', Expected module: %p, Found module: %p", 
                      szFunctionName, pModule, pEngineModule);
        return false;
    }
    
    MS_ANGEL_INFO("ExecuteFunction: About to execute '%s' from module '%s' (validated)", 
                  szFunctionName, moduleName);
        
    // Acquire context from pool
    asIScriptContext* pContext = this->AcquireContext();
    if (!pContext)
    {
        MS_ANGEL_ERROR("CAngelScriptManager::ExecuteFunction: Failed to acquire context for '%s'", szFunctionName);
        return false;
    }
    
    // Prepare the function call
    int r = pContext->Prepare(pFunction);
    if (r < 0)
    {
        MS_ANGEL_ERROR("CAngelScriptManager::ExecuteFunction: Failed to prepare function '%s' (error: %d)", 
                      szFunctionName, r);
        MS_ANGEL_ERROR("  This could indicate corrupted bytecode or invalid function state");
        this->ReleaseContext(pContext);
        return false;
    }
    
    MS_ANGEL_INFO("ExecuteFunction: Function '%s' prepared, executing...", szFunctionName);
    
    // Execute the function
    r = pContext->Execute();
    if (r != asEXECUTION_FINISHED)
    {
        if (r == asEXECUTION_EXCEPTION)
        {
            MS_ANGEL_ERROR("CAngelScriptManager::ExecuteFunction: Exception in function '%s': %s", 
                          szFunctionName, pContext->GetExceptionString());
            
            // Get exception details
            int line = pContext->GetExceptionLineNumber();
            const char* section = nullptr;
            asIScriptFunction* func = pContext->GetExceptionFunction();
            if (func)
            {
                MS_ANGEL_ERROR("  Exception in: %s", func->GetDeclaration());
                MS_ANGEL_ERROR("  Module: %s", func->GetModuleName());
                MS_ANGEL_ERROR("  Line: %d", line);
            }
        }
        else
        {
            MS_ANGEL_ERROR("CAngelScriptManager::ExecuteFunction: Failed to execute function '%s' (result: %d)", 
                          szFunctionName, r);
        }
        this->ReleaseContext(pContext);
        return false;
    }
    
    MS_ANGEL_INFO("CAngelScriptManager::ExecuteFunction: Successfully executed function '%s'", szFunctionName);
    this->ReleaseContext(pContext);
    return true;
}

//==========================================================================
// Memory allocation tracking
//==========================================================================
int timesSent = 0;
void* ASMalloc(size_t size)
{
    void* ptr = malloc(size);
    if (ptr && CAngelScriptManager::s_pInstance)
    {
        CAngelScriptManager::s_pInstance->m_nMemoryUsed += size;
        
        // Check if we've exceeded memory limit
        if (CAngelScriptManager::s_pInstance->m_nMemoryUsed > 
            CAngelScriptManager::s_pInstance->m_nMemoryLimit && timesSent < 10)
        {
            MS_ANGEL_INFO("WARNING: Memory limit exceeded! Used: %zu, Limit: %zu",
                          CAngelScriptManager::s_pInstance->m_nMemoryUsed,
                   CAngelScriptManager::s_pInstance->m_nMemoryLimit);
                
            timesSent++;

        }
    }
    return ptr;
}

void ASFree(void* ptr)
{
    if (ptr)
    {
        // Note: We can't easily track the size being freed without additional bookkeeping
        // For now, just free the memory - proper tracking will be added later
        free(ptr);
    }
}