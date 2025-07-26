#ifndef ASDEBUGGER_H
#define ASDEBUGGER_H

#include <vector>
#include <map>
#include <string>
#include <angelscript.h>

// Forward declarations
class asIScriptEngine;
class asIScriptContext;
class asIScriptFunction;

/**
 * ASBreakpoint - Represents a script breakpoint
 */
struct ASBreakpoint
{
    std::string scriptName;
    int lineNumber;
    bool enabled;
    int hitCount;
    
    ASBreakpoint() : lineNumber(0), enabled(true), hitCount(0) {}
    ASBreakpoint(const std::string& script, int line) 
        : scriptName(script), lineNumber(line), enabled(true), hitCount(0) {}
};

/**
 * ASCallFrame - Represents a call stack frame
 */
struct ASCallFrame
{
    std::string functionName;
    std::string scriptName;
    int lineNumber;
    int columnNumber;
    
    ASCallFrame() : lineNumber(0), columnNumber(0) {}
};

/**
 * ASVariableInfo - Represents variable inspection data
 */
struct ASVariableInfo
{
    std::string name;
    std::string type;
    std::string value;
    bool canExpand;
    
    ASVariableInfo() : canExpand(false) {}
};

/**
 * ASProfilerEntry - Represents profiling data for a function
 */
struct ASProfilerEntry
{
    std::string functionName;
    std::string scriptName;
    unsigned int callCount;
    double totalTime;
    double averageTime;
    double minTime;
    double maxTime;
    
    ASProfilerEntry() : callCount(0), totalTime(0.0), averageTime(0.0), 
                       minTime(0.0), maxTime(0.0) {}
};

/**
 * ASDebugger - Main debugging interface for AngelScript
 * Provides breakpoint management, variable inspection, call stack tracking,
 * and performance profiling capabilities.
 */
class ASDebugger
{
public:
    ASDebugger();
    ~ASDebugger();
    
    // Initialization
    bool Initialize(asIScriptEngine* pEngine);
    void Shutdown();
    
    // Breakpoint Management
    bool AddBreakpoint(const std::string& scriptName, int lineNumber);
    bool RemoveBreakpoint(const std::string& scriptName, int lineNumber);
    bool EnableBreakpoint(const std::string& scriptName, int lineNumber, bool enable);
    void ClearAllBreakpoints();
    std::vector<ASBreakpoint> GetBreakpoints() const;
    bool HasBreakpoint(const std::string& scriptName, int lineNumber) const;
    
    // Execution Control
    void EnableStepping(bool enable);
    bool IsSteppingEnabled() const;
    void StepInto();
    void StepOver();
    void StepOut();
    void Continue();
    
    // Variable Inspection
    std::vector<ASVariableInfo> GetLocalVariables(asIScriptContext* pContext);
    std::vector<ASVariableInfo> GetGlobalVariables();
    ASVariableInfo GetVariableInfo(const std::string& varName, asIScriptContext* pContext);
    bool SetVariableValue(const std::string& varName, const std::string& value, asIScriptContext* pContext);
    
    // Call Stack
    std::vector<ASCallFrame> GetCallStack(asIScriptContext* pContext);
    
    // Script State
    bool IsExecutionPaused() const;
    std::string GetCurrentScript() const;
    int GetCurrentLine() const;
    
    // Debug Output
    void LogMessage(const std::string& message);
    void LogWarning(const std::string& warning);
    void LogError(const std::string& error);
    
    // Callbacks for AngelScript engine
    static void LineCallback(asIScriptContext* pContext, void* pUserData);
    static void ExceptionCallback(asIScriptContext* pContext, void* pUserData);
    
private:
    asIScriptEngine* m_pEngine;
    std::vector<ASBreakpoint> m_breakpoints;
    bool m_bSteppingEnabled;
    bool m_bExecutionPaused;
    std::string m_currentScript;
    int m_currentLine;
    
    // Internal helpers
    bool CheckBreakpoint(const std::string& scriptName, int lineNumber);
    void HandleBreakpoint(asIScriptContext* pContext);
    std::string FormatVariableValue(void* pValue, int typeId);
    
    // Prevent copy/assignment
    ASDebugger(const ASDebugger&);
    ASDebugger& operator=(const ASDebugger&);
};

/**
 * ASProfiler - Performance profiling for AngelScript execution
 * Monitors script performance, tracks function call times, and identifies bottlenecks.
 */
class ASProfiler
{
public:
    ASProfiler();
    ~ASProfiler();
    
    // Initialization
    bool Initialize(asIScriptEngine* pEngine);
    void Shutdown();
    
    // Profiling Control
    void StartProfiling();
    void StopProfiling();
    bool IsProfilingActive() const;
    void Reset();
    
    // Data Collection
    void RecordFunctionStart(const std::string& functionName, const std::string& scriptName);
    void RecordFunctionEnd(const std::string& functionName, const std::string& scriptName, double duration);
    
    // Results
    std::vector<ASProfilerEntry> GetProfileResults() const;
    ASProfilerEntry GetFunctionProfile(const std::string& functionName) const;
    
    // Memory Tracking
    void RecordMemoryUsage(size_t bytesUsed);
    size_t GetPeakMemoryUsage() const;
    size_t GetCurrentMemoryUsage() const;
    
    // Reports
    void GenerateReport(const std::string& filename) const;
    void LogSummary() const;
    
    // Callbacks
    static void FunctionStartCallback(asIScriptContext* pContext, void* pUserData);
    static void FunctionEndCallback(asIScriptContext* pContext, void* pUserData);
    
private:
    asIScriptEngine* m_pEngine;
    bool m_bProfilingActive;
    std::map<std::string, ASProfilerEntry> m_profileData;
    size_t m_currentMemoryUsage;
    size_t m_peakMemoryUsage;
    
    // Timing helpers
    std::map<std::string, double> m_functionStartTimes;
    double GetProfilerTime() const;
    
    // Prevent copy/assignment
    ASProfiler(const ASProfiler&);
    ASProfiler& operator=(const ASProfiler&);
};

#endif // ASDEBUGGER_H