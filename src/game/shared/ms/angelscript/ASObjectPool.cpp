//==========================================================================
// ASObjectPool.cpp - AngelScript Object Pooling and Memory Optimization Implementation
// Master Sword Rebirth AngelScript Implementation
//==========================================================================

#include "ASObjectPool.h"
#include "CAngelScriptManager.h"
#include <angelscript.h>
#include <ctime>
#include <algorithm>
#include <cstdio>
#include <functional>

//==========================================================================
// ASTypePool Implementation
//==========================================================================

ASTypePool::ASTypePool(asITypeInfo* pTypeInfo, const char* szTypeName)
    : m_pTypeInfo(pTypeInfo)
    , m_sTypeName(szTypeName ? szTypeName : "")
    , m_nMaxObjects(16)
    , m_nMinObjects(2)
    , m_fObjectLifetime(60.0f) // 60 seconds default
    , m_nTotalCreated(0)
    , m_nTotalReused(0)
    , m_nActiveObjects(0)
{
    if (m_pTypeInfo)
    {
        m_pTypeInfo->AddRef();
    }
}

ASTypePool::~ASTypePool()
{
    ClearAll();
    
    if (m_pTypeInfo)
    {
        m_pTypeInfo->Release();
    }
}

asIScriptObject* ASTypePool::AcquireObject()
{
    if (!m_pTypeInfo)
    {
        return nullptr;
    }
    
    // Try to find an available object
    ASPooledObject* pPooledObj = FindAvailableObject();
    if (pPooledObj)
    {
        pPooledObj->bInUse = true;
        pPooledObj->fLastUsed = GetCurrentTime();
        pPooledObj->nUsageCount++;
        m_nActiveObjects++;
        m_nTotalReused++;
        
        return pPooledObj->pObject;
    }
    
    // Create new object if we haven't reached the limit
    if (static_cast<int>(m_Objects.size()) < m_nMaxObjects)
    {
        asIScriptObject* pNewObject = CreateNewObject();
        if (pNewObject)
        {
            ASPooledObject pooledObj;
            pooledObj.pObject = pNewObject;
            pooledObj.pTypeInfo = m_pTypeInfo;
            pooledObj.bInUse = true;
            pooledObj.fLastUsed = GetCurrentTime();
            pooledObj.nUsageCount = 1;
            
            m_Objects.push_back(pooledObj);
            m_nActiveObjects++;
            m_nTotalCreated++;
            
            return pNewObject;
        }
    }
    
    // Pool is full, remove oldest unused object and create new one
    RemoveOldestUnusedObject();
    return AcquireObject(); // Recursive call should succeed now
}

void ASTypePool::ReleaseObject(asIScriptObject* pObject)
{
    if (!pObject)
    {
        return;
    }
    
    // Find the object in our pool
    for (auto& pooledObj : m_Objects)
    {
        if (pooledObj.pObject == pObject)
        {
            if (pooledObj.bInUse)
            {
                pooledObj.bInUse = false;
                pooledObj.fLastUsed = GetCurrentTime();
                m_nActiveObjects--;
            }
            return;
        }
    }
    
    // Object not found in pool - this shouldn't happen
    // But if it does, just release it directly
    pObject->Release();
}

void ASTypePool::CleanupUnusedObjects(float fCurrentTime)
{
    auto it = m_Objects.begin();
    while (it != m_Objects.end())
    {
        if (!it->bInUse && (fCurrentTime - it->fLastUsed) > m_fObjectLifetime)
        {
            // Keep minimum objects in pool
            if (static_cast<int>(m_Objects.size()) <= m_nMinObjects)
            {
                ++it;
                continue;
            }
            
            if (it->pObject)
            {
                it->pObject->Release();
            }
            it = m_Objects.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ASTypePool::ClearAll()
{
    for (auto& pooledObj : m_Objects)
    {
        if (pooledObj.pObject)
        {
            pooledObj.pObject->Release();
        }
    }
    m_Objects.clear();
    m_nActiveObjects = 0;
}

size_t ASTypePool::GetMemoryUsage() const
{
    if (!m_pTypeInfo)
    {
        return 0;
    }
    
    // Estimate memory usage
    size_t nObjectSize = m_pTypeInfo->GetSize();
    return m_Objects.size() * (nObjectSize + sizeof(ASPooledObject));
}

asIScriptObject* ASTypePool::CreateNewObject()
{
    if (!m_pTypeInfo)
    {
        return nullptr;
    }
    
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
    
    // Create the object
    asIScriptObject* pObject = reinterpret_cast<asIScriptObject*>(pEngine->CreateScriptObject(m_pTypeInfo));
    return pObject;
}

ASPooledObject* ASTypePool::FindAvailableObject()
{
    for (auto& pooledObj : m_Objects)
    {
        if (!pooledObj.bInUse)
        {
            return &pooledObj;
        }
    }
    return nullptr;
}

void ASTypePool::RemoveOldestUnusedObject()
{
    float fOldestTime = GetCurrentTime();
    auto oldestIt = m_Objects.end();
    
    for (auto it = m_Objects.begin(); it != m_Objects.end(); ++it)
    {
        if (!it->bInUse && it->fLastUsed < fOldestTime)
        {
            fOldestTime = it->fLastUsed;
            oldestIt = it;
        }
    }
    
    if (oldestIt != m_Objects.end())
    {
        if (oldestIt->pObject)
        {
            oldestIt->pObject->Release();
        }
        m_Objects.erase(oldestIt);
    }
}

float ASTypePool::GetCurrentTime() const
{
    return static_cast<float>(clock()) / CLOCKS_PER_SEC;
}

//==========================================================================
// ASObjectPool Implementation
//==========================================================================

ASObjectPool* ASObjectPool::s_pInstance = nullptr;

ASObjectPool::ASObjectPool()
    : m_bEnabled(true)
    , m_nDefaultMaxObjects(16)
    , m_fDefaultObjectLifetime(60.0f)
    , m_nTotalMemoryUsed(0)
    , m_nPeakMemoryUsed(0)
    , m_nTotalObjectsCreated(0)
    , m_nTotalObjectsReused(0)
{
}

ASObjectPool::~ASObjectPool()
{
    ClearAllPools();
}

ASObjectPool* ASObjectPool::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new ASObjectPool();
    }
    return s_pInstance;
}

void ASObjectPool::Shutdown()
{
    if (s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

asIScriptObject* ASObjectPool::AcquireObject(const char* szTypeName)
{
    if (!m_bEnabled || !szTypeName)
    {
        return nullptr;
    }
    
    ASTypePool* pPool = GetOrCreateTypePool(szTypeName);
    if (!pPool)
    {
        return nullptr;
    }
    
    asIScriptObject* pObject = pPool->AcquireObject();
    if (pObject)
    {
        UpdateMemoryStats();
    }
    
    return pObject;
}

asIScriptObject* ASObjectPool::AcquireObject(asITypeInfo* pTypeInfo)
{
    if (!m_bEnabled || !pTypeInfo)
    {
        return nullptr;
    }
    
    ASTypePool* pPool = GetOrCreateTypePool(pTypeInfo);
    if (!pPool)
    {
        return nullptr;
    }
    
    asIScriptObject* pObject = pPool->AcquireObject();
    if (pObject)
    {
        UpdateMemoryStats();
    }
    
    return pObject;
}

void ASObjectPool::ReleaseObject(asIScriptObject* pObject)
{
    if (!m_bEnabled || !pObject)
    {
        return;
    }
    
    std::string sTypeName = GetTypeNameFromObject(pObject);
    if (sTypeName.empty())
    {
        // Can't determine type, just release directly
        pObject->Release();
        return;
    }
    
    auto it = m_TypePools.find(sTypeName);
    if (it != m_TypePools.end())
    {
        it->second->ReleaseObject(pObject);
        UpdateMemoryStats();
    }
    else
    {
        // No pool for this type, release directly
        pObject->Release();
    }
}

ASTypePool* ASObjectPool::GetOrCreateTypePool(const char* szTypeName)
{
    if (!szTypeName)
    {
        return nullptr;
    }
    
    auto it = m_TypePools.find(szTypeName);
    if (it != m_TypePools.end())
    {
        return it->second.get();
    }
    
    // Find the type info
    asITypeInfo* pTypeInfo = FindTypeInfo(szTypeName);
    if (!pTypeInfo)
    {
        return nullptr;
    }
    
    // Create new pool
    auto pPool = std::make_unique<ASTypePool>(pTypeInfo, szTypeName);
    pPool->SetMaxObjects(m_nDefaultMaxObjects);
    pPool->SetObjectLifetime(m_fDefaultObjectLifetime);
    
    ASTypePool* pResult = pPool.get();
    m_TypePools[szTypeName] = std::move(pPool);
    
    return pResult;
}

ASTypePool* ASObjectPool::GetOrCreateTypePool(asITypeInfo* pTypeInfo)
{
    if (!pTypeInfo)
    {
        return nullptr;
    }
    
    const char* szTypeName = pTypeInfo->GetName();
    return GetOrCreateTypePool(szTypeName);
}

void ASObjectPool::RemoveTypePool(const char* szTypeName)
{
    if (!szTypeName)
    {
        return;
    }
    
    auto it = m_TypePools.find(szTypeName);
    if (it != m_TypePools.end())
    {
        m_TypePools.erase(it);
        UpdateMemoryStats();
    }
}

void ASObjectPool::Think()
{
    if (!m_bEnabled)
    {
        return;
    }
    
    // Cleanup old objects in all pools
    CleanupAllPools();
    
    // Update memory statistics
    UpdateMemoryStats();
}

void ASObjectPool::CleanupAllPools()
{
    float fCurrentTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;
    
    for (auto& pair : m_TypePools)
    {
        if (pair.second)
        {
            pair.second->CleanupUnusedObjects(fCurrentTime);
        }
    }
}

void ASObjectPool::ClearAllPools()
{
    for (auto& pair : m_TypePools)
    {
        if (pair.second)
        {
            pair.second->ClearAll();
        }
    }
    m_TypePools.clear();
    
    m_nTotalMemoryUsed = 0;
}

float ASObjectPool::GetReuseRatio() const
{
    if (m_nTotalObjectsCreated == 0)
    {
        return 0.0f;
    }
    
    return static_cast<float>(m_nTotalObjectsReused) / static_cast<float>(m_nTotalObjectsCreated + m_nTotalObjectsReused);
}

void ASObjectPool::PrintPoolStats() const
{
    printf("[ASObjectPool] Total Pools: %zu\n", m_TypePools.size());
    printf("[ASObjectPool] Total Memory: %zu bytes\n", m_nTotalMemoryUsed);
    printf("[ASObjectPool] Peak Memory: %zu bytes\n", m_nPeakMemoryUsed);
    printf("[ASObjectPool] Objects Created: %d\n", m_nTotalObjectsCreated);
    printf("[ASObjectPool] Objects Reused: %d\n", m_nTotalObjectsReused);
    printf("[ASObjectPool] Reuse Ratio: %.2f%%\n", GetReuseRatio() * 100.0f);
}

void ASObjectPool::PrintDetailedStats() const
{
    PrintPoolStats();
    
    for (const auto& pair : m_TypePools)
    {
        const ASTypePool* pPool = pair.second.get();
        if (pPool)
        {
            printf("[Pool:%s] Active: %d, Total: %d, Created: %d, Reused: %d, Memory: %zu bytes\n",
                pPool->GetTypeName(),
                pPool->GetActiveObjectCount(),
                pPool->GetTotalObjectCount(),
                pPool->GetTotalCreated(),
                pPool->GetTotalReused(),
                pPool->GetMemoryUsage());
        }
    }
}

void ASObjectPool::UpdateMemoryStats()
{
    size_t nTotalMemory = 0;
    m_nTotalObjectsCreated = 0;
    m_nTotalObjectsReused = 0;
    
    for (const auto& pair : m_TypePools)
    {
        const ASTypePool* pPool = pair.second.get();
        if (pPool)
        {
            nTotalMemory += pPool->GetMemoryUsage();
            m_nTotalObjectsCreated += pPool->GetTotalCreated();
            m_nTotalObjectsReused += pPool->GetTotalReused();
        }
    }
    
    m_nTotalMemoryUsed = nTotalMemory;
    if (nTotalMemory > m_nPeakMemoryUsed)
    {
        m_nPeakMemoryUsed = nTotalMemory;
    }
}

asITypeInfo* ASObjectPool::FindTypeInfo(const char* szTypeName)
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
    
    // Look for the type in all modules
    asUINT nModuleCount = pEngine->GetModuleCount();
    for (asUINT i = 0; i < nModuleCount; i++)
    {
        asIScriptModule* pModule = pEngine->GetModuleByIndex(i);
        if (!pModule)
        {
            continue;
        }
        
        asITypeInfo* pTypeInfo = pModule->GetTypeInfoByName(szTypeName);
        if (pTypeInfo)
        {
            return pTypeInfo;
        }
    }
    
    return nullptr;
}

std::string ASObjectPool::GetTypeNameFromObject(asIScriptObject* pObject)
{
    if (!pObject)
    {
        return "";
    }
    
    asITypeInfo* pTypeInfo = pObject->GetObjectType();
    if (!pTypeInfo)
    {
        return "";
    }
    
    const char* szTypeName = pTypeInfo->GetName();
    return szTypeName ? szTypeName : "";
}

//==========================================================================
// ASMemoryMonitor Implementation
//==========================================================================

ASMemoryMonitor* ASMemoryMonitor::s_pInstance = nullptr;

ASMemoryMonitor::ASMemoryMonitor()
    : m_nCurrentMemoryUsed(0)
    , m_nPeakMemoryUsed(0)
    , m_nMemoryLimit(128 * 1024 * 1024) // 128MB default for 32-bit
    , m_nLowMemoryThreshold(96 * 1024 * 1024) // 96MB threshold
    , m_nTotalAllocations(0)
    , m_nTotalDeallocations(0)
    , m_nTotalBytesAllocated(0)
    , m_nTotalBytesFreed(0)
    , m_fLastGCTime(0.0f)
    , m_nGCExecutions(0)
    , m_fTotalGCTime(0.0f)
    , m_bAutoGC(true)
    , m_fGCInterval(30.0f) // 30 seconds
    , m_nGCThreshold(64 * 1024 * 1024) // 64MB
{
}

ASMemoryMonitor::~ASMemoryMonitor()
{
}

ASMemoryMonitor* ASMemoryMonitor::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new ASMemoryMonitor();
    }
    return s_pInstance;
}

void ASMemoryMonitor::Shutdown()
{
    if (s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

void ASMemoryMonitor::TrackAllocation(size_t nBytes)
{
    m_nCurrentMemoryUsed += nBytes;
    m_nTotalAllocations++;
    m_nTotalBytesAllocated += nBytes;
    
    if (m_nCurrentMemoryUsed > m_nPeakMemoryUsed)
    {
        m_nPeakMemoryUsed = m_nCurrentMemoryUsed;
    }
    
    CheckMemoryPressure();
}

void ASMemoryMonitor::TrackDeallocation(size_t nBytes)
{
    if (m_nCurrentMemoryUsed >= nBytes)
    {
        m_nCurrentMemoryUsed -= nBytes;
    }
    else
    {
        m_nCurrentMemoryUsed = 0;
    }
    
    m_nTotalDeallocations++;
    m_nTotalBytesFreed += nBytes;
}

void ASMemoryMonitor::UpdateCurrentUsage(size_t nBytes)
{
    m_nCurrentMemoryUsed = nBytes;
    
    if (m_nCurrentMemoryUsed > m_nPeakMemoryUsed)
    {
        m_nPeakMemoryUsed = m_nCurrentMemoryUsed;
    }
    
    CheckMemoryPressure();
}

void ASMemoryMonitor::TriggerGC()
{
    ExecuteGC();
}

float ASMemoryMonitor::GetAverageGCTime() const
{
    if (m_nGCExecutions == 0)
    {
        return 0.0f;
    }
    
    return m_fTotalGCTime / m_nGCExecutions;
}

bool ASMemoryMonitor::Is32BitLimitApproaching() const
{
    // Consider 32-bit limit approaching if we're using more than 75% of available memory
    const size_t n32BitLimit = 2ULL * 1024 * 1024 * 1024; // 2GB
    const size_t nThreshold = (n32BitLimit * 3) / 4; // 75%
    
    return m_nCurrentMemoryUsed > nThreshold;
}

void ASMemoryMonitor::Check32BitConstraints()
{
    if (Is32BitLimitApproaching())
    {
        printf("[ASMemoryMonitor] WARNING: Approaching 32-bit memory limit! Current: %zu MB\n", 
               m_nCurrentMemoryUsed / (1024 * 1024));
        
        // Force garbage collection
        TriggerGC();
        
        // Clear object pools if still critical
        if (IsMemoryLimitExceeded())
        {
            ASObjectPool* pObjectPool = ASObjectPool::Instance();
            if (pObjectPool)
            {
                pObjectPool->ClearAllPools();
            }
        }
    }
}

void ASMemoryMonitor::Think()
{
    float fCurrentTime = GetCurrentTime();
    
    // Auto GC if enabled and interval passed
    if (m_bAutoGC && (fCurrentTime - m_fLastGCTime) > m_fGCInterval)
    {
        ExecuteGC();
    }
    
    // Check 32-bit constraints
    Check32BitConstraints();
}

void ASMemoryMonitor::PrintMemoryStats() const
{
    printf("[ASMemoryMonitor] Current Memory: %zu bytes (%.2f MB)\n", 
           m_nCurrentMemoryUsed, m_nCurrentMemoryUsed / (1024.0f * 1024.0f));
    printf("[ASMemoryMonitor] Peak Memory: %zu bytes (%.2f MB)\n", 
           m_nPeakMemoryUsed, m_nPeakMemoryUsed / (1024.0f * 1024.0f));
    printf("[ASMemoryMonitor] Memory Limit: %zu bytes (%.2f MB)\n", 
           m_nMemoryLimit, m_nMemoryLimit / (1024.0f * 1024.0f));
    printf("[ASMemoryMonitor] Allocations: %d, Deallocations: %d\n", 
           m_nTotalAllocations, m_nTotalDeallocations);
    printf("[ASMemoryMonitor] GC Executions: %d, Average GC Time: %.2f ms\n", 
           m_nGCExecutions, GetAverageGCTime() * 1000.0f);
}

void ASMemoryMonitor::PrintDetailedMemoryReport() const
{
    PrintMemoryStats();
    
    float fMemoryUtilization = static_cast<float>(m_nCurrentMemoryUsed) / m_nMemoryLimit;
    printf("[ASMemoryMonitor] Memory Utilization: %.1f%%\n", fMemoryUtilization * 100.0f);
    
    if (IsLowMemory())
    {
        printf("[ASMemoryMonitor] WARNING: Low memory condition detected!\n");
    }
    
    if (Is32BitLimitApproaching())
    {
        printf("[ASMemoryMonitor] WARNING: Approaching 32-bit memory limit!\n");
    }
}

void ASMemoryMonitor::ExecuteGC()
{
    float fStartTime = GetCurrentTime();
    
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (pManager && pManager->IsInitialized())
    {
        asIScriptEngine* pEngine = pManager->GetEngine();
        if (pEngine)
        {
            pEngine->GarbageCollect(asGC_FULL_CYCLE);
        }
    }
    
    float fEndTime = GetCurrentTime();
    float fGCTime = fEndTime - fStartTime;
    
    m_fLastGCTime = fEndTime;
    m_nGCExecutions++;
    m_fTotalGCTime += fGCTime;
}

void ASMemoryMonitor::CheckMemoryPressure()
{
    if (IsMemoryLimitExceeded())
    {
        printf("[ASMemoryMonitor] Memory limit exceeded! Triggering emergency GC...\n");
        TriggerGC();
    }
    else if (IsLowMemory() && m_bAutoGC)
    {
        if (m_nCurrentMemoryUsed > m_nGCThreshold)
        {
            TriggerGC();
        }
    }
}

float ASMemoryMonitor::GetCurrentTime() const
{
    return static_cast<float>(clock()) / CLOCKS_PER_SEC;
}

//==========================================================================
// ASScriptCache Implementation
//==========================================================================

ASScriptCache* ASScriptCache::s_pInstance = nullptr;

ASScriptCache::ASScriptCache()
    : m_nMaxCachedScripts(32)
    , m_fScriptLifetime(300.0f) // 5 minutes
    , m_nCacheHits(0)
    , m_nCacheMisses(0)
{
}

ASScriptCache::~ASScriptCache()
{
    ClearCache();
}

ASScriptCache* ASScriptCache::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new ASScriptCache();
    }
    return s_pInstance;
}

void ASScriptCache::Shutdown()
{
    if (s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

asIScriptModule* ASScriptCache::GetCachedScript(const char* szScriptName, const char* szContent)
{
    if (!szScriptName)
    {
        return nullptr;
    }
    
    auto it = m_CachedScripts.find(szScriptName);
    if (it != m_CachedScripts.end())
    {
        ASCachedScript& cached = it->second;
        
        // Check if content matches (if provided)
        if (szContent)
        {
            size_t nContentHash = HashString(szContent);
            if (cached.nContentHash != nContentHash)
            {
                // Content changed, remove old cache
                RemoveScript(szScriptName);
                m_nCacheMisses++;
                return nullptr;
            }
        }
        
        // Update usage
        cached.fLastUsed = GetCurrentTime();
        cached.nUsageCount++;
        m_nCacheHits++;
        
        return cached.pModule;
    }
    
    m_nCacheMisses++;
    return nullptr;
}

void ASScriptCache::CacheScript(const char* szScriptName, const char* szContent, asIScriptModule* pModule)
{
    if (!szScriptName || !pModule)
    {
        return;
    }
    
    // Remove old entry if exists
    RemoveScript(szScriptName);
    
    // Check cache size limit
    if (static_cast<int>(m_CachedScripts.size()) >= m_nMaxCachedScripts)
    {
        RemoveOldestScript();
    }
    
    // Create new cache entry
    ASCachedScript cached;
    cached.sScriptContent = szContent ? szContent : "";
    cached.pModule = pModule;
    cached.fLastUsed = GetCurrentTime();
    cached.nUsageCount = 1;
    cached.nContentHash = szContent ? HashString(szContent) : 0;
    
    // Note: Modules are managed by engine, no manual reference counting needed
    
    m_CachedScripts[szScriptName] = cached;
}

void ASScriptCache::RemoveScript(const char* szScriptName)
{
    if (!szScriptName)
    {
        return;
    }
    
    auto it = m_CachedScripts.find(szScriptName);
    if (it != m_CachedScripts.end())
    {
        // Note: Modules are managed by engine, no manual cleanup needed
        m_CachedScripts.erase(it);
    }
}

void ASScriptCache::ClearCache()
{
    for (auto& pair : m_CachedScripts)
    {
        // Note: Modules are managed by engine, no manual cleanup needed
    }
    m_CachedScripts.clear();
}

void ASScriptCache::CleanupOldScripts()
{
    float fCurrentTime = GetCurrentTime();
    
    auto it = m_CachedScripts.begin();
    while (it != m_CachedScripts.end())
    {
        if ((fCurrentTime - it->second.fLastUsed) > m_fScriptLifetime)
        {
            // Note: Modules are managed by engine, no manual cleanup needed
            it = m_CachedScripts.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

float ASScriptCache::GetCacheHitRatio() const
{
    int nTotalRequests = m_nCacheHits + m_nCacheMisses;
    if (nTotalRequests == 0)
    {
        return 0.0f;
    }
    
    return static_cast<float>(m_nCacheHits) / nTotalRequests;
}

size_t ASScriptCache::HashString(const char* szContent) const
{
    if (!szContent)
    {
        return 0;
    }
    
    // Simple hash function
    std::hash<std::string> hasher;
    return hasher(szContent);
}

void ASScriptCache::RemoveOldestScript()
{
    if (m_CachedScripts.empty())
    {
        return;
    }
    
    auto oldestIt = m_CachedScripts.begin();
    float fOldestTime = oldestIt->second.fLastUsed;
    
    for (auto it = m_CachedScripts.begin(); it != m_CachedScripts.end(); ++it)
    {
        if (it->second.fLastUsed < fOldestTime)
        {
            fOldestTime = it->second.fLastUsed;
            oldestIt = it;
        }
    }
    
    // Note: Modules are managed by engine, no manual cleanup needed
    m_CachedScripts.erase(oldestIt);
}

float ASScriptCache::GetCurrentTime() const
{
    return static_cast<float>(clock()) / CLOCKS_PER_SEC;
}

//==========================================================================
// Global Functions
//==========================================================================

void RegisterMemoryOptimizationFunctions(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        return;
    }
    
    // TODO: Register script-callable memory optimization functions
    // This would include functions like:
    // - void CollectGarbage()
    // - int GetMemoryUsage()
    // - void SetMemoryLimit(int)
    // etc.
}

void InitializeOptimizationSystems()
{
    // Initialize all optimization systems
    ASObjectPool::Instance();
    ASMemoryMonitor::Instance();
    ASScriptCache::Instance();
}

void ShutdownOptimizationSystems()
{
    ASScriptCache::Shutdown();
    ASMemoryMonitor::Shutdown();
    ASObjectPool::Shutdown();
}

void ThinkOptimizationSystems()
{
    ASObjectPool* pObjectPool = ASObjectPool::Instance();
    if (pObjectPool)
    {
        pObjectPool->Think();
    }
    
    ASMemoryMonitor* pMemoryMonitor = ASMemoryMonitor::Instance();
    if (pMemoryMonitor)
    {
        pMemoryMonitor->Think();
    }
    
    ASScriptCache* pScriptCache = ASScriptCache::Instance();
    if (pScriptCache)
    {
        pScriptCache->CleanupOldScripts();
    }
}