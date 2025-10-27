#ifndef ANGELSCRIPTMANAGER_H
#define ANGELSCRIPTMANAGER_H

#include <cstddef>  // for size_t
#include <vector>
#include <string>

// Forward declarations
class asIScriptEngine;
class asIScriptModule;
class asIScriptContext;
class asIScriptFunction;
class ASDebugger;

//==========================================================================
// CAngelScriptManager - Singleton manager for AngelScript engine
// 
// Manages the lifecycle of the AngelScript engine, modules, and contexts
// Provides memory tracking for 32-bit constraints
// Thread-safe initialization and context pooling
//==========================================================================
class CAngelScriptManager
{
private:
    static CAngelScriptManager* s_pInstance;
    
    asIScriptEngine* m_pEngine;
    bool m_bInitialized;
    
    // Memory tracking for 32-bit constraints
    size_t m_nMemoryUsed;
    size_t m_nMemoryLimit;
    
    // Context pool management
    std::vector<asIScriptContext*> m_ContextPool;
    static const size_t MAX_CONTEXT_POOL_SIZE = 8;  // Maximum contexts to keep in pool
    
    // Debugger
    ASDebugger* m_pDebugger;
    
    // Private constructor for singleton
    CAngelScriptManager();
    
public:
    ~CAngelScriptManager();
    
    // Singleton access
    static CAngelScriptManager* Instance();
    static void Shutdown();
    
    // Core functionality
    bool Initialize();
    void Destroy();
    
    // Engine access
    asIScriptEngine* GetEngine() const { return m_pEngine; }
    bool IsInitialized() const { return m_bInitialized; }
    
    // Memory management
    size_t GetMemoryUsed() const { return m_nMemoryUsed; }
    size_t GetMemoryLimit() const { return m_nMemoryLimit; }
    void SetMemoryLimit(size_t nBytes) { m_nMemoryLimit = nBytes; }
    
    // Module management
    asIScriptModule* GetModule(const char* szModuleName);
    bool CompileModule(const char* szModuleName, const char* szScript);
    void RemoveModule(const char* szModuleName);
    
    // Context pool
    asIScriptContext* AcquireContext();
    void ReleaseContext(asIScriptContext* pContext);
    void ClearContextPool(); // Clear all pooled contexts (call during level changes)
    
    // Level change handling
    void PrepareForLevelChange(); // Comprehensive cleanup for level transitions
    void ReinitializeForLevelChange(); // Complete teardown and reinit of AngelScript engine
    void ClearAllModules(); // Remove all script modules
    
    // Global function execution
    bool CallGlobalFunction(const char* szFunctionName, const char* szModuleName = nullptr);
    bool CallGlobalFunctionWithParams(const char* szFunctionName, const std::vector<std::string>& params, const char* szModuleName = nullptr);
    
    // Debugger access
    ASDebugger* GetDebugger() const { return m_pDebugger; }
    
private:
    // Helper function for function execution
    bool ExecuteFunction(asIScriptFunction* pFunction, const char* szFunctionName);
    
public:
    // Update and maintenance
    void Think(); // Called from MSGameThink() for garbage collection
    
    // Debug and logging
    void LogMessage(const char* szMessage, int nLevel = 0);
    
    // Friend functions for memory tracking
    friend void* ASMalloc(size_t size);
    friend void ASFree(void* ptr);
};

// Memory allocation tracking functions
void* ASMalloc(size_t size);
void ASFree(void* ptr);

#endif // ANGELSCRIPTMANAGER_H