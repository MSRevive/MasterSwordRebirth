#include "CAngelScriptManager.h"
#include <cstdio>  // for printf
#include <cstring> // for memset
#include <cstdlib> // for malloc, free
#include <ctime>   // for clock, CLOCKS_PER_SEC
#include <new>     // for placement new
#include <vector>  // for std::vector
#include "../mslogger.h"  // Include MSLogger for unified logging
#include "cvardef.h"        // For cvar_t definition

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
        MS_ANGEL_ERROR("%s (%d, %d): %s", 
                       msg->section ? msg->section : "Unknown", 
                       msg->row, msg->col, msg->message);
    }
    else if (msg->type == asMSGTYPE_WARNING) 
    {
        MS_ANGEL_INFO("WARNING: %s (%d, %d): %s", 
                      msg->section ? msg->section : "Unknown", 
                      msg->row, msg->col, msg->message);
    }
    else
    {
        MS_ANGEL_INFO("%s (%d, %d): %s", 
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
        
    // Release all contexts in the pool
    for (size_t i = 0; i < m_ContextPool.size(); i++)
    {
        if (m_ContextPool[i])
        {
            m_ContextPool[i]->Release();
        }
    }
    m_ContextPool.clear();
    
    // Shutdown debugger
    if (m_pDebugger)
    {
        m_pDebugger->Shutdown();
        delete m_pDebugger;
        m_pDebugger = nullptr;
    }
    
    // Shutdown optimization systems
    ShutdownOptimizationSystems();
    
    // Shutdown coroutine manager
    ASCoroutineManager::Shutdown();
    
    // Clean up the engine
    if (m_pEngine)
    {
        m_pEngine->ShutDownAndRelease();
        m_pEngine = nullptr;
    }
    
    m_bInitialized = false;
    
    LogMessage("AngelScript Manager destroyed");
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
        
        // Make sure the context is in a clean state
        pContext->Unprepare();
    }
    else
    {
        // Create a new context if pool is empty
        pContext = m_pEngine->CreateContext();
        if (!pContext)
        {
            LogMessage("Failed to create script context", 1);
        }
    }
    
    // Set line callback for debugging if debugger is enabled
    if (pContext && m_pDebugger && IsDebugModeEnabled())
    {
        pContext->SetLineCallback(asFUNCTION(ASDebugger::LineCallback), m_pDebugger, asCALL_CDECL_OBJLAST);
        pContext->SetExceptionCallback(asFUNCTION(ASDebugger::ExceptionCallback), m_pDebugger, asCALL_CDECL_OBJLAST);
    }
    
    return pContext;
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
bool CAngelScriptManager::CallGlobalFunction(const char* szFunctionName, const char* szModuleName)
{
    if (!m_bInitialized || !m_pEngine || !szFunctionName)
    {
        MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunction: Invalid state or parameters");
        return false;
    }

    MS_ANGEL_INFO("=== CallGlobalFunction: Looking for '%s' in module '%s' ===", 
                  szFunctionName, szModuleName ? szModuleName : "ALL");

    // If module name is specified, search only in that module
    if (szModuleName)
    {
        asIScriptModule* pModule = m_pEngine->GetModule(szModuleName);
        if (!pModule)
        {
            MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunction: Module '%s' not found", szModuleName);
            return false;
        }

        asIScriptFunction* pFunction = pModule->GetFunctionByName(szFunctionName);
        if (!pFunction)
        {
            MS_ANGEL_ERROR("CAngelScriptManager::CallGlobalFunction: Function '%s' not found in module '%s'", 
                          szFunctionName, szModuleName);
            
            // List all functions in the module for debugging
            MS_ANGEL_INFO("Functions in module '%s':", szModuleName);
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

        MS_ANGEL_INFO("Found function '%s' in module '%s', executing...", szFunctionName, szModuleName);
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
// ExecuteFunction - Helper to execute a function with proper context handling
//==========================================================================
bool CAngelScriptManager::ExecuteFunction(asIScriptFunction* pFunction, const char* szFunctionName)
{
    if (!pFunction)
        return false;
    
    MS_ANGEL_INFO("ExecuteFunction: About to execute '%s'", szFunctionName);
        
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