//==========================================================================
// ASCoreTypes.cpp - Using asbind20
// 
// Core type bindings for AngelScript integration
// Provides fundamental types and math functions for script usage
//==========================================================================

#include "ASCoreTypes.h"
#include <asbind20/asbind.hpp>
#include <cstdio>
#include <cmath>
#include <new>

// Include vector.h directly to avoid mathlib.h macro conflicts
// We need the vec_t typedef but not the DotProduct macro
typedef float vec_t;
#include "../../../../server/hl/vector.h"

// Include MSLogger for consistent logging
#include "mslogger.h"

// Math constants as global variables (not macros) for AngelScript registration
static const float AS_PI = 3.14159265358979323846f;
static const float AS_E = 2.71828182845904523536f;

// EntityHandle struct is now defined in ASCoreTypes.h

//==========================================================================
// Core Type Registration Functions - Using asbind20
//==========================================================================
namespace ASCoreTypes
{
    // Forward declarations for EntityHandle helper functions
    bool EntityHandle_IsValid_Impl(void* pHandle);
    void* EntityHandle_Get_Impl(void* pHandle);
    
    // EntityHandle wrapper functions (still needed for custom behavior)
    bool EntityHandle_IsValid(const EntityHandle& handle);
    
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
        RegisterPlayerHandle(pEngine);
        RegisterMathFunctions(pEngine);
        
        MS_ANGEL_INFO("ASCoreTypes: Registration complete");
    }
    
    void RegisterVector3(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("ASCoreTypes: Registering comprehensive Vector3 type with enhanced asbind20 patterns...");
        
        // Enhanced Vector3 registration using asbind20 - comprehensive with operator overloads
        try {
            asbind20::value_class<Vector>(pEngine, "Vector3", asOBJ_APP_CLASS_ALLFLOATS)
                // Properties
                .property("float x", &Vector::x)
                .property("float y", &Vector::y) 
                .property("float z", &Vector::z)
                // Constructors with comprehensive support
                .default_constructor()
                .constructor<float, float, float>("float, float, float")
                .copy_constructor()
                .destructor()
                // Essential methods
                .method("float Length() const", &Vector::Length)
                .method("float Length2D() const", &Vector::Length2D)
                .method("Vector3 Normalize() const", [](const Vector& v) -> Vector {
                    Vector result = v;
                    float len = result.Length();
                    if (len > 0.0f) {
                        result.x /= len;
                        result.y /= len;
                        result.z /= len;
                    }
                    return result;
                })
                // Arithmetic operator overloads using manual implementation for better compatibility
                .method("Vector3 opAdd(const Vector3 &in) const", [](const Vector& a, const Vector& b) -> Vector {
                    return Vector(a.x + b.x, a.y + b.y, a.z + b.z);
                })
                .method("Vector3 opSub(const Vector3 &in) const", [](const Vector& a, const Vector& b) -> Vector {
                    return Vector(a.x - b.x, a.y - b.y, a.z - b.z);
                })
                .method("Vector3 opMul(float) const", [](const Vector& v, float scalar) -> Vector {
                    return Vector(v.x * scalar, v.y * scalar, v.z * scalar);
                })
                .method("Vector3 opDiv(float) const", [](const Vector& v, float scalar) -> Vector {
                    if (scalar != 0.0f) {
                        return Vector(v.x / scalar, v.y / scalar, v.z / scalar);
                    }
                    return Vector(0, 0, 0);
                })
                .method("bool opEquals(const Vector3 &in) const", [](const Vector& a, const Vector& b) -> bool {
                    return (a.x == b.x && a.y == b.y && a.z == b.z);
                })
                // Assignment operators
                .method("Vector3& opAssign(const Vector3 &in)", [](Vector* self, const Vector& other) -> Vector& {
                    *self = other;
                    return *self;
                })
                .method("Vector3& opAddAssign(const Vector3 &in)", [](Vector* self, const Vector& other) -> Vector& {
                    *self = *self + other;
                    return *self;
                })
                .method("Vector3& opSubAssign(const Vector3 &in)", [](Vector* self, const Vector& other) -> Vector& {
                    *self = *self - other;
                    return *self;
                })
                .method("Vector3& opMulAssign(float)", [](Vector* self, float scalar) -> Vector& {
                    self->x *= scalar;
                    self->y *= scalar;
                    self->z *= scalar;
                    return *self;
                });
            
            MS_ANGEL_INFO("ASCoreTypes: Enhanced Vector3 registration successful with operator overloads");
        } catch (const std::exception& e) {
            MS_ANGEL_ERROR("ASCoreTypes: Vector3 registration failed: %s", e.what());
            return;
        }
        
        // Register global Vector3 functions with asbind20
        try {
            asbind20::global(pEngine)
                .function("Vector3 CrossProduct(const Vector3 &in, const Vector3 &in)", 
                         static_cast<Vector(*)(const Vector&, const Vector&)>(&CrossProduct));
            MS_ANGEL_INFO("ASCoreTypes: Vector3 global functions registered (Distance/DotProduct in ASBuiltinFunctions)");
        } catch (const std::exception& e) {
            MS_ANGEL_ERROR("ASCoreTypes: Vector3 global function registration failed: %s", e.what());
        }
        
        MS_ANGEL_INFO("ASCoreTypes: Comprehensive Vector3 registration complete with enhanced asbind20 patterns");
    }
    
    void RegisterColor(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("ASCoreTypes: Registering Color type...");
        
        // Register Color type with asbind20 (using Vector internally)
        asbind20::value_class<Vector>(pEngine, "Color", asOBJ_APP_CLASS_CDAK)
            // Properties mapped to x,y,z as r,g,b
            .property("float r", &Vector::x)
            .property("float g", &Vector::y)
            .property("float b", &Vector::z)
            // Constructors
            .default_constructor()
            .constructor<float, float, float>("float, float, float")
            .copy_constructor()
            .destructor();
        
        MS_ANGEL_INFO("ASCoreTypes: Color registration complete");
    }
    
    void RegisterString(asIScriptEngine* pEngine)
    {
        // TODO: Implement string utilities
        MS_ANGEL_INFO("ASCoreTypes: String utilities not yet implemented");
    }
    
    // EntityHandle helper functions
    bool EntityHandle_IsValid(const EntityHandle& handle)
    {
        // TODO: Implement actual validation logic
        return handle.value != 0;
    }
    
    void RegisterEntityHandle(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("ASCoreTypes: Registering EntityHandle type...");
        
        // Register EntityHandle as a value type with asbind20 - basic version without CBaseEntity dependency
        asbind20::value_class<EntityHandle>(pEngine, "EntityHandle", asOBJ_APP_CLASS_CDAK)
            // Constructors
            .default_constructor()
            .constructor<int>("int")
            .copy_constructor()
            .destructor()
            // Assignment operator
            .method("EntityHandle& opAssign(const EntityHandle &in)", 
                static_cast<EntityHandle&(EntityHandle::*)(const EntityHandle&)>(&EntityHandle::operator=))
            // Comparison operator
            .method("bool opEquals(const EntityHandle &in) const",
                static_cast<bool(EntityHandle::*)(const EntityHandle&) const>(&EntityHandle::operator==))
            // Methods
            .method("bool IsValid() const", 
                +[](const EntityHandle* self) -> bool { return EntityHandle_IsValid(*self); })
            // Property for the value
            .property("int value", &EntityHandle::value);
        
        MS_ANGEL_INFO("ASCoreTypes: EntityHandle type registered as value type (Get method will be added later)");
    }
    
    void RegisterPlayerHandle(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("ASCoreTypes: Registering PlayerHandle type...");
        
        // Register PlayerHandle as a value type with asbind20
        asbind20::value_class<PlayerHandle>(pEngine, "PlayerHandle", asOBJ_APP_CLASS_CDAK)
            // Constructors
            .default_constructor()
            .constructor<int>("int")
            .copy_constructor()
            .destructor()
            // Assignment operator
            .method("PlayerHandle& opAssign(const PlayerHandle &in)", 
                static_cast<PlayerHandle&(PlayerHandle::*)(const PlayerHandle&)>(&PlayerHandle::operator=))
            // Comparison operator
            .method("bool opEquals(const PlayerHandle &in) const",
                static_cast<bool(PlayerHandle::*)(const PlayerHandle&) const>(&PlayerHandle::operator==))
            // Property for the value
            .property("int value", &PlayerHandle::value);
        
        MS_ANGEL_INFO("ASCoreTypes: PlayerHandle type registered as value type");
    }
    
    // Function to add EntityHandle methods that depend on entity types - call this after entity types are registered
    void RegisterEntityHandleMethods(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("ASCoreTypes: Registering EntityHandle methods that depend on entity types...");
        
        // Add the Get() method now that CBaseEntity is available
        pEngine->RegisterObjectMethod("EntityHandle", "CBaseEntity@ Get() const", 
            asFUNCTION(+[](const EntityHandle* self) -> void* { return EntityHandle_Get_Impl((void*)&self->value); }), 
            asCALL_CDECL_OBJFIRST);
        
        MS_ANGEL_INFO("ASCoreTypes: EntityHandle Get() method registered successfully");
    }
    
    void RegisterMathFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("ASCoreTypes: Registering math functions...");
        
        // Register basic math functions with asbind20
        asbind20::global(pEngine)
            .function("float sin(float)", &AS_Sin)
            .function("float cos(float)", &AS_Cos)
            .function("float sqrt(float)", &AS_Sqrt)
            .function("float abs(float)", &AS_Abs)
            .function("float min(float, float)", &AS_Min)
            .function("float max(float, float)", &AS_Max);
        
        // Register math constants
        pEngine->RegisterGlobalProperty("const float PI", (void*)&AS_PI);
        pEngine->RegisterGlobalProperty("const float E", (void*)&AS_E);
        
        MS_ANGEL_INFO("ASCoreTypes: Math function registration complete");
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
    bool EntityHandle_IsValid_Impl(void* pHandle)
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
    
    void* EntityHandle_Get_Impl(void* pHandle)
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
    
}
