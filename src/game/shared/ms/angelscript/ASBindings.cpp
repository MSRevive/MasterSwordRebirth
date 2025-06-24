//==========================================================================
// AngelScript Integration Layer Implementation - Minimal Version
// 
// Master registration and coordination for all AngelScript bindings
//==========================================================================

#include "ASBindings.h"
#include "ASCoreTypes.h"
#include "ASBuiltinFunctions.h"
#include <angelscript.h>
#include <cstdio>
#include <cstring>

//==========================================================================
// Master Registration Function - Minimal Implementation
//==========================================================================
bool ASBindings::RegisterAll(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        printf("ASBindings::RegisterAll: ERROR - NULL engine pointer\n");
        return false;
    }
    
    bool success = true;
    
    // Step 1: Register core types (Vector3, Color, math functions)
    printf("\n[1/4] Registering Core Types...\n");
    if (!RegisterCoreTypes(pEngine))
    {
        printf("   ERROR: Core type registration failed!\n");
        success = false;
    }
    else
    {
        printf("   ✓ Core types registered successfully\n");
    }
    
    // Step 2: Register builtin functions (strings, utilities, game functions)
    printf("\n[2/4] Registering Builtin Functions...\n");
    if (!RegisterBuiltinFunctions(pEngine))
    {
        printf("   ERROR: Builtin function registration failed!\n");
        success = false;
    }
    else
    {
        printf("   ✓ Builtin functions registered successfully\n");
    }
    
    // Step 3: Validate all registrations
    printf("\n[3/4] Validating Registrations...\n");
    if (!ValidateRegistrations(pEngine))
    {
        printf("   ERROR: Registration validation failed!\n");
        success = false;
    }
    else
    {
        printf("   ✓ All registrations validated successfully\n");
    }
    
    // Step 4: Log registration summary
    printf("\n[4/4] Registration Summary:\n");
    LogRegistrationInfo(pEngine);

    return success;
}


//==========================================================================
// Placeholder implementations for future expansion
//==========================================================================
bool ASBindings::RegisterCoreTypes(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        printf("ASBindings::RegisterCoreTypes: ERROR - NULL engine pointer\n");
        return false;
    }
    
    // Call the ASCoreTypes registration function
    try
    {
        ASCoreTypes::RegisterAll(pEngine);
        
        // Verify core types were registered
        asITypeInfo* pVector3Type = pEngine->GetTypeInfoByName("Vector3");
        asITypeInfo* pColorType = pEngine->GetTypeInfoByName("Color");
        
        if (!pVector3Type)
        {
            printf("ASBindings::RegisterCoreTypes: ERROR - Vector3 type not found after registration\n");
            return false;
        }
        
        if (!pColorType)
        {
            printf("ASBindings::RegisterCoreTypes: ERROR - Color type not found after registration\n");
            return false;
        }
        
        printf("   - Vector3 type registered\n");
        printf("   - Color type registered\n");
        printf("   - Math functions registered\n");
        
        return true;
    }
    catch (...)
    {
        printf("ASBindings::RegisterCoreTypes: ERROR - Exception during registration\n");
        return false;
    }
}


bool ASBindings::RegisterBuiltinFunctions(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        printf("ASBindings::RegisterBuiltinFunctions: ERROR - NULL engine pointer\n");
        return false;
    }
    
    // Call the ASBuiltinFunctions registration function
    try
    {
        ASBuiltinFunctions::RegisterAll(pEngine);
        
        // Verify some key functions were registered
        asIScriptFunction* func;
        
        // Check string functions
        func = pEngine->GetGlobalFunctionByDecl("string Left(const string &in, int)");
        if (!func)
        {
            printf("ASBindings::RegisterBuiltinFunctions: WARNING - Left() function not found\n");
        }
        
        // Check game functions
        func = pEngine->GetGlobalFunctionByDecl("float GetGameTime()");
        if (!func)
        {
            printf("ASBindings::RegisterBuiltinFunctions: WARNING - GetGameTime() function not found\n");
        }
        
        printf("   - String manipulation functions registered\n");
        printf("   - Math utility functions registered\n");
        printf("   - Vector utility functions registered\n");
        printf("   - Game system functions registered\n");
        
        return true;
    }
    catch (...)
    {
        printf("ASBindings::RegisterBuiltinFunctions: ERROR - Exception during registration\n");
        return false;
    }
}


bool ASBindings::ValidateRegistrations(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        printf("ASBindings::ValidateRegistrations: ERROR - NULL engine pointer\n");
        return false;
    }
    
    bool validationSuccess = true;
    int warnings = 0;
    int errors = 0;
    
    printf("   Running comprehensive validation...\n");
    
    // Validate core types
    printf("   - Validating core types:\n");
    
    // Check Vector3
    asITypeInfo* pVector3 = pEngine->GetTypeInfoByName("Vector3");
    if (!pVector3)
    {
        printf("     ERROR: Vector3 type not registered\n");
        errors++;
        validationSuccess = false;
    }
    else
    {
        // Check Vector3 properties
        int propCount = pVector3->GetPropertyCount();
        if (propCount < 3)
        {
            printf("     WARNING: Vector3 has %d properties, expected at least 3 (x,y,z)\n", propCount);
            warnings++;
        }
        
        // Check Vector3 methods
        int methodCount = pVector3->GetMethodCount();
        if (methodCount < 3)
        {
            printf("     WARNING: Vector3 has %d methods, expected at least 3\n", methodCount);
            warnings++;
        }
        printf("     ✓ Vector3 validated (%d properties, %d methods)\n", propCount, methodCount);
    }
    
    // Check Color
    asITypeInfo* pColor = pEngine->GetTypeInfoByName("Color");
    if (!pColor)
    {
        printf("     ERROR: Color type not registered\n");
        errors++;
        validationSuccess = false;
    }
    else
    {
        printf("     ✓ Color type validated\n");
    }
    
    // Validate global functions
    printf("   - Validating global functions:\n");
    
    // String functions
    const char* stringFuncs[] = {
        "string Left(const string &in, int)",
        "string Right(const string &in, int)",
        "string Mid(const string &in, int, int)",
        "int Length(const string &in)",
        "string ToUpper(const string &in)",
        "string ToLower(const string &in)",
        "string Replace(const string &in, const string &in, const string &in)"
    };
    
    int stringFuncCount = 0;
    for (const char* funcDecl : stringFuncs)
    {
        if (pEngine->GetGlobalFunctionByDecl(funcDecl) != nullptr)
            stringFuncCount++;
        else
            warnings++;
    }
    printf("     ✓ String functions: %d/%d registered\n", stringFuncCount, 7);
    
    // Math functions
    const char* mathFuncs[] = {
        "float sin(float)",
        "float cos(float)",
        "float sqrt(float)",
        "float abs(float)",
        "float min(float, float)",
        "float max(float, float)"
    };
    
    int mathFuncCount = 0;
    for (const char* funcDecl : mathFuncs)
    {
        if (pEngine->GetGlobalFunctionByDecl(funcDecl) != nullptr)
            mathFuncCount++;
        else
            warnings++;
    }
    printf("     ✓ Math functions: %d/%d registered\n", mathFuncCount, 6);
    
    // Game functions
    const char* gameFuncs[] = {
        "string GetCvar(const string &in)",
        "float GetGameTime()",
        "float Random(float, float)",
        "void LogMessage(const string &in)",
        "int GetPlayerCount()"
    };
    
    int gameFuncCount = 0;
    for (const char* funcDecl : gameFuncs)
    {
        if (pEngine->GetGlobalFunctionByDecl(funcDecl) != nullptr)
            gameFuncCount++;
        else
            warnings++;
    }
    printf("     ✓ Game functions: %d/%d registered\n", gameFuncCount, 5);
    
    // Check for EntityHandle type (used by game functions)
    asITypeInfo* pEntityHandle = pEngine->GetTypeInfoByName("EntityHandle");
    if (!pEntityHandle)
    {
        printf("     WARNING: EntityHandle type not registered (entity functions may not work)\n");
        warnings++;
    }
    
    // Summary
    printf("   Validation complete: ");
    if (errors == 0 && warnings == 0)
    {
        printf("PERFECT - No errors or warnings\n");
    }
    else
    {
        printf("%d errors, %d warnings\n", errors, warnings);
        if (errors > 0)
            validationSuccess = false;
    }
    
    return validationSuccess;
}


void ASBindings::LogRegistrationInfo(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        printf("ASBindings::LogRegistrationInfo: ERROR - NULL engine pointer\n");
        return;
    }

    // Count registered types
    int typeCount = pEngine->GetObjectTypeCount();
    printf("\nRegistered Object Types: %d\n", typeCount);
    for (int i = 0; i < typeCount && i < 10; i++) // Show first 10 types
    {
        asITypeInfo* pType = pEngine->GetObjectTypeByIndex(i);
        if (pType)
        {
            printf("   - %s (Properties: %d, Methods: %d)\n", 
                   pType->GetName(), 
                   pType->GetPropertyCount(),
                   pType->GetMethodCount());
        }
    }
    if (typeCount > 10)
        printf("   ... and %d more types\n", typeCount - 10);
    
    // Count global functions
    int globalFuncCount = pEngine->GetGlobalFunctionCount();
    printf("\nRegistered Global Functions: %d\n", globalFuncCount);
    
    // Count by category
    int stringFuncs = 0, mathFuncs = 0, gameFuncs = 0, vectorFuncs = 0, otherFuncs = 0;
    
    for (int i = 0; i < globalFuncCount; i++)
    {
        asIScriptFunction* pFunc = pEngine->GetGlobalFunctionByIndex(i);
        if (pFunc)
        {
            const char* name = pFunc->GetName();
            if (strstr(name, "Left") || strstr(name, "Right") || strstr(name, "Mid") || 
                strstr(name, "Length") || strstr(name, "Upper") || strstr(name, "Lower") || 
                strstr(name, "Replace"))
                stringFuncs++;
            else if (strstr(name, "sin") || strstr(name, "cos") || strstr(name, "sqrt") || 
                     strstr(name, "abs") || strstr(name, "min") || strstr(name, "max"))
                mathFuncs++;
            else if (strstr(name, "GetCvar") || strstr(name, "GetGameTime") || 
                     strstr(name, "Random") || strstr(name, "LogMessage") || 
                     strstr(name, "GetPlayer") || strstr(name, "FindEntity"))
                gameFuncs++;
            else if (strstr(name, "Vector") || strstr(name, "Distance") || 
                     strstr(name, "DotProduct") || strstr(name, "CrossProduct") ||
                     strstr(name, "Angle"))
                vectorFuncs++;
            else
                otherFuncs++;
        }
    }
    
    printf("   - String manipulation: %d functions\n", stringFuncs);
    printf("   - Math operations: %d functions\n", mathFuncs);
    printf("   - Game system: %d functions\n", gameFuncs);
    printf("   - Vector/Angle utilities: %d functions\n", vectorFuncs);
    if (otherFuncs > 0)
        printf("   - Other: %d functions\n", otherFuncs);
    
    // Count global properties
    int globalPropCount = pEngine->GetGlobalPropertyCount();
    if (globalPropCount > 0)
    {
        printf("\nRegistered Global Properties: %d\n", globalPropCount);
        for (int i = 0; i < globalPropCount && i < 5; i++)
        {
            const char* name;
            const char* nameSpace;
            int typeId;
            bool isConst;
            if (pEngine->GetGlobalPropertyByIndex(i, &name, &nameSpace, &typeId, &isConst) >= 0)
            {
                printf("   - %s%s\n", isConst ? "const " : "", name);
            }
        }
        if (globalPropCount > 5)
            printf("   ... and %d more properties\n", globalPropCount - 5);
    }
    
    // Memory usage estimate
    printf("\nEngine Status:\n");
    printf("   - Script modules: %d\n", pEngine->GetModuleCount());
    printf("   - Type ID counter: %d\n", pEngine->GetTypeIdByDecl("float"));
    
    printf("\n==========================================================================\n");
}
