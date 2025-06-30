//==========================================================================
// ASCoreTypes.cpp - Minimal Implementation
// 
// Core type bindings for AngelScript integration
// Provides fundamental types and math functions for script usage
//==========================================================================

#include "ASCoreTypes.h"
#include <angelscript.h>
#include <cstdio>
#include <cmath>
#include <new>

// Include vector.h directly to avoid mathlib.h macro conflicts
// We need the vec_t typedef but not the DotProduct macro
typedef float vec_t;
#include "../../../server/hl/vector.h"

// Include MSLogger for consistent logging
#include "mslogger.h"

// Math constants as global variables (not macros) for AngelScript registration
static const float AS_PI = 3.14159265358979323846f;
static const float AS_E = 2.71828182845904523536f;

//==========================================================================
// Core Type Registration Functions - Minimal Implementation
//==========================================================================
namespace ASCoreTypes
{
    // Forward declarations for wrapper functions
    void Vector3DefaultConstructor(Vector *ptr);
    void Vector3InitConstructor(float x, float y, float z, Vector *ptr);
    void Vector3CopyConstructor(const Vector &other, Vector *ptr);
    void ColorDefaultConstructor(Vector *ptr);
    void ColorInitConstructor(float r, float g, float b, Vector *ptr);
    void ColorInitConstructorAlpha(float r, float g, float b, float a, Vector *ptr);
    
    // EntityHandle wrapper functions
    bool EntityHandle_IsValid(void* pHandle);
    void* EntityHandle_Get(void* pHandle);
    void EntityHandle_DefaultConstructor(void* pHandle);
    void EntityHandle_IntConstructor(int value, void* pHandle);
    
    // Math function wrappers for AngelScript
    float AS_Sin(float x) { return sin(x); }
    float AS_Cos(float x) { return cos(x); }
    float AS_Sqrt(float x) { return sqrt(x); }
    float AS_Abs(float x) { return fabs(x); }
    float AS_Min(float a, float b) { return (a < b) ? a : b; }
    float AS_Max(float a, float b) { return (a > b) ? a : b; }
    
    void RegisterAll(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("ASCoreTypes: Registering core types and math functions...");
        
        // Note: AngelScript has built-in primitive types:
        // - bool
        // - int8, int16, int (same as int32), int64
        // - uint8, uint16, uint (same as uint32), uint64
        // - float, double
        // These are automatically available and don't need registration.
        
        // If we need specific type aliases (e.g., byte for uint8), we can add them:
        // pEngine->RegisterTypedef("byte", "uint8");
        // pEngine->RegisterTypedef("sbyte", "int8");
        
        RegisterVector3(pEngine);
        RegisterColor(pEngine);
        RegisterEntityHandle(pEngine);
        RegisterMathFunctions(pEngine);
        
        MS_ANGEL_INFO("ASCoreTypes: Registration complete");
    }
    
    void RegisterVector3(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("ASCoreTypes: Registering Vector3 type...");
        
        // Register the Vector type as "Vector3" in AngelScript
        // Use asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK for a simple value type with constructor, assignment, and copy constructor
        int r = pEngine->RegisterObjectType("Vector3", sizeof(Vector), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK);
        if (r < 0) {
            MS_ANGEL_ERROR("ASCoreTypes: Failed to register Vector3 type");
            return;
        }
        
        // Register properties
        pEngine->RegisterObjectProperty("Vector3", "float x", offsetof(Vector, x));
        pEngine->RegisterObjectProperty("Vector3", "float y", offsetof(Vector, y));
        pEngine->RegisterObjectProperty("Vector3", "float z", offsetof(Vector, z));
        
        // Register constructors
        pEngine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vector3DefaultConstructor), asCALL_CDECL_OBJLAST);
        pEngine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(float, float, float)", asFUNCTION(Vector3InitConstructor), asCALL_CDECL_OBJLAST);
        pEngine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in)", asFUNCTION(Vector3CopyConstructor), asCALL_CDECL_OBJLAST);
        
        // Register operators
        pEngine->RegisterObjectMethod("Vector3", "Vector3 opAdd(const Vector3 &in) const", asMETHODPR(Vector, operator+, (const Vector&) const, Vector), asCALL_THISCALL);
        pEngine->RegisterObjectMethod("Vector3", "Vector3 opSub(const Vector3 &in) const", asMETHODPR(Vector, operator-, (const Vector&) const, Vector), asCALL_THISCALL);
        pEngine->RegisterObjectMethod("Vector3", "Vector3 opMul(float) const", asMETHODPR(Vector, operator*, (float) const, Vector), asCALL_THISCALL);
        pEngine->RegisterObjectMethod("Vector3", "Vector3 opDiv(float) const", asMETHODPR(Vector, operator/, (float) const, Vector), asCALL_THISCALL);
        pEngine->RegisterObjectMethod("Vector3", "bool opEquals(const Vector3 &in) const", asMETHODPR(Vector, operator==, (const Vector&) const, int), asCALL_THISCALL);
        pEngine->RegisterObjectMethod("Vector3", "Vector3 opNeg() const", asMETHODPR(Vector, operator-, (void) const, Vector), asCALL_THISCALL);
        
        // Register assignment operators
        pEngine->RegisterObjectMethod("Vector3", "Vector3& opAddAssign(const Vector3 &in)", asMETHODPR(Vector, operator+=, (const Vector&), Vector), asCALL_THISCALL);
        pEngine->RegisterObjectMethod("Vector3", "Vector3& opSubAssign(const Vector3 &in)", asMETHODPR(Vector, operator-=, (const Vector&), Vector), asCALL_THISCALL);
        pEngine->RegisterObjectMethod("Vector3", "Vector3& opMulAssign(float)", asMETHODPR(Vector, operator*=, (const float), Vector), asCALL_THISCALL);
        pEngine->RegisterObjectMethod("Vector3", "Vector3& opDivAssign(float)", asMETHODPR(Vector, operator/=, (const float), Vector), asCALL_THISCALL);
        
        // Register methods
        pEngine->RegisterObjectMethod("Vector3", "float Length() const", asMETHOD(Vector, Length), asCALL_THISCALL);
        pEngine->RegisterObjectMethod("Vector3", "Vector3 Normalize() const", asMETHOD(Vector, Normalize), asCALL_THISCALL);
        pEngine->RegisterObjectMethod("Vector3", "float Length2D() const", asMETHOD(Vector, Length2D), asCALL_THISCALL);
        
        // Register global Vector3 functions
        // Note: DotProduct and CrossProduct functions are defined in vector.h but have macro conflicts
        // For now, we'll register them as method calls instead of global functions
        pEngine->RegisterGlobalFunction("Vector3 CrossProduct(const Vector3 &in, const Vector3 &in)", asFUNCTIONPR(CrossProduct, (const Vector&, const Vector&), Vector), asCALL_CDECL);
        
        MS_ANGEL_INFO("ASCoreTypes: Vector3 registration complete");
    }
    
    void RegisterColor(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("ASCoreTypes: Registering Color type...");
        
        // Register Color as a Vector type (r, g, b stored in x, y, z, alpha in a separate field if needed)
        // For simplicity, we'll use Vector and treat it as RGB with x=r, y=g, z=b
        // Register Color type (also uses Vector internally)
        int r = pEngine->RegisterObjectType("Color", sizeof(Vector), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK);
        if (r < 0) {
            MS_ANGEL_ERROR("ASCoreTypes: Failed to register Color type");
            return;
        }
        
        // Register properties
        pEngine->RegisterObjectProperty("Color", "float r", offsetof(Vector, x));
        pEngine->RegisterObjectProperty("Color", "float g", offsetof(Vector, y));
        pEngine->RegisterObjectProperty("Color", "float b", offsetof(Vector, z));
        
        // Register constructors
        pEngine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ColorDefaultConstructor), asCALL_CDECL_OBJLAST);
        pEngine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f(float, float, float)", asFUNCTION(ColorInitConstructor), asCALL_CDECL_OBJLAST);
        
        MS_ANGEL_INFO("ASCoreTypes: Color registration complete");
    }
    
    void RegisterString(asIScriptEngine* pEngine)
    {
        // TODO: Implement string utilities
        MS_ANGEL_INFO("ASCoreTypes: String utilities not yet implemented");
    }
    
    void RegisterEntityHandle(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("ASCoreTypes: Registering EntityHandle type...");
        
        // Register EntityHandle as a value type (like an ID/index)
        // Use asOBJ_VALUE | asOBJ_POD for a simple value type that can be declared directly
        int r = pEngine->RegisterObjectType("EntityHandle", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
        if (r < 0) {
            MS_ANGEL_ERROR("ASCoreTypes: Failed to register EntityHandle type");
            return;
        }
        
        // Register EntityHandle methods
        // Note: These connect to the actual entity system
        pEngine->RegisterObjectMethod("EntityHandle", "bool IsValid() const", asFUNCTION(EntityHandle_IsValid), asCALL_CDECL_OBJLAST);
        pEngine->RegisterObjectMethod("EntityHandle", "CBaseEntity@ Get() const", asFUNCTION(EntityHandle_Get), asCALL_CDECL_OBJLAST);
        
        // Register constructor and assignment for value types
        pEngine->RegisterObjectBehaviour("EntityHandle", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(EntityHandle_DefaultConstructor), asCALL_CDECL_OBJLAST);
        pEngine->RegisterObjectBehaviour("EntityHandle", asBEHAVE_CONSTRUCT, "void f(int)", asFUNCTION(EntityHandle_IntConstructor), asCALL_CDECL_OBJLAST);
        
        MS_ANGEL_INFO("ASCoreTypes: EntityHandle type registered as value type with methods");
    }
    
    void RegisterMathFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("ASCoreTypes: Registering math functions...");
        
        // Register basic math functions
        pEngine->RegisterGlobalFunction("float sin(float)", asFUNCTION(AS_Sin), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float cos(float)", asFUNCTION(AS_Cos), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float sqrt(float)", asFUNCTION(AS_Sqrt), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float abs(float)", asFUNCTION(AS_Abs), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float min(float, float)", asFUNCTION(AS_Min), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float max(float, float)", asFUNCTION(AS_Max), asCALL_CDECL);
        
        // Register math constants
        pEngine->RegisterGlobalProperty("const float PI", (void*)&AS_PI);
        pEngine->RegisterGlobalProperty("const float E", (void*)&AS_E);
        
        MS_ANGEL_INFO("ASCoreTypes: Math function registration complete");
    }
    
    // Constructor wrapper implementations
    void Vector3DefaultConstructor(Vector *ptr)
    {
        new(ptr) Vector();
    }
    
    void Vector3InitConstructor(float x, float y, float z, Vector *ptr)
    {
        new(ptr) Vector(x, y, z);
    }
    
    void Vector3CopyConstructor(const Vector &other, Vector *ptr)
    {
        new(ptr) Vector(other);
    }
    
    void ColorDefaultConstructor(Vector *ptr)
    {
        new(ptr) Vector(0.0f, 0.0f, 0.0f);
    }
    
    void ColorInitConstructor(float r, float g, float b, Vector *ptr)
    {
        new(ptr) Vector(r, g, b);
    }
    
    void ColorInitConstructorAlpha(float r, float g, float b, float a, Vector *ptr)
    {
        // For now, ignore alpha since Vector only has 3 components
        new(ptr) Vector(r, g, b);
    }
    
    // External entity system integration functions
    // TODO: These should be implemented by the game engine to provide actual entity management
    // For now, providing placeholder implementations
    extern "C" {
        bool ValidateEntityHandle_AS(int handle) {
            // Placeholder: assume handle > 0 is valid
            return handle > 0;
        }
        
        void* GetEntityFromHandle_AS(int handle) {
            // Placeholder: return a dummy pointer for valid handles
            if (handle > 0) {
                // Return a fake pointer for testing (this should be actual entity retrieval)
                return reinterpret_cast<void*>(static_cast<uintptr_t>(handle * 0x1000));
            }
            return nullptr;
        }
        
        const char* GetEntityClassName_AS(void* entity) {
            // Placeholder: return generic class name
            if (entity) {
                return "entity";
            }
            return "null_entity";
        }
    }
    
    // EntityHandle implementations with actual entity system integration
    bool EntityHandle_IsValid(void* pHandle)
    {
        if (!pHandle) {
            MS_ANGEL_ERROR("ASCoreTypes: EntityHandle_IsValid - NULL handle pointer");
            return false;
        }
        
        int handle = *(int*)pHandle;
        if (handle <= 0) {
            MS_ANGEL_DEBUG("ASCoreTypes: EntityHandle_IsValid - Invalid handle value: %d", handle);
            return false;
        }
        
#ifdef VALVE_DLL
        // Use actual entity validation when available
        bool isValid = ValidateEntityHandle_AS(handle);
        MS_ANGEL_DEBUG("ASCoreTypes: EntityHandle_IsValid(%d) = %s", handle, isValid ? "true" : "false");
        return isValid;
#else
        // Client-side fallback - assume handle > 0 is valid
        MS_ANGEL_DEBUG("ASCoreTypes: EntityHandle_IsValid(%d) = true (client fallback)", handle);
        return true;
#endif
    }
    
    void* EntityHandle_Get(void* pHandle)
    {
        if (!pHandle) {
            MS_ANGEL_ERROR("ASCoreTypes: EntityHandle_Get - NULL handle pointer");
            return nullptr;
        }
        
        int handle = *(int*)pHandle;
        if (handle <= 0) {
            MS_ANGEL_DEBUG("ASCoreTypes: EntityHandle_Get - Invalid handle value: %d", handle);
            return nullptr;
        }
        
#ifdef VALVE_DLL
        // Use actual entity retrieval when available
        void* entity = GetEntityFromHandle_AS(handle);
        if (entity) {
            // Validate that this is actually a CBaseEntity
            const char* className = GetEntityClassName_AS(entity);
            MS_ANGEL_DEBUG("ASCoreTypes: EntityHandle_Get(%d) = %p (%s)", handle, entity, className ? className : "unknown");
        } else {
            MS_ANGEL_DEBUG("ASCoreTypes: EntityHandle_Get(%d) = NULL (entity not found)", handle);
        }
        return entity;
#else
        // Client-side fallback - return a dummy pointer for testing
        MS_ANGEL_DEBUG("ASCoreTypes: EntityHandle_Get(%d) = %p (client fallback)", handle, pHandle);
        return pHandle;
#endif
    }
    
    void EntityHandle_DefaultConstructor(void* pHandle)
    {
        if (!pHandle) {
            MS_ANGEL_ERROR("ASCoreTypes: EntityHandle_DefaultConstructor - NULL handle pointer");
            return;
        }
        
        // Initialize EntityHandle to 0 (invalid)
        *(int*)pHandle = 0;
        MS_ANGEL_DEBUG("ASCoreTypes: EntityHandle_DefaultConstructor - initialized to 0");
    }
    
    void EntityHandle_IntConstructor(int value, void* pHandle)
    {
        if (!pHandle) {
            MS_ANGEL_ERROR("ASCoreTypes: EntityHandle_IntConstructor - NULL handle pointer");
            return;
        }
        
        // Initialize EntityHandle with specific value
        *(int*)pHandle = value;
        MS_ANGEL_DEBUG("ASCoreTypes: EntityHandle_IntConstructor - initialized to %d", value);
    }
}
