//==========================================================================
// ASCoroutines.cpp - AngelScript Coroutine System Implementation
// Master Sword Rebirth AngelScript Implementation
//==========================================================================

#include "ASCoroutines.h"
#include "CAngelScriptManager.h"
#include <angelscript.h>
#include <ctime>
#include <algorithm>

//==========================================================================
// ASCoroutine Implementation
//==========================================================================

ASCoroutine::ASCoroutine(int nId, asIScriptFunction* pFunction, const char* szFunctionName)
    : m_pContext(nullptr)
    , m_pFunction(pFunction)
    , m_sFunctionName(szFunctionName ? szFunctionName : "")
    , m_eState(ASCoroutineState::RUNNING)
    , m_fResumeTime(0.0f)
    , m_fStartTime(0.0f)
    , m_nCoroutineId(nId)
    , m_nExecutionCount(0)
    , m_fTotalRunTime(0.0f)
{
    if (m_pFunction)
    {
        m_pFunction->AddRef();
    }
    
    m_fStartTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;
}

ASCoroutine::~ASCoroutine()
{
    Cancel();
    
    if (m_pFunction)
    {
        m_pFunction->Release();
        m_pFunction = nullptr;
    }
}

bool ASCoroutine::Initialize()
{
    if (!m_pFunction)
    {
        SetError("No function provided for coroutine");
        return false;
    }
    
    // Acquire context from the manager
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (!pManager)
    {
        SetError("AngelScript manager not available");
        return false;
    }
    
    m_pContext = pManager->AcquireContext();
    if (!m_pContext)
    {
        SetError("Failed to acquire script context");
        return false;
    }
    
    // Prepare the context with our function
    int result = m_pContext->Prepare(m_pFunction);
    if (result < 0)
    {
        SetError("Failed to prepare script context");
        return false;
    }
    
    return true;
}

bool ASCoroutine::Execute()
{
    if (!m_pContext || m_eState != ASCoroutineState::RUNNING)
    {
        return false;
    }
    
    float fStartTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;
    
    // Execute the script
    int result = m_pContext->Execute();
    
    float fEndTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;
    UpdateExecutionStats(fStartTime, fEndTime);
    
    switch (result)
    {
        case asEXECUTION_FINISHED:
            m_eState = ASCoroutineState::COMPLETED;
            break;
            
        case asEXECUTION_SUSPENDED:
            m_eState = ASCoroutineState::SUSPENDED;
            break;
            
        case asEXECUTION_ABORTED:
            m_eState = ASCoroutineState::CANCELLED;
            SetError("Script execution was aborted");
            break;
            
        case asEXECUTION_EXCEPTION:
            m_eState = ASCoroutineState::ERROR;
            SetError("Script execution encountered an exception");
            break;
            
        default:
            m_eState = ASCoroutineState::ERROR;
            SetError("Unknown script execution result");
            break;
    }
    
    return result >= 0;
}

bool ASCoroutine::Resume()
{
    if (!m_pContext || m_eState != ASCoroutineState::SUSPENDED)
    {
        return false;
    }
    
    m_eState = ASCoroutineState::RUNNING;
    return Execute();
}

void ASCoroutine::Suspend(float fDelay)
{
    if (m_eState == ASCoroutineState::RUNNING)
    {
        m_eState = ASCoroutineState::SUSPENDED;
        m_fResumeTime = (static_cast<float>(clock()) / CLOCKS_PER_SEC) + fDelay;
    }
}

void ASCoroutine::Cancel()
{
    if (m_eState != ASCoroutineState::COMPLETED && m_eState != ASCoroutineState::CANCELLED)
    {
        m_eState = ASCoroutineState::CANCELLED;
        
        if (m_pContext)
        {
            m_pContext->Abort();
            
            // Release context back to manager
            CAngelScriptManager* pManager = CAngelScriptManager::Instance();
            if (pManager)
            {
                pManager->ReleaseContext(m_pContext);
            }
            m_pContext = nullptr;
        }
    }
}

bool ASCoroutine::ShouldResume(float fCurrentTime) const
{
    return m_eState == ASCoroutineState::SUSPENDED && fCurrentTime >= m_fResumeTime;
}

void ASCoroutine::SetError(const char* szError)
{
    m_sLastError = szError ? szError : "Unknown error";
    m_eState = ASCoroutineState::ERROR;
}

void ASCoroutine::UpdateExecutionStats(float fStartTime, float fEndTime)
{
    m_nExecutionCount++;
    m_fTotalRunTime += (fEndTime - fStartTime);
}

//==========================================================================
// ASCoroutineManager Implementation
//==========================================================================

ASCoroutineManager* ASCoroutineManager::s_pInstance = nullptr;

ASCoroutineManager::ASCoroutineManager()
    : m_nNextCoroutineId(1)
    , m_nActiveCoroutines(0)
    , m_nTotalCoroutinesCreated(0)
    , m_fTotalExecutionTime(0.0f)
    , m_nMaxConcurrentCoroutines(32)
    , m_fMaxExecutionTimePerFrame(0.016f) // ~16ms max per frame (60 FPS target)
{
}

ASCoroutineManager::~ASCoroutineManager()
{
    StopAllCoroutines();
}

ASCoroutineManager* ASCoroutineManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new ASCoroutineManager();
    }
    return s_pInstance;
}

void ASCoroutineManager::Shutdown()
{
    if (s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

int ASCoroutineManager::StartCoroutine(const char* szFunctionName)
{
    if (!szFunctionName)
    {
        return -1;
    }
    
    asIScriptFunction* pFunction = FindGlobalFunction(szFunctionName);
    if (!pFunction)
    {
        return -1;
    }
    
    return StartCoroutine(pFunction, szFunctionName);
}

int ASCoroutineManager::StartCoroutine(asIScriptFunction* pFunction, const char* szFunctionName)
{
    if (!pFunction)
    {
        return -1;
    }
    
    // Check concurrent limit
    if (m_nActiveCoroutines >= m_nMaxConcurrentCoroutines)
    {
        return -1;
    }
    
    // Create new coroutine
    int nCoroutineId = m_nNextCoroutineId++;
    auto pCoroutine = std::make_unique<ASCoroutine>(nCoroutineId, pFunction, szFunctionName);
    
    if (!pCoroutine->Initialize())
    {
        return -1;
    }
    
    m_Coroutines.push_back(std::move(pCoroutine));
    m_nActiveCoroutines++;
    m_nTotalCoroutinesCreated++;
    
    return nCoroutineId;
}

bool ASCoroutineManager::StopCoroutine(int nCoroutineId)
{
    ASCoroutine* pCoroutine = FindCoroutine(nCoroutineId);
    if (!pCoroutine)
    {
        return false;
    }
    
    pCoroutine->Cancel();
    return true;
}

void ASCoroutineManager::StopAllCoroutines()
{
    for (auto& pCoroutine : m_Coroutines)
    {
        if (pCoroutine && !pCoroutine->IsCompleted())
        {
            pCoroutine->Cancel();
        }
    }
    
    m_Coroutines.clear();
    m_nActiveCoroutines = 0;
}

void ASCoroutineManager::Think(float fCurrentTime)
{
    if (m_Coroutines.empty())
    {
        return;
    }
    
    float fFrameStartTime = fCurrentTime;
    float fExecutionTimeUsed = 0.0f;
    
    // Process coroutines
    for (auto& pCoroutine : m_Coroutines)
    {
        if (!pCoroutine)
        {
            continue;
        }
        
        if (fExecutionTimeUsed >= m_fMaxExecutionTimePerFrame)
        {
            break; // Don't exceed frame time budget
        }
        
        float fCoroutineStartTime = GetCurrentTime();
        
        if (pCoroutine->IsRunning())
        {
            pCoroutine->Execute();
        }
        else if (pCoroutine->IsSuspended() && pCoroutine->ShouldResume(fCurrentTime))
        {
            pCoroutine->Resume();
        }
        
        float fCoroutineEndTime = GetCurrentTime();
        fExecutionTimeUsed += (fCoroutineEndTime - fCoroutineStartTime);
    }
    
    // Clean up completed coroutines
    CleanupCompletedCoroutines();
    
    // Update performance stats
    UpdatePerformanceStats();
}

bool ASCoroutineManager::SuspendCoroutine(int nCoroutineId, float fDelay)
{
    ASCoroutine* pCoroutine = FindCoroutine(nCoroutineId);
    if (!pCoroutine)
    {
        return false;
    }
    
    pCoroutine->Suspend(fDelay);
    return true;
}

bool ASCoroutineManager::ResumeCoroutine(int nCoroutineId)
{
    ASCoroutine* pCoroutine = FindCoroutine(nCoroutineId);
    if (!pCoroutine)
    {
        return false;
    }
    
    return pCoroutine->Resume();
}

bool ASCoroutineManager::IsCoroutineRunning(int nCoroutineId) const
{
    for (const auto& pCoroutine : m_Coroutines)
    {
        if (pCoroutine && pCoroutine->GetId() == nCoroutineId)
        {
            return pCoroutine->IsRunning() || pCoroutine->IsSuspended();
        }
    }
    return false;
}

void ASCoroutineManager::Delay(float fSeconds)
{
    // This would be called from within a script to suspend the current coroutine
    // Implementation would need to identify the current coroutine context
    // For now, this is a placeholder for the concept
}

void ASCoroutineManager::Yield()
{
    Delay(0.0f); // Yield to next frame
}

void ASCoroutineManager::CleanupCompletedCoroutines()
{
    auto it = std::remove_if(m_Coroutines.begin(), m_Coroutines.end(),
        [](const std::unique_ptr<ASCoroutine>& pCoroutine) {
            return pCoroutine && pCoroutine->IsCompleted();
        });
    
    int nRemoved = std::distance(it, m_Coroutines.end());
    m_Coroutines.erase(it, m_Coroutines.end());
    m_nActiveCoroutines -= nRemoved;
}

ASCoroutine* ASCoroutineManager::FindCoroutine(int nCoroutineId)
{
    for (auto& pCoroutine : m_Coroutines)
    {
        if (pCoroutine && pCoroutine->GetId() == nCoroutineId)
        {
            return pCoroutine.get();
        }
    }
    return nullptr;
}

void ASCoroutineManager::RemoveCoroutine(int nCoroutineId)
{
    auto it = std::find_if(m_Coroutines.begin(), m_Coroutines.end(),
        [nCoroutineId](const std::unique_ptr<ASCoroutine>& pCoroutine) {
            return pCoroutine && pCoroutine->GetId() == nCoroutineId;
        });
    
    if (it != m_Coroutines.end())
    {
        m_Coroutines.erase(it);
        m_nActiveCoroutines--;
    }
}

float ASCoroutineManager::GetCurrentTime() const
{
    return static_cast<float>(clock()) / CLOCKS_PER_SEC;
}

void ASCoroutineManager::UpdatePerformanceStats()
{
    // Calculate total execution time for this frame
    float fTotalTime = 0.0f;
    for (const auto& pCoroutine : m_Coroutines)
    {
        if (pCoroutine)
        {
            fTotalTime += pCoroutine->GetRunTime();
        }
    }
    m_fTotalExecutionTime = fTotalTime;
}

asIScriptFunction* ASCoroutineManager::FindGlobalFunction(const char* szFunctionName)
{
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (!pManager || !pManager->IsInitialized())
    {
        return nullptr;
    }
    
    asIScriptEngine* pEngine = pManager->GetEngine();
    if (!pEngine)
    {
        return nullptr;
    }
    
    // Look for the function in all modules
    asUINT nModuleCount = pEngine->GetModuleCount();
    for (asUINT i = 0; i < nModuleCount; i++)
    {
        asIScriptModule* pModule = pEngine->GetModuleByIndex(i);
        if (!pModule)
        {
            continue;
        }
        
        asIScriptFunction* pFunction = pModule->GetFunctionByName(szFunctionName);
        if (pFunction)
        {
            return pFunction;
        }
    }
    
    return nullptr;
}

//==========================================================================
// Global Script Functions
//==========================================================================

void RegisterCoroutineFunctions(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        return;
    }
    
    // Register coroutine management functions
    pEngine->RegisterGlobalFunction("int StartCoroutine(const string &in)", 
        asFUNCTION(StartCoroutine), asCALL_CDECL);
    pEngine->RegisterGlobalFunction("void StopCoroutine(int)", 
        asFUNCTION(StopCoroutine), asCALL_CDECL);
    pEngine->RegisterGlobalFunction("void DelaySeconds(float)", 
        asFUNCTION(DelaySeconds), asCALL_CDECL);
    pEngine->RegisterGlobalFunction("void YieldFrame()", 
        asFUNCTION(YieldFrame), asCALL_CDECL);
    pEngine->RegisterGlobalFunction("bool IsCoroutineRunning(int)", 
        asFUNCTION(IsCoroutineRunning), asCALL_CDECL);
}

int StartCoroutine(const std::string& sFunctionName)
{
    ASCoroutineManager* pManager = ASCoroutineManager::Instance();
    if (!pManager)
    {
        return -1;
    }
    
    return pManager->StartCoroutine(sFunctionName.c_str());
}

void StopCoroutine(int nCoroutineId)
{
    ASCoroutineManager* pManager = ASCoroutineManager::Instance();
    if (pManager)
    {
        pManager->StopCoroutine(nCoroutineId);
    }
}

void DelaySeconds(float fSeconds)
{
    ASCoroutineManager* pManager = ASCoroutineManager::Instance();
    if (pManager)
    {
        pManager->Delay(fSeconds);
    }
}

void YieldFrame()
{
    ASCoroutineManager* pManager = ASCoroutineManager::Instance();
    if (pManager)
    {
        pManager->Yield();
    }
}

bool IsCoroutineRunning(int nCoroutineId)
{
    ASCoroutineManager* pManager = ASCoroutineManager::Instance();
    if (!pManager)
    {
        return false;
    }
    
    return pManager->IsCoroutineRunning(nCoroutineId);
}