#ifndef ASSCRIPTCLASSES_H
#define ASSCRIPTCLASSES_H

#include <angelscript.h>
#include <map>
#include <string>

// Forward declarations
class CBaseEntity;

//
// Base script class that all AngelScript classes can inherit from
// Provides entity reference, variable storage, and event system
//
class CGameScript
{
public:
    CGameScript();
    virtual ~CGameScript();
    
    // Entity management
    void SetOwner(CBaseEntity* pEntity);
    CBaseEntity* GetOwner() const;
    bool IsValidOwner() const;
    
    // Variable system - compatible with legacy script system
    void SetVar(const std::string& name, const std::string& value);
    void SetVar(const std::string& name, float value);
    void SetVar(const std::string& name, int value);
    std::string GetVarString(const std::string& name, const std::string& defaultValue = "");
    float GetVarFloat(const std::string& name, float defaultValue = 0.0f);
    int GetVarInt(const std::string& name, int defaultValue = 0);
    bool HasVar(const std::string& name) const;
    void RemoveVar(const std::string& name);
    void ClearVars();
    
    // Script event callbacks - Override these in derived classes
    virtual void OnSpawn() {}
    virtual void OnThink() {}
    virtual void OnDamage(int damage) {}
    virtual void OnUse(CBaseEntity* activator, CBaseEntity* caller, int useType) {}
    virtual void OnTouch(CBaseEntity* other) {}
    virtual void OnBlocked(CBaseEntity* other) {}
    virtual void OnDeath(CBaseEntity* attacker) {}
    virtual void OnRevive() {}
    virtual void OnTakeDamage(CBaseEntity* inflictor, CBaseEntity* attacker, float damage, int damageType) {}
    virtual void OnStartAttack() {}
    virtual void OnEndAttack() {}
    virtual void OnHitByAttack(CBaseEntity* attacker, float damage) {}
    virtual void OnMissedByAttack(CBaseEntity* attacker) {}
    virtual void OnTrigger(CBaseEntity* activator) {}
    virtual void OnMovementKeyPressed() {}
    virtual void OnScriptSay(const std::string& text) {}
    
protected:
    CBaseEntity* m_pEntity;                          // The entity this script is attached to
    std::map<std::string, std::string> m_Variables;  // String-based variable storage for compatibility
    
private:
    // Copy constructor and assignment operator - disabled for safety
    CGameScript(const CGameScript&);
    CGameScript& operator=(const CGameScript&);
};

//
// AngelScript binding namespace for script classes
//
namespace ASScriptClasses
{
    // Registration functions
    bool RegisterAll(asIScriptEngine* pEngine);
    bool RegisterCGameScript(asIScriptEngine* pEngine);
    bool RegisterScriptClassFactories(asIScriptEngine* pEngine);
    
    // Script class management functions
    CGameScript* CreateScriptInstance(const std::string& className);
    void DestroyScriptInstance(CGameScript* pScript);
    
    // AngelScript reference counting wrapper for CGameScript
    void CGameScript_AddRef(CGameScript* obj);
    void CGameScript_Release(CGameScript* obj);
    
    // AngelScript constructor/destructor wrappers
    CGameScript* CGameScript_Construct();
    void CGameScript_Destruct(CGameScript* obj);
    
    // AngelScript-compatible method wrappers for variable system
    void CGameScript_SetVarString(CGameScript* obj, const std::string& name, const std::string& value);
    void CGameScript_SetVarFloat(CGameScript* obj, const std::string& name, float value);
    void CGameScript_SetVarInt(CGameScript* obj, const std::string& name, int value);
    std::string CGameScript_GetVarString(CGameScript* obj, const std::string& name, const std::string& defaultValue);
    float CGameScript_GetVarFloat(CGameScript* obj, const std::string& name, float defaultValue);
    int CGameScript_GetVarInt(CGameScript* obj, const std::string& name, int defaultValue);
    bool CGameScript_HasVar(CGameScript* obj, const std::string& name);
    void CGameScript_RemoveVar(CGameScript* obj, const std::string& name);
    void CGameScript_ClearVars(CGameScript* obj);
    
    // Entity reference wrappers
    void CGameScript_SetOwner(CGameScript* obj, CBaseEntity* pEntity);
    CBaseEntity* CGameScript_GetOwner(CGameScript* obj);
    bool CGameScript_IsValidOwner(CGameScript* obj);
    
    // Event callback registration helpers
    bool RegisterEventCallback(asIScriptEngine* pEngine, const char* className, const char* eventName, const char* signature);
}

#endif // ASSCRIPTCLASSES_H