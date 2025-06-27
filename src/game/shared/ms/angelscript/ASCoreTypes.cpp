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
        
        printf("ASCoreTypes: Registering core types and math functions...\n");
        
        RegisterVector3(pEngine);
        RegisterColor(pEngine);
        RegisterEntityHandle(pEngine);
        RegisterMathFunctions(pEngine);
        
        printf("ASCoreTypes: Registration complete\n");
    }
    
    void RegisterVector3(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        printf("ASCoreTypes: Registering Vector3 type...\n");
        
        // Register the Vector type as "Vector3" in AngelScript
        // Use asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK for a simple value type with constructor, assignment, and copy constructor
        int r = pEngine->RegisterObjectType("Vector3", sizeof(Vector), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK);
        if (r < 0) {
            printf("ASCoreTypes: Failed to register Vector3 type\n");
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
        
        printf("ASCoreTypes: Vector3 registration complete\n");
    }
    
    void RegisterColor(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        printf("ASCoreTypes: Registering Color type...\n");
        
        // Register Color as a Vector type (r, g, b stored in x, y, z, alpha in a separate field if needed)
        // For simplicity, we'll use Vector and treat it as RGB with x=r, y=g, z=b
        // Register Color type (also uses Vector internally)
        int r = pEngine->RegisterObjectType("Color", sizeof(Vector), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK);
        if (r < 0) {
            printf("ASCoreTypes: Failed to register Color type\n");
            return;
        }
        
        // Register properties
        pEngine->RegisterObjectProperty("Color", "float r", offsetof(Vector, x));
        pEngine->RegisterObjectProperty("Color", "float g", offsetof(Vector, y));
        pEngine->RegisterObjectProperty("Color", "float b", offsetof(Vector, z));
        
        // Register constructors
        pEngine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ColorDefaultConstructor), asCALL_CDECL_OBJLAST);
        pEngine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f(float, float, float)", asFUNCTION(ColorInitConstructor), asCALL_CDECL_OBJLAST);
        
        printf("ASCoreTypes: Color registration complete\n");
    }
    
    void RegisterString(asIScriptEngine* pEngine)
    {
        // TODO: Implement string utilities
        printf("ASCoreTypes: String utilities not yet implemented\n");
    }
    
    void RegisterEntityHandle(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        printf("ASCoreTypes: Registering EntityHandle type...\n");
        
        // Register EntityHandle as a reference type (handle to an entity)
        int r = pEngine->RegisterObjectType("EntityHandle", 0, asOBJ_REF | asOBJ_NOCOUNT);
        if (r < 0) {
            printf("ASCoreTypes: Failed to register EntityHandle type\n");
            return;
        }
        
        printf("ASCoreTypes: EntityHandle type registered\n");
    }
    
    void RegisterMathFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        printf("ASCoreTypes: Registering math functions...\n");
        
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
        
        printf("ASCoreTypes: Math function registration complete\n");
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
}
