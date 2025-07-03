//==========================================================================
// ASCoroutines.h - AngelScript Coroutine System
// Master Sword Rebirth AngelScript Implementation
//==========================================================================

#ifndef ASCOROUTINES_H
#define ASCOROUTINES_H

#include <vector>
#include <memory>
#include <string>

class asIScriptEngine;
class asIScriptContext;
class asIScriptFunction;

//==========================================================================
// Coroutine state enumeration
//==========================================================================
enum class ASCoroutineState
{
    RUNNING,      // Coroutine is actively running
    SUSPENDED,    // Coroutine is suspended (waiting for timer/condition)
    COMPLETED,    // Coroutine finished execution
    CANCELLED,    // Coroutine was cancelled
    ERROR         // Coroutine encountered an error
};

//==========================================================================
// Individual coroutine instance
// Manages script execution state across multiple frames
//==========================================================================
class ASCoroutine
{
private:
    asIScriptContext* m_pContext;
    asIScriptFunction* m_pFunction;
    std::string m_sFunctionName;
    
    ASCoroutineState m_eState;
    float m_fResumeTime;        // When to resume execution (for delays)
    float m_fStartTime;         // When coroutine started
    int m_nCoroutineId;         // Unique ID for this coroutine
    
    // Error handling
    std::string m_sLastError;
    
    // Execution tracking
    int m_nExecutionCount;      // How many times we've executed
    float m_fTotalRunTime;      // Total time spent executing
    
public:
    ASCoroutine(int nId, asIScriptFunction* pFunction, const char* szFunctionName);
    ~ASCoroutine();
    
    // Core execution
    bool Initialize();
    bool Execute();
    bool Resume();
    void Suspend(float fDelay = 0.0f);
    void Cancel();
    
    // State management
    ASCoroutineState GetState() const { return m_eState; }
    bool IsRunning() const { return m_eState == ASCoroutineState::RUNNING; }
    bool IsSuspended() const { return m_eState == ASCoroutineState::SUSPENDED; }
    bool IsCompleted() const { return m_eState == ASCoroutineState::COMPLETED || m_eState == ASCoroutineState::CANCELLED; }
    bool HasError() const { return m_eState == ASCoroutineState::ERROR; }
    
    // Timing
    bool ShouldResume(float fCurrentTime) const;
    float GetResumeTime() const { return m_fResumeTime; }
    float GetRunTime() const { return m_fTotalRunTime; }
    
    // Information
    int GetId() const { return m_nCoroutineId; }
    const char* GetFunctionName() const { return m_sFunctionName.c_str(); }
    const char* GetLastError() const { return m_sLastError.c_str(); }
    int GetExecutionCount() const { return m_nExecutionCount; }
    
    // Context management
    asIScriptContext* GetContext() const { return m_pContext; }
    void SetContext(asIScriptContext* pContext) { m_pContext = pContext; }
    
private:
    void SetError(const char* szError);
    void UpdateExecutionStats(float fStartTime, float fEndTime);
};

//==========================================================================
// Coroutine manager - handles lifecycle of all coroutines
// Integrates with game loop for frame-based execution
//==========================================================================
class ASCoroutineManager
{
private:
    std::vector<std::unique_ptr<ASCoroutine>> m_Coroutines;
    int m_nNextCoroutineId;
    
    // Performance tracking
    int m_nActiveCoroutines;
    int m_nTotalCoroutinesCreated;
    float m_fTotalExecutionTime;
    
    // Configuration
    int m_nMaxConcurrentCoroutines;
    float m_fMaxExecutionTimePerFrame;
    
    // Singleton instance
    static ASCoroutineManager* s_pInstance;
    
    ASCoroutineManager();
    
public:
    ~ASCoroutineManager();
    
    // Singleton access
    static ASCoroutineManager* Instance();
    static void Shutdown();
    
    // Coroutine lifecycle
    int StartCoroutine(const char* szFunctionName);
    int StartCoroutine(asIScriptFunction* pFunction, const char* szFunctionName);
    bool StopCoroutine(int nCoroutineId);
    void StopAllCoroutines();
    
    // Frame processing - called from game loop
    void Think(float fCurrentTime);
    
    // Coroutine control
    bool SuspendCoroutine(int nCoroutineId, float fDelay = 0.0f);
    bool ResumeCoroutine(int nCoroutineId);
    bool IsCoroutineRunning(int nCoroutineId) const;
    
    // Information and debugging
    int GetActiveCoroutineCount() const { return m_nActiveCoroutines; }
    int GetTotalCoroutinesCreated() const { return m_nTotalCoroutinesCreated; }
    float GetTotalExecutionTime() const { return m_fTotalExecutionTime; }
    
    // Configuration
    void SetMaxConcurrentCoroutines(int nMax) { m_nMaxConcurrentCoroutines = nMax; }
    void SetMaxExecutionTimePerFrame(float fMaxTime) { m_fMaxExecutionTimePerFrame = fMaxTime; }
    
    // Utility functions for scripts
    void Delay(float fSeconds);  // Suspend current coroutine for specified time
    void Yield();                // Yield execution to next frame
    
    // Cleanup
    void CleanupCompletedCoroutines();
    
private:
    ASCoroutine* FindCoroutine(int nCoroutineId);
    void RemoveCoroutine(int nCoroutineId);
    float GetCurrentTime() const;
    
    // Performance monitoring
    void UpdatePerformanceStats();
    
    // Integration with existing AngelScript manager
    asIScriptFunction* FindGlobalFunction(const char* szFunctionName);
};

//==========================================================================
// Global functions for script access
//==========================================================================

// Register coroutine functions with AngelScript
void RegisterCoroutineFunctions(asIScriptEngine* pEngine);

// Script-callable functions
int StartCoroutine(const std::string& sFunctionName);
void StopCoroutine(int nCoroutineId);
void DelaySeconds(float fSeconds);
void YieldFrame();
bool IsCoroutineRunning(int nCoroutineId);

#endif // ASCOROUTINES_H