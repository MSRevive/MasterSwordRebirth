//==========================================================================
// ASCoreTypes.cpp - Using asbind20
// 
// Core type bindings for AngelScript integration
// Provides fundamental types and math functions for script usage
//==========================================================================

#include "ASCoreTypes.h"
#include "ASScriptContext.h"
#include "ASBindExtensions.h"
#include <asbind20/asbind.hpp>
#include <cstdio>
#include <cmath>
#include <new>

// Include vector.h directly to avoid mathlib.h macro conflicts
// We need the vec_t typedef but not the DotProduct macro
typedef float vec_t;
#include "hl/vector.h"

// Include MSLogger for consistent logging
#include "mslogger.h"

// Math constants as global variables (not macros) for AngelScript registration
static const float AS_PI = 3.14159265358979323846f;
static const float AS_E = 2.71828182845904523536f;

//==========================================================================
// Core Type Registration Functions - Using asbind20
//==========================================================================
namespace ASCoreTypes
{
    
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
    
    void RegisterMathFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("ASCoreTypes: Registering math functions...");
        
        // Register basic math functions with context-aware asbind20
        using namespace asbind20::ctx;
        asbind20::global_ctx(pEngine)
            .function("float sin(float)", &AS_Sin, shared)
            .function("float cos(float)", &AS_Cos, shared)
            .function("float sqrt(float)", &AS_Sqrt, shared)
            .function("float abs(float)", &AS_Abs, shared)
            .function("float min(float, float)", &AS_Min, shared)
            .function("float max(float, float)", &AS_Max, shared);
        
        // Register math constants
        pEngine->RegisterGlobalProperty("const float PI", (void*)&AS_PI);
        pEngine->RegisterGlobalProperty("const float E", (void*)&AS_E);
        
        MS_ANGEL_INFO("ASCoreTypes: Math function registration complete");
    }
    
}
