#include "ASScriptClasses.h"
#include <iostream>
#include <sstream>
#include <cstdio>

// Include for CBaseEntity - will be forward declaration for now to avoid circular dependencies
// #include "hl/cbase.h"  // Commented out to avoid build complexity
class CBaseEntity;  // Forward declaration placeholder

//
// CGameScript Implementation
//

CGameScript::CGameScript()
    : m_pEntity(nullptr)
{
    // Initialize empty variable dictionary
    m_Variables.clear();
}

CGameScript::~CGameScript()
{
    // Clean up variables
    m_Variables.clear();
    
    // Note: We don't delete m_pEntity as we don't own it
    m_pEntity = nullptr;
}

void CGameScript::SetOwner(CBaseEntity* pEntity)
{
    m_pEntity = pEntity;
}

CBaseEntity* CGameScript::GetOwner() const
{
    return m_pEntity;
}

bool CGameScript::IsValidOwner() const
{
    return m_pEntity != nullptr;
}

void CGameScript::SetVar(const std::string& name, const std::string& value)
{
    m_Variables[name] = value;
}

void CGameScript::SetVar(const std::string& name, float value)
{
    std::ostringstream oss;
    oss << value;
    m_Variables[name] = oss.str();
}

void CGameScript::SetVar(const std::string& name, int value)
{
    std::ostringstream oss;
    oss << value;
    m_Variables[name] = oss.str();
}

std::string CGameScript::GetVarString(const std::string& name, const std::string& defaultValue)
{
    auto it = m_Variables.find(name);
    if (it != m_Variables.end())
        return it->second;
    return defaultValue;
}

float CGameScript::GetVarFloat(const std::string& name, float defaultValue)
{
    auto it = m_Variables.find(name);
    if (it != m_Variables.end())
    {
        float result = defaultValue;
        std::istringstream iss(it->second);
        iss >> result;
        return result;
    }
    return defaultValue;
}

int CGameScript::GetVarInt(const std::string& name, int defaultValue)
{
    auto it = m_Variables.find(name);
    if (it != m_Variables.end())
    {
        int result = defaultValue;
        std::istringstream iss(it->second);
        iss >> result;
        return result;
    }
    return defaultValue;
}

bool CGameScript::HasVar(const std::string& name) const
{
    return m_Variables.find(name) != m_Variables.end();
}

void CGameScript::RemoveVar(const std::string& name)
{
    m_Variables.erase(name);
}

void CGameScript::ClearVars()
{
    m_Variables.clear();
}

//
// ASScriptClasses Implementation
//

namespace ASScriptClasses
{
    // Master registration function
    bool RegisterAll(asIScriptEngine* pEngine)
    {
        if (!pEngine)
        {
            printf("ASScriptClasses::RegisterAll: ERROR - NULL engine pointer\\n");
            return false;
        }
        
        printf("\\n=== ASScriptClasses: Registering Script Class System ===\\n");
        
        bool success = true;
        
        // Step 1: Register base CGameScript class
        printf("[1/2] Registering CGameScript base class...\\n");
        if (!RegisterCGameScript(pEngine))
        {
            printf("   ERROR: CGameScript registration failed!\\n");
            success = false;
        }
        else
        {
            printf("   ✓ CGameScript registered successfully\\n");
        }
        
        // Step 2: Register script class factory system
        printf("[2/2] Registering script class factories...\\n");
        if (!RegisterScriptClassFactories(pEngine))
        {
            printf("   ERROR: Script class factory registration failed!\\n");
            success = false;
        }
        else
        {
            printf("   ✓ Script class factories registered successfully\\n");
        }
        
        if (success)
        {
            printf("=== ASScriptClasses: Registration Complete ===\\n\\n");
        }
        else
        {
            printf("=== ASScriptClasses: Registration FAILED ===\\n\\n");
        }
        
        return success;
    }
    
    // Register the CGameScript base class
    bool RegisterCGameScript(asIScriptEngine* pEngine)
        {
            if (!pEngine) return false;
            
            int r = 0;
            
            // Register the CGameScript class as reference type with factory behavior
            r = pEngine->RegisterObjectType("CGameScript", 0, asOBJ_REF | asOBJ_NOCOUNT);
            if (r < 0)
            {
                printf("ASScriptClasses: Failed to register CGameScript type\n");
                return false;
            }
            
            // Register factory behavior for construction
            r = pEngine->RegisterObjectBehaviour("CGameScript", asBEHAVE_FACTORY, "CGameScript@ f()", asFUNCTION(CGameScript_Construct), asCALL_CDECL);
            if (r < 0) printf("ASScriptClasses: Warning - Failed to register CGameScript factory\n");
            
            // Register variable system methods
            r = pEngine->RegisterObjectMethod("CGameScript", "void SetVar(const string &in, const string &in)", asFUNCTION(CGameScript_SetVarString), asCALL_CDECL_OBJFIRST);
            if (r < 0) printf("ASScriptClasses: Warning - Failed to register SetVar(string)\n");
            
            r = pEngine->RegisterObjectMethod("CGameScript", "void SetVar(const string &in, float)", asFUNCTION(CGameScript_SetVarFloat), asCALL_CDECL_OBJFIRST);
            if (r < 0) printf("ASScriptClasses: Warning - Failed to register SetVar(float)\n");
            
            r = pEngine->RegisterObjectMethod("CGameScript", "void SetVar(const string &in, int)", asFUNCTION(CGameScript_SetVarInt), asCALL_CDECL_OBJFIRST);
            if (r < 0) printf("ASScriptClasses: Warning - Failed to register SetVar(int)\n");
            
            r = pEngine->RegisterObjectMethod("CGameScript", "string GetVar(const string &in, const string &in = \"\")", asFUNCTION(CGameScript_GetVarString), asCALL_CDECL_OBJFIRST);
            if (r < 0) printf("ASScriptClasses: Warning - Failed to register GetVar(string)\n");
            
            r = pEngine->RegisterObjectMethod("CGameScript", "bool HasVar(const string &in)", asFUNCTION(CGameScript_HasVar), asCALL_CDECL_OBJFIRST);
            if (r < 0) printf("ASScriptClasses: Warning - Failed to register HasVar\n");
            
            r = pEngine->RegisterObjectMethod("CGameScript", "void RemoveVar(const string &in)", asFUNCTION(CGameScript_RemoveVar), asCALL_CDECL_OBJFIRST);
            if (r < 0) printf("ASScriptClasses: Warning - Failed to register RemoveVar\n");
            
            r = pEngine->RegisterObjectMethod("CGameScript", "void ClearVars()", asFUNCTION(CGameScript_ClearVars), asCALL_CDECL_OBJFIRST);
            if (r < 0) printf("ASScriptClasses: Warning - Failed to register ClearVars\n");
            
            // Register virtual event methods that can be overridden in script
            r = pEngine->RegisterObjectMethod("CGameScript", "void OnSpawn()", asMETHOD(CGameScript, OnSpawn), asCALL_THISCALL);
            if (r < 0) printf("ASScriptClasses: Warning - Failed to register OnSpawn\n");
            
            r = pEngine->RegisterObjectMethod("CGameScript", "void OnThink()", asMETHOD(CGameScript, OnThink), asCALL_THISCALL);
            if (r < 0) printf("ASScriptClasses: Warning - Failed to register OnThink\n");
            
            r = pEngine->RegisterObjectMethod("CGameScript", "void OnDamage(int)", asMETHOD(CGameScript, OnDamage), asCALL_THISCALL);
            if (r < 0) printf("ASScriptClasses: Warning - Failed to register OnDamage\n");
            
            // Register entity management methods
            r = pEngine->RegisterObjectMethod("CGameScript", "bool IsValidOwner()", asFUNCTION(CGameScript_IsValidOwner), asCALL_CDECL_OBJFIRST);
            if (r < 0) printf("ASScriptClasses: Warning - Failed to register IsValidOwner\n");
            
            printf("ASScriptClasses: Base CGameScript class registered with variable system and virtual methods\n");
            return true;
        }

    
    // Register script class factory system
    bool RegisterScriptClassFactories(asIScriptEngine* pEngine)
        {
            if (!pEngine) return false;
            
            // Enable script classes to be defined in AngelScript
            int r = pEngine->RegisterInterface("IScriptInterface");
            if (r >= 0)
            {
                printf("ASScriptClasses: Registered IScriptInterface for script-defined interfaces\n");
            }
            
            // Register global factory functions for script class creation
            r = pEngine->RegisterGlobalFunction("CGameScript@ CreateScript(const string &in)", asFUNCTION(CreateScriptInstance), asCALL_CDECL);
            if (r < 0)
            {
                printf("ASScriptClasses: Failed to register CreateScript function\n");
                return false;
            }
            
            // Configure engine for script classes
            // Allow ref counted script objects
            pEngine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
            
            printf("ASScriptClasses: Script class factory system registered\n");
            return true;
        }

    
    // Script class management functions
    CGameScript* CreateScriptInstance(const std::string& className)
    {
        // For now we only support the base CGameScript
        // Future versions will support derived classes
        if (className == "CGameScript" || className.empty())
        {
            return new CGameScript();
        }
        
        printf("ASScriptClasses: Warning - Unknown script class '%s', creating base CGameScript\\n", className.c_str());
        return new CGameScript();
    }
    
    void DestroyScriptInstance(CGameScript* pScript)
    {
        if (pScript)
        {
            delete pScript;
        }
    }
    
    // AngelScript reference counting wrappers
    void CGameScript_AddRef(CGameScript* obj)
    {
        // For now we use simple new/delete
        // Future versions may implement proper reference counting
        // For now, this is a no-op since AngelScript manages the lifetime
    }
    
    void CGameScript_Release(CGameScript* obj)
    {
        // For now we use simple new/delete
        // The actual deletion is handled by the factory system
        if (obj)
        {
            delete obj;
        }
    }
    
    // AngelScript constructor wrapper
    CGameScript* CGameScript_Construct()
    {
        // This is called by AngelScript when constructing the object
        return new CGameScript();
    }
    
    // AngelScript destructor wrapper
    void CGameScript_Destruct(CGameScript* obj)
    {
        // This is called by AngelScript when destructing the object
        obj->~CGameScript();
    }
    
    // AngelScript-compatible method wrappers for variable system
    void CGameScript_SetVarString(CGameScript* obj, const std::string& name, const std::string& value)
    {
        if (obj) obj->SetVar(name, value);
    }
    
    void CGameScript_SetVarFloat(CGameScript* obj, const std::string& name, float value)
    {
        if (obj) obj->SetVar(name, value);
    }
    
    void CGameScript_SetVarInt(CGameScript* obj, const std::string& name, int value)
    {
        if (obj) obj->SetVar(name, value);
    }
    
    std::string CGameScript_GetVarString(CGameScript* obj, const std::string& name, const std::string& defaultValue)
    {
        if (obj) return obj->GetVarString(name, defaultValue);
        return defaultValue;
    }
    
    float CGameScript_GetVarFloat(CGameScript* obj, const std::string& name, float defaultValue)
    {
        if (obj) return obj->GetVarFloat(name, defaultValue);
        return defaultValue;
    }
    
    int CGameScript_GetVarInt(CGameScript* obj, const std::string& name, int defaultValue)
    {
        if (obj) return obj->GetVarInt(name, defaultValue);
        return defaultValue;
    }
    
    bool CGameScript_HasVar(CGameScript* obj, const std::string& name)
    {
        if (obj) return obj->HasVar(name);
        return false;
    }
    
    void CGameScript_RemoveVar(CGameScript* obj, const std::string& name)
    {
        if (obj) obj->RemoveVar(name);
    }
    
    void CGameScript_ClearVars(CGameScript* obj)
    {
        if (obj) obj->ClearVars();
    }
    
    // Entity reference wrappers
    void CGameScript_SetOwner(CGameScript* obj, CBaseEntity* pEntity)
    {
        if (obj) obj->SetOwner(pEntity);
    }
    
    CBaseEntity* CGameScript_GetOwner(CGameScript* obj)
    {
        if (obj) return obj->GetOwner();
        return nullptr;
    }
    
    bool CGameScript_IsValidOwner(CGameScript* obj)
    {
        if (obj) return obj->IsValidOwner();
        return false;
    }
    
    // Event callback registration helper
    bool RegisterEventCallback(asIScriptEngine* pEngine, const char* className, const char* eventName, const char* signature)
    {
        if (!pEngine || !className || !eventName || !signature)
            return false;
        
        // For now this is a placeholder for future event system integration
        // Future versions will implement full event callback registration via a virtual method in the script class
        printf("ASScriptClasses: Event callback registration placeholder - %s::%s%s\\n", className, eventName, signature);
        return true;
    }
}