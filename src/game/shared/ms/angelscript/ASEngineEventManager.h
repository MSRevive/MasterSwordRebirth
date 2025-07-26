#ifndef ASENGINEEVENTMANAGER_H
#define ASENGINEEVENTMANAGER_H

//==========================================================================
// AngelScript Engine Event Manager
// 
// Manages global engine events that AngelScript can subscribe to
// Provides bridge between C++ engine events and AngelScript handlers
//==========================================================================

#include <vector>
#include <string>

class asIScriptEngine;
class asIScriptContext;
class asIScriptFunction;
class asIScriptGeneric;

//==========================================================================
// Engine Event Types
//==========================================================================
enum class EngineEventType
{
    PLAYER_CONNECT,
    PLAYER_DISCONNECT, 
    MONSTER_KILLED,
    TREASURE_SPAWNED,
    PLAYER_SPAWNED,
    PLAYER_SAY_TEXT
};

//==========================================================================
// Event Handler Structure
//==========================================================================
struct EngineEventHandler
{
    EngineEventType eventType;
    asIScriptFunction* pFunction;
    std::string moduleName;
    
    EngineEventHandler(EngineEventType type, asIScriptFunction* func, const std::string& module)
        : eventType(type), pFunction(func), moduleName(module) {}
};

//==========================================================================
// ASEngineEventManager - Singleton Event Dispatcher
//==========================================================================
class ASEngineEventManager
{
private:
    static ASEngineEventManager* s_pInstance;
    
    std::vector<EngineEventHandler> m_EventHandlers;
    asIScriptEngine* m_pEngine;
    bool m_bInitialized;

    // Recursion protection for PLAYER_SAY_TEXT events
    static int s_nSayTextEventDepth;
    static const int MAX_SAY_TEXT_EVENT_DEPTH = 2;

    // RAII helper for depth management
    class SayTextEventDepthGuard
    {
    public:
        SayTextEventDepthGuard() { ++s_nSayTextEventDepth; }
        ~SayTextEventDepthGuard() { --s_nSayTextEventDepth; }
        
        bool IsMaxDepthExceeded() const { return s_nSayTextEventDepth > MAX_SAY_TEXT_EVENT_DEPTH; }
        int GetCurrentDepth() const { return s_nSayTextEventDepth; }
    };

    // Private constructor for singleton
    ASEngineEventManager();
    
public:
    ~ASEngineEventManager();
    
    // Singleton access
    static ASEngineEventManager* Instance();
    static void Shutdown();
    
    // Initialization
    bool Initialize(asIScriptEngine* pEngine);
    void Destroy();
    
    // Event Registration (called from AngelScript)
    bool RegisterEventHandler(EngineEventType eventType, asIScriptFunction* pFunction, const char* szModuleName);
    void UnregisterEventHandler(EngineEventType eventType, const char* szModuleName);
    void UnregisterAllHandlers(const char* szModuleName);
    
    // Event Dispatching (called from C++ engine)
    void FirePlayerConnectEvent(const char* szPlayerName, const char* szSteamID);
    void FirePlayerDisconnectEvent(const char* szPlayerName, const char* szSteamID);
    void FireMonsterKilledEvent(const char* szMonsterName, const char* szKillerName, float fPosX, float fPosY, float fPosZ);
    void FireTreasureSpawnedEvent(const char* szTreasureName, float fPosX, float fPosY, float fPosZ);
    void FirePlayerSpawnedEvent(const char* szPlayerName);
    void FirePlayerSayTextEvent(const char* szPlayerName, const char* szSteamID, const char* szText);
    
    // Utility
    void LogEventHandlers();
    int GetHandlerCount(EngineEventType eventType) const;
    const char* GetEventTypeName(EngineEventType eventType) const;
    
private:
    // Internal event dispatch
    void DispatchEvent(EngineEventType eventType, const std::vector<std::string>& parameters);
    
    // Helper functions
    asIScriptContext* AcquireContext();
    void ReleaseContext(asIScriptContext* pContext);
    bool IsParameterByReference(asIScriptFunction* pFunction, int paramIndex);
};

//==========================================================================
// AngelScript Registration Functions
//==========================================================================
namespace ASEngineEvents
{
    // Register engine event functions with AngelScript
    bool RegisterEngineEventFunctions(asIScriptEngine* pEngine);
    
    // AngelScript callable functions
    void RegisterEngineEvent_Generic(asIScriptGeneric* gen);
    void UnregisterEngineEvent(const std::string& eventName);
    void LogEngineEventHandlers();
}

#endif // ASENGINEEVENTMANAGER_H