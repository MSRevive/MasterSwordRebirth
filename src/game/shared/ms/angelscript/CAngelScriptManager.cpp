#include "CAngelScriptManager.h"
#include <cstdio>  // for printf
#include <cstring> // for memset
#include <cstdlib> // for malloc, free
#include <new>     // for placement new
#include <vector>  // for std::vector

// Only include AngelScript if we're compiling with it enabled
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244) // conversion warnings from AngelScript
#pragma warning(disable: 4996) // deprecated function warnings
#endif

#include "angelscript.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Static instance
CAngelScriptManager* CAngelScriptManager::s_pInstance = nullptr;

//==========================================================================
// Constructor
//==========================================================================
CAngelScriptManager::CAngelScriptManager()
    : m_pEngine(nullptr)
    , m_bInitialized(false)
    , m_nMemoryUsed(0)
    , m_nMemoryLimit(134217728) // 128MB default limit
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
    const char* szType = "INFO";
    if (msg->type == asMSGTYPE_ERROR) 
        szType = "ERROR";
    else if (msg->type == asMSGTYPE_WARNING) 
        szType = "WARNING";
    
    // For now, just output to console - will integrate with MS logging later
    printf("[AngelScript %s] %s (%d, %d): %s\n", 
           szType, msg->section, msg->row, msg->col, msg->message);
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
    
    // Configure engine properties for 32-bit constraints
    r = m_pEngine->SetEngineProperty(asEP_MAX_STACK_SIZE, 1024*1024); // 1MB stack
    if (r < 0)
    {
        LogMessage("Failed to set AngelScript stack size", 1);
    }
    
    // Set up memory allocation hooks
    r = asSetGlobalMemoryFunctions(ASMalloc, ASFree);
    if (r < 0)
    {
        LogMessage("Failed to set AngelScript memory functions", 1);
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
}

//==========================================================================
// Log message with AngelScript prefix
//==========================================================================
void CAngelScriptManager::LogMessage(const char* szMessage, int nLevel)
{
    // TODO: Integrate with MS logging system
    // For now, do nothing - will implement when integrated with svglobals
}

//==========================================================================
// Memory allocation tracking
//==========================================================================
void* ASMalloc(size_t size)
{
    void* ptr = malloc(size);
    if (ptr && CAngelScriptManager::s_pInstance)
    {
        CAngelScriptManager::s_pInstance->m_nMemoryUsed += size;
        
        // Check if we've exceeded memory limit
        if (CAngelScriptManager::s_pInstance->m_nMemoryUsed > 
            CAngelScriptManager::s_pInstance->m_nMemoryLimit)
        {
            printf("[AngelScript] WARNING: Memory limit exceeded! Used: %zu, Limit: %zu\n",
                   CAngelScriptManager::s_pInstance->m_nMemoryUsed,
                   CAngelScriptManager::s_pInstance->m_nMemoryLimit);
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