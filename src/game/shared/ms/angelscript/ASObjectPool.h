//==========================================================================
// ASObjectPool.h - AngelScript Object Pooling and Memory Optimization
// Master Sword Rebirth AngelScript Implementation
//==========================================================================

#ifndef ASOBJECTPOOL_H
#define ASOBJECTPOOL_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <queue>

class asIScriptEngine;
class asIScriptModule;
class asIScriptObject;
class asITypeInfo;

//==========================================================================
// Pooled object information
//==========================================================================
struct ASPooledObject
{
    asIScriptObject* pObject;
    asITypeInfo* pTypeInfo;
    bool bInUse;
    float fLastUsed;        // Time when object was last used
    int nUsageCount;        // How many times this object has been used
    
    ASPooledObject() 
        : pObject(nullptr)
        , pTypeInfo(nullptr)
        , bInUse(false)
        , fLastUsed(0.0f)
        , nUsageCount(0)
    {
    }
};

//==========================================================================
// Object pool for a specific type
//==========================================================================
class ASTypePool
{
private:
    std::vector<ASPooledObject> m_Objects;
    asITypeInfo* m_pTypeInfo;
    std::string m_sTypeName;
    
    // Pool configuration
    int m_nMaxObjects;
    int m_nMinObjects;
    float m_fObjectLifetime;  // How long to keep unused objects
    
    // Performance metrics
    int m_nTotalCreated;
    int m_nTotalReused;
    int m_nActiveObjects;
    
public:
    ASTypePool(asITypeInfo* pTypeInfo, const char* szTypeName);
    ~ASTypePool();
    
    // Pool management
    asIScriptObject* AcquireObject();
    void ReleaseObject(asIScriptObject* pObject);
    
    // Maintenance
    void CleanupUnusedObjects(float fCurrentTime);
    void ClearAll();
    
    // Configuration
    void SetMaxObjects(int nMax) { m_nMaxObjects = nMax; }
    void SetMinObjects(int nMin) { m_nMinObjects = nMin; }
    void SetObjectLifetime(float fLifetime) { m_fObjectLifetime = fLifetime; }
    
    // Information
    const char* GetTypeName() const { return m_sTypeName.c_str(); }
    int GetActiveObjectCount() const { return m_nActiveObjects; }
    int GetTotalObjectCount() const { return m_Objects.size(); }
    int GetTotalCreated() const { return m_nTotalCreated; }
    int GetTotalReused() const { return m_nTotalReused; }
    
    // Memory usage
    size_t GetMemoryUsage() const;
    
private:
    asIScriptObject* CreateNewObject();
    ASPooledObject* FindAvailableObject();
    void RemoveOldestUnusedObject();
    float GetCurrentTime() const;
};

//==========================================================================
// Main object pooling manager
//==========================================================================
class ASObjectPool
{
private:
    std::unordered_map<std::string, std::unique_ptr<ASTypePool>> m_TypePools;
    
    // Configuration
    bool m_bEnabled;
    int m_nDefaultMaxObjects;
    float m_fDefaultObjectLifetime;
    
    // Performance tracking
    size_t m_nTotalMemoryUsed;
    size_t m_nPeakMemoryUsed;
    int m_nTotalObjectsCreated;
    int m_nTotalObjectsReused;
    
    // Singleton
    static ASObjectPool* s_pInstance;
    
    ASObjectPool();
    
public:
    ~ASObjectPool();
    
    // Singleton access
    static ASObjectPool* Instance();
    static void Shutdown();
    
    // Pool management
    asIScriptObject* AcquireObject(const char* szTypeName);
    asIScriptObject* AcquireObject(asITypeInfo* pTypeInfo);
    void ReleaseObject(asIScriptObject* pObject);
    
    // Type pool management
    ASTypePool* GetOrCreateTypePool(const char* szTypeName);
    ASTypePool* GetOrCreateTypePool(asITypeInfo* pTypeInfo);
    void RemoveTypePool(const char* szTypeName);
    
    // Configuration
    void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }
    bool IsEnabled() const { return m_bEnabled; }
    void SetDefaultMaxObjects(int nMax) { m_nDefaultMaxObjects = nMax; }
    void SetDefaultObjectLifetime(float fLifetime) { m_fDefaultObjectLifetime = fLifetime; }
    
    // Maintenance
    void Think(); // Called from game loop for cleanup
    void CleanupAllPools();
    void ClearAllPools();
    
    // Performance monitoring
    size_t GetTotalMemoryUsed() const { return m_nTotalMemoryUsed; }
    size_t GetPeakMemoryUsed() const { return m_nPeakMemoryUsed; }
    int GetTotalObjectsCreated() const { return m_nTotalObjectsCreated; }
    int GetTotalObjectsReused() const { return m_nTotalObjectsReused; }
    float GetReuseRatio() const;
    
    // Debug information
    void PrintPoolStats() const;
    void PrintDetailedStats() const;
    
private:
    void UpdateMemoryStats();
    asITypeInfo* FindTypeInfo(const char* szTypeName);
    std::string GetTypeNameFromObject(asIScriptObject* pObject);
};

//==========================================================================
// Memory monitoring and optimization
//==========================================================================
class ASMemoryMonitor
{
private:
    // Memory tracking
    size_t m_nCurrentMemoryUsed;
    size_t m_nPeakMemoryUsed;
    size_t m_nMemoryLimit;
    size_t m_nLowMemoryThreshold;
    
    // Allocation tracking
    int m_nTotalAllocations;
    int m_nTotalDeallocations;
    size_t m_nTotalBytesAllocated;
    size_t m_nTotalBytesFreed;
    
    // Performance metrics
    float m_fLastGCTime;
    int m_nGCExecutions;
    float m_fTotalGCTime;
    
    // Configuration
    bool m_bAutoGC;
    float m_fGCInterval;
    size_t m_nGCThreshold;
    
    // Singleton
    static ASMemoryMonitor* s_pInstance;
    
    ASMemoryMonitor();
    
public:
    ~ASMemoryMonitor();
    
    // Singleton access
    static ASMemoryMonitor* Instance();
    static void Shutdown();
    
    // Memory tracking
    void TrackAllocation(size_t nBytes);
    void TrackDeallocation(size_t nBytes);
    void UpdateCurrentUsage(size_t nBytes);
    
    // Memory limits
    void SetMemoryLimit(size_t nBytes) { m_nMemoryLimit = nBytes; }
    size_t GetMemoryLimit() const { return m_nMemoryLimit; }
    bool IsMemoryLimitExceeded() const { return m_nCurrentMemoryUsed > m_nMemoryLimit; }
    bool IsLowMemory() const { return m_nCurrentMemoryUsed > m_nLowMemoryThreshold; }
    
    // Garbage collection
    void TriggerGC();
    void SetAutoGC(bool bEnabled) { m_bAutoGC = bEnabled; }
    void SetGCInterval(float fInterval) { m_fGCInterval = fInterval; }
    void SetGCThreshold(size_t nThreshold) { m_nGCThreshold = nThreshold; }
    
    // Statistics
    size_t GetCurrentMemoryUsed() const { return m_nCurrentMemoryUsed; }
    size_t GetPeakMemoryUsed() const { return m_nPeakMemoryUsed; }
    int GetTotalAllocations() const { return m_nTotalAllocations; }
    float GetAverageGCTime() const;
    
    // 32-bit specific monitoring
    bool Is32BitLimitApproaching() const;
    void Check32BitConstraints();
    
    // Frame processing
    void Think(); // Called each frame to check memory and trigger GC if needed
    
    // Debug output
    void PrintMemoryStats() const;
    void PrintDetailedMemoryReport() const;
    
private:
    void ExecuteGC();
    void CheckMemoryPressure();
    float GetCurrentTime() const;
};

//==========================================================================
// Script caching system
//==========================================================================
struct ASCachedScript
{
    std::string sScriptContent;
    asIScriptModule* pModule;
    float fLastUsed;
    int nUsageCount;
    size_t nContentHash;
    
    ASCachedScript()
        : pModule(nullptr)
        , fLastUsed(0.0f)
        , nUsageCount(0)
        , nContentHash(0)
    {
    }
};

class ASScriptCache
{
private:
    std::unordered_map<std::string, ASCachedScript> m_CachedScripts;
    
    // Configuration
    int m_nMaxCachedScripts;
    float m_fScriptLifetime;
    
    // Performance tracking
    int m_nCacheHits;
    int m_nCacheMisses;
    
    // Singleton
    static ASScriptCache* s_pInstance;
    
    ASScriptCache();
    
public:
    ~ASScriptCache();
    
    // Singleton access
    static ASScriptCache* Instance();
    static void Shutdown();
    
    // Cache management
    asIScriptModule* GetCachedScript(const char* szScriptName, const char* szContent = nullptr);
    void CacheScript(const char* szScriptName, const char* szContent, asIScriptModule* pModule);
    void RemoveScript(const char* szScriptName);
    void ClearCache();
    
    // Maintenance
    void CleanupOldScripts();
    
    // Configuration
    void SetMaxCachedScripts(int nMax) { m_nMaxCachedScripts = nMax; }
    void SetScriptLifetime(float fLifetime) { m_fScriptLifetime = fLifetime; }
    
    // Statistics
    int GetCacheHits() const { return m_nCacheHits; }
    int GetCacheMisses() const { return m_nCacheMisses; }
    float GetCacheHitRatio() const;
    int GetCachedScriptCount() const { return m_CachedScripts.size(); }
    
private:
    size_t HashString(const char* szContent) const;
    void RemoveOldestScript();
    float GetCurrentTime() const;
};

//==========================================================================
// Global functions for integration
//==========================================================================

// Register memory optimization functions with AngelScript
void RegisterMemoryOptimizationFunctions(asIScriptEngine* pEngine);

// Initialize all optimization systems
void InitializeOptimizationSystems();
void ShutdownOptimizationSystems();

// Frame processing for all systems
void ThinkOptimizationSystems();

#endif // ASOBJECTPOOL_H