#include "ASDebugger.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <iomanip>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/time.h>
#endif

//-----------------------------------------------------------------------------
// ASDebugger Implementation
//-----------------------------------------------------------------------------

ASDebugger::ASDebugger()
    : m_pEngine(nullptr)
    , m_bSteppingEnabled(false)
    , m_bExecutionPaused(false)
    , m_currentLine(0)
{
}

ASDebugger::~ASDebugger()
{
    Shutdown();
}

bool ASDebugger::Initialize(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        LogError("ASDebugger::Initialize - Invalid engine pointer");
        return false;
    }
    
    m_pEngine = pEngine;
    
    // Set up line callback for debugging
    m_pEngine->SetEngineProperty(asEP_INCLUDE_JIT_INSTRUCTIONS, false);
    
    LogMessage("ASDebugger initialized successfully");
    return true;
}

void ASDebugger::Shutdown()
{
    if (m_pEngine)
    {
        // Clear all callbacks and data
        ClearAllBreakpoints();
        m_pEngine = nullptr;
    }
    
    m_bSteppingEnabled = false;
    m_bExecutionPaused = false;
    m_currentScript.clear();
    m_currentLine = 0;
    
    LogMessage("ASDebugger shutdown complete");
}

//-----------------------------------------------------------------------------
// Breakpoint Management
//-----------------------------------------------------------------------------

bool ASDebugger::AddBreakpoint(const std::string& scriptName, int lineNumber)
{
    // Check if breakpoint already exists
    for (const auto& bp : m_breakpoints)
    {
        if (bp.scriptName == scriptName && bp.lineNumber == lineNumber)
        {
            LogWarning("Breakpoint already exists at " + scriptName + ":" + std::to_string(lineNumber));
            return false;
        }
    }
    
    m_breakpoints.push_back(ASBreakpoint(scriptName, lineNumber));
    LogMessage("Added breakpoint at " + scriptName + ":" + std::to_string(lineNumber));
    return true;
}

bool ASDebugger::RemoveBreakpoint(const std::string& scriptName, int lineNumber)
{
    auto it = std::find_if(m_breakpoints.begin(), m_breakpoints.end(),
        [&](const ASBreakpoint& bp) {
            return bp.scriptName == scriptName && bp.lineNumber == lineNumber;
        });
    
    if (it != m_breakpoints.end())
    {
        m_breakpoints.erase(it);
        LogMessage("Removed breakpoint at " + scriptName + ":" + std::to_string(lineNumber));
        return true;
    }
    
    LogWarning("Breakpoint not found at " + scriptName + ":" + std::to_string(lineNumber));
    return false;
}

bool ASDebugger::EnableBreakpoint(const std::string& scriptName, int lineNumber, bool enable)
{
    for (auto& bp : m_breakpoints)
    {
        if (bp.scriptName == scriptName && bp.lineNumber == lineNumber)
        {
            bp.enabled = enable;
            LogMessage((enable ? "Enabled" : "Disabled") + std::string(" breakpoint at ") + 
                      scriptName + ":" + std::to_string(lineNumber));
            return true;
        }
    }
    
    LogWarning("Breakpoint not found at " + scriptName + ":" + std::to_string(lineNumber));
    return false;
}

void ASDebugger::ClearAllBreakpoints()
{
    size_t count = m_breakpoints.size();
    m_breakpoints.clear();
    LogMessage("Cleared " + std::to_string(count) + " breakpoints");
}

std::vector<ASBreakpoint> ASDebugger::GetBreakpoints() const
{
    return m_breakpoints;
}

bool ASDebugger::HasBreakpoint(const std::string& scriptName, int lineNumber) const
{
    for (const auto& bp : m_breakpoints)
    {
        if (bp.scriptName == scriptName && bp.lineNumber == lineNumber && bp.enabled)
        {
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
// Execution Control
//-----------------------------------------------------------------------------

void ASDebugger::EnableStepping(bool enable)
{
    m_bSteppingEnabled = enable;
    LogMessage(enable ? "Script stepping enabled" : "Script stepping disabled");
}

bool ASDebugger::IsSteppingEnabled() const
{
    return m_bSteppingEnabled;
}

void ASDebugger::StepInto()
{
    m_bSteppingEnabled = true;
    m_bExecutionPaused = false;
    LogMessage("Step into");
}

void ASDebugger::StepOver()
{
    m_bSteppingEnabled = true;
    m_bExecutionPaused = false;
    LogMessage("Step over");
}

void ASDebugger::StepOut()
{
    m_bSteppingEnabled = true;
    m_bExecutionPaused = false;
    LogMessage("Step out");
}

void ASDebugger::Continue()
{
    m_bSteppingEnabled = false;
    m_bExecutionPaused = false;
    LogMessage("Continue execution");
}

//-----------------------------------------------------------------------------
// Variable Inspection
//-----------------------------------------------------------------------------

std::vector<ASVariableInfo> ASDebugger::GetLocalVariables(asIScriptContext* pContext)
{
    std::vector<ASVariableInfo> variables;
    
    if (!pContext)
    {
        LogError("GetLocalVariables - Invalid context");
        return variables;
    }
    
    asIScriptFunction* pFunc = pContext->GetFunction();
    if (!pFunc)
    {
        return variables;
    }
    
    // Get local variable count from context
    int varCount = pContext->GetVarCount();
    
    for (int i = 0; i < varCount; i++)
    {
        ASVariableInfo varInfo;
        
        const char* varName = pContext->GetVarName(i);
        if (varName)
        {
            varInfo.name = varName;
        }
        
        int typeId = pContext->GetVarTypeId(i);
        const char* typeName = m_pEngine->GetTypeDeclaration(typeId);
        if (typeName)
        {
            varInfo.type = typeName;
        }
        
        // Get variable value (simplified for basic types)
        void* pVarPtr = pContext->GetAddressOfVar(i);
        if (pVarPtr)
        {
            varInfo.value = FormatVariableValue(pVarPtr, typeId);
        }
        else
        {
            varInfo.value = "<unable to read>";
        }
        
        variables.push_back(varInfo);
    }
    
    return variables;
}

std::vector<ASVariableInfo> ASDebugger::GetGlobalVariables()
{
    std::vector<ASVariableInfo> variables;
    
    if (!m_pEngine)
    {
        LogError("GetGlobalVariables - Engine not initialized");
        return variables;
    }
    
    // Get global variable count
    int globalCount = m_pEngine->GetGlobalPropertyCount();
    
    for (int i = 0; i < globalCount; i++)
    {
        ASVariableInfo varInfo;
        
        const char* varName = nullptr;
        int typeId = 0;
        bool isConst = false;
        
        if (m_pEngine->GetGlobalPropertyByIndex(i, &varName, nullptr, &typeId, &isConst) >= 0)
        {
            if (varName)
            {
                varInfo.name = varName;
            }
            
            const char* typeName = m_pEngine->GetTypeDeclaration(typeId);
            if (typeName)
            {
                varInfo.type = typeName;
            }
            
            // Get global variable value (simplified - would need proper global property access)
            varInfo.value = "<global variable>";
            
            variables.push_back(varInfo);
        }
    }
    
    return variables;
}

ASVariableInfo ASDebugger::GetVariableInfo(const std::string& varName, asIScriptContext* pContext)
{
    ASVariableInfo varInfo;
    varInfo.name = varName;
    
    if (!pContext)
    {
        varInfo.value = "<invalid context>";
        return varInfo;
    }
    
    // Try to find in local variables first
    int varCount = pContext->GetVarCount();
    for (int i = 0; i < varCount; i++)
    {
        const char* localVarName = pContext->GetVarName(i);
        if (localVarName && varName == localVarName)
        {
            int typeId = pContext->GetVarTypeId(i);
            const char* typeName = m_pEngine->GetTypeDeclaration(typeId);
            if (typeName)
            {
                varInfo.type = typeName;
            }
            
            void* pVarPtr = pContext->GetAddressOfVar(i);
            if (pVarPtr)
            {
                varInfo.value = FormatVariableValue(pVarPtr, typeId);
            }
            
            return varInfo;
        }
    }
    
    // Try global variables
    int globalIndex = m_pEngine->GetGlobalPropertyIndexByName(varName.c_str());
    if (globalIndex >= 0)
    {
        const char* globalVarName = nullptr;
        int typeId = 0;
        bool isConst = false;
        
        if (m_pEngine->GetGlobalPropertyByIndex(globalIndex, &globalVarName, nullptr, &typeId, &isConst) >= 0)
        {
            const char* typeName = m_pEngine->GetTypeDeclaration(typeId);
            if (typeName)
            {
                varInfo.type = typeName;
            }
            
            // Simplified global variable access
            varInfo.value = "<global variable>";
            
            return varInfo;
        }
    }
    
    varInfo.value = "<variable not found>";
    return varInfo;
}

bool ASDebugger::SetVariableValue(const std::string& varName, const std::string& value, asIScriptContext* pContext)
{
    LogMessage("SetVariableValue: " + varName + " = " + value);
    // Simplified implementation - would need type-specific parsing
    LogWarning("SetVariableValue not fully implemented yet");
    return false;
}

//-----------------------------------------------------------------------------
// Call Stack
//-----------------------------------------------------------------------------

std::vector<ASCallFrame> ASDebugger::GetCallStack(asIScriptContext* pContext)
{
    std::vector<ASCallFrame> callStack;
    
    if (!pContext)
    {
        LogError("GetCallStack - Invalid context");
        return callStack;
    }
    
    int stackSize = pContext->GetCallstackSize();
    
    for (int i = 0; i < stackSize; i++)
    {
        ASCallFrame frame;
        
        asIScriptFunction* pFunc = pContext->GetFunction(i);
        if (pFunc)
        {
            frame.functionName = pFunc->GetName();
            
            // Get script section name (approximates script name)
            const char* sectionName = pFunc->GetScriptSectionName();
            if (sectionName)
            {
                frame.scriptName = sectionName;
            }
            
            // Get line number
            frame.lineNumber = pContext->GetLineNumber(i, &frame.columnNumber);
        }
        
        callStack.push_back(frame);
    }
    
    return callStack;
}

//-----------------------------------------------------------------------------
// Script State
//-----------------------------------------------------------------------------

bool ASDebugger::IsExecutionPaused() const
{
    return m_bExecutionPaused;
}

std::string ASDebugger::GetCurrentScript() const
{
    return m_currentScript;
}

int ASDebugger::GetCurrentLine() const
{
    return m_currentLine;
}

//-----------------------------------------------------------------------------
// Debug Output
//-----------------------------------------------------------------------------

void ASDebugger::LogMessage(const std::string& message)
{
    std::cout << "[ASDebugger] " << message << std::endl;
}

void ASDebugger::LogWarning(const std::string& warning)
{
    std::cout << "[ASDebugger WARNING] " << warning << std::endl;
}

void ASDebugger::LogError(const std::string& error)
{
    std::cerr << "[ASDebugger ERROR] " << error << std::endl;
}

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

void ASDebugger::LineCallback(asIScriptContext* pContext, void* pUserData)
{
    ASDebugger* pDebugger = static_cast<ASDebugger*>(pUserData);
    if (!pDebugger || !pContext)
        return;
    
    asIScriptFunction* pFunc = pContext->GetFunction();
    if (!pFunc)
        return;
    
    const char* sectionName = pFunc->GetScriptSectionName();
    std::string scriptName = sectionName ? sectionName : "unknown";
    
    int lineNumber = pContext->GetLineNumber();
    
    pDebugger->m_currentScript = scriptName;
    pDebugger->m_currentLine = lineNumber;
    
    // Check for breakpoints
    if (pDebugger->HasBreakpoint(scriptName, lineNumber))
    {
        pDebugger->HandleBreakpoint(pContext);
    }
    
    // Handle stepping
    if (pDebugger->m_bSteppingEnabled)
    {
        pDebugger->m_bExecutionPaused = true;
        // In a real implementation, this would pause execution
        // For now, just log the step
        pDebugger->LogMessage("Step: " + scriptName + ":" + std::to_string(lineNumber));
    }
}

void ASDebugger::ExceptionCallback(asIScriptContext* pContext, void* pUserData)
{
    ASDebugger* pDebugger = static_cast<ASDebugger*>(pUserData);
    if (!pDebugger || !pContext)
        return;
    
    asIScriptFunction* pFunc = pContext->GetFunction();
    const char* sectionName = pFunc ? pFunc->GetScriptSectionName() : "unknown";
    int lineNumber = pContext->GetLineNumber();
    
    std::string errorMsg = "Script exception at ";
    errorMsg += sectionName ? sectionName : "unknown";
    errorMsg += ":" + std::to_string(lineNumber);
    
    pDebugger->LogError(errorMsg);
    pDebugger->m_bExecutionPaused = true;
}

//-----------------------------------------------------------------------------
// Private Helpers
//-----------------------------------------------------------------------------

bool ASDebugger::CheckBreakpoint(const std::string& scriptName, int lineNumber)
{
    for (auto& bp : m_breakpoints)
    {
        if (bp.scriptName == scriptName && bp.lineNumber == lineNumber && bp.enabled)
        {
            bp.hitCount++;
            return true;
        }
    }
    return false;
}

void ASDebugger::HandleBreakpoint(asIScriptContext* pContext)
{
    asIScriptFunction* pFunc = pContext->GetFunction();
    const char* sectionName = pFunc ? pFunc->GetScriptSectionName() : "unknown";
    int lineNumber = pContext->GetLineNumber();
    
    LogMessage("Breakpoint hit at " + std::string(sectionName) + ":" + std::to_string(lineNumber));
    m_bExecutionPaused = true;
    
    // In a real implementation, this would pause script execution
    // and wait for user input to continue
}

std::string ASDebugger::FormatVariableValue(void* pValue, int typeId)
{
    if (!pValue || !m_pEngine)
        return "<null>";
    
    // Get type info
    asITypeInfo* pTypeInfo = m_pEngine->GetTypeInfoById(typeId);
    
    // Handle basic types
    if (typeId == asTYPEID_BOOL)
    {
        return *static_cast<bool*>(pValue) ? "true" : "false";
    }
    else if (typeId == asTYPEID_INT8)
    {
        return std::to_string(*static_cast<char*>(pValue));
    }
    else if (typeId == asTYPEID_INT16)
    {
        return std::to_string(*static_cast<short*>(pValue));
    }
    else if (typeId == asTYPEID_INT32)
    {
        return std::to_string(*static_cast<int*>(pValue));
    }
    else if (typeId == asTYPEID_FLOAT)
    {
        return std::to_string(*static_cast<float*>(pValue));
    }
    else if (typeId == asTYPEID_DOUBLE)
    {
        return std::to_string(*static_cast<double*>(pValue));
    }
    else if (typeId & asTYPEID_OBJHANDLE)
    {
        return "<object handle>";
    }
    else if (pTypeInfo)
    {
        return "<" + std::string(pTypeInfo->GetName()) + ">";
    }
    
    return "<unknown type>";
}

//-----------------------------------------------------------------------------
// ASProfiler Implementation
//-----------------------------------------------------------------------------

ASProfiler::ASProfiler()
    : m_pEngine(nullptr)
    , m_bProfilingActive(false)
    , m_currentMemoryUsage(0)
    , m_peakMemoryUsage(0)
{
}

ASProfiler::~ASProfiler()
{
    Shutdown();
}

bool ASProfiler::Initialize(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        std::cerr << "[ASProfiler ERROR] Invalid engine pointer" << std::endl;
        return false;
    }
    
    m_pEngine = pEngine;
    Reset();
    
    std::cout << "[ASProfiler] Profiler initialized successfully" << std::endl;
    return true;
}

void ASProfiler::Shutdown()
{
    StopProfiling();
    m_pEngine = nullptr;
    m_profileData.clear();
    m_functionStartTimes.clear();
    
    std::cout << "[ASProfiler] Profiler shutdown complete" << std::endl;
}

void ASProfiler::StartProfiling()
{
    m_bProfilingActive = true;
    Reset();
    std::cout << "[ASProfiler] Profiling started" << std::endl;
}

void ASProfiler::StopProfiling()
{
    m_bProfilingActive = false;
    std::cout << "[ASProfiler] Profiling stopped" << std::endl;
}

bool ASProfiler::IsProfilingActive() const
{
    return m_bProfilingActive;
}

void ASProfiler::Reset()
{
    m_profileData.clear();
    m_functionStartTimes.clear();
    m_currentMemoryUsage = 0;
    m_peakMemoryUsage = 0;
    std::cout << "[ASProfiler] Profiling data reset" << std::endl;
}

void ASProfiler::RecordFunctionStart(const std::string& functionName, const std::string& scriptName)
{
    if (!m_bProfilingActive)
        return;
    
    std::string fullName = scriptName + "::" + functionName;
    m_functionStartTimes[fullName] = GetProfilerTime();
}

void ASProfiler::RecordFunctionEnd(const std::string& functionName, const std::string& scriptName, double duration)
{
    if (!m_bProfilingActive)
        return;
    
    std::string fullName = scriptName + "::" + functionName;
    
    // Calculate duration if not provided
    if (duration <= 0.0)
    {
        auto it = m_functionStartTimes.find(fullName);
        if (it != m_functionStartTimes.end())
        {
            duration = GetProfilerTime() - it->second;
            m_functionStartTimes.erase(it);
        }
        else
        {
            return; // No start time recorded
        }
    }
    
    // Update profile data
    ASProfilerEntry& entry = m_profileData[fullName];
    entry.functionName = functionName;
    entry.scriptName = scriptName;
    entry.callCount++;
    entry.totalTime += duration;
    entry.averageTime = entry.totalTime / entry.callCount;
    
    if (entry.callCount == 1 || duration < entry.minTime)
    {
        entry.minTime = duration;
    }
    
    if (entry.callCount == 1 || duration > entry.maxTime)
    {
        entry.maxTime = duration;
    }
}

std::vector<ASProfilerEntry> ASProfiler::GetProfileResults() const
{
    std::vector<ASProfilerEntry> results;
    
    for (const auto& pair : m_profileData)
    {
        results.push_back(pair.second);
    }
    
    // Sort by total time (descending)
    std::sort(results.begin(), results.end(),
        [](const ASProfilerEntry& a, const ASProfilerEntry& b) {
            return a.totalTime > b.totalTime;
        });
    
    return results;
}

ASProfilerEntry ASProfiler::GetFunctionProfile(const std::string& functionName) const
{
    for (const auto& pair : m_profileData)
    {
        if (pair.second.functionName == functionName)
        {
            return pair.second;
        }
    }
    
    return ASProfilerEntry(); // Empty entry if not found
}

void ASProfiler::RecordMemoryUsage(size_t bytesUsed)
{
    m_currentMemoryUsage = bytesUsed;
    if (bytesUsed > m_peakMemoryUsage)
    {
        m_peakMemoryUsage = bytesUsed;
    }
}

size_t ASProfiler::GetPeakMemoryUsage() const
{
    return m_peakMemoryUsage;
}

size_t ASProfiler::GetCurrentMemoryUsage() const
{
    return m_currentMemoryUsage;
}

void ASProfiler::GenerateReport(const std::string& filename) const
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "[ASProfiler ERROR] Cannot open file for report: " << filename << std::endl;
        return;
    }
    
    file << "AngelScript Profiling Report\n";
    file << "============================\n\n";
    
    file << "Memory Usage:\n";
    file << "  Current: " << m_currentMemoryUsage << " bytes\n";
    file << "  Peak: " << m_peakMemoryUsage << " bytes\n\n";
    
    file << "Function Performance:\n";
    file << "Function Name                Script Name              Calls    Total Time  Avg Time    Min Time    Max Time\n";
    file << "------------------------------------------------------------------------------------------------------------\n";
    
    auto results = GetProfileResults();
    for (const auto& entry : results)
    {
        file << std::left << std::setw(28) << entry.functionName
             << std::setw(24) << entry.scriptName
             << std::setw(8) << entry.callCount
             << std::setw(12) << entry.totalTime
             << std::setw(12) << entry.averageTime
             << std::setw(12) << entry.minTime
             << std::setw(12) << entry.maxTime << "\n";
    }
    
    file.close();
    std::cout << "[ASProfiler] Report generated: " << filename << std::endl;
}

void ASProfiler::LogSummary() const
{
    std::cout << "\n[ASProfiler] Performance Summary:" << std::endl;
    std::cout << "  Functions profiled: " << m_profileData.size() << std::endl;
    std::cout << "  Peak memory usage: " << m_peakMemoryUsage << " bytes" << std::endl;
    
    auto results = GetProfileResults();
    if (!results.empty())
    {
        std::cout << "  Top function by time: " << results[0].functionName 
                  << " (" << results[0].totalTime << "s)" << std::endl;
    }
}

double ASProfiler::GetProfilerTime() const
{
#ifdef _WIN32
    static LARGE_INTEGER frequency;
    static bool initialized = false;
    if (!initialized)
    {
        QueryPerformanceFrequency(&frequency);
        initialized = true;
    }
    
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return static_cast<double>(counter.QuadPart) / frequency.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
#endif
}

void ASProfiler::FunctionStartCallback(asIScriptContext* pContext, void* pUserData)
{
    ASProfiler* pProfiler = static_cast<ASProfiler*>(pUserData);
    if (!pProfiler || !pContext)
        return;
    
    asIScriptFunction* pFunc = pContext->GetFunction();
    if (pFunc)
    {
        const char* funcName = pFunc->GetName();
        const char* sectionName = pFunc->GetScriptSectionName();
        
        if (funcName && sectionName)
        {
            pProfiler->RecordFunctionStart(funcName, sectionName);
        }
    }
}

void ASProfiler::FunctionEndCallback(asIScriptContext* pContext, void* pUserData)
{
    ASProfiler* pProfiler = static_cast<ASProfiler*>(pUserData);
    if (!pProfiler || !pContext)
        return;
    
    asIScriptFunction* pFunc = pContext->GetFunction();
    if (pFunc)
    {
        const char* funcName = pFunc->GetName();
        const char* sectionName = pFunc->GetScriptSectionName();
        
        if (funcName && sectionName)
        {
            pProfiler->RecordFunctionEnd(funcName, sectionName, 0.0); // Duration calculated internally
        }
    }
}