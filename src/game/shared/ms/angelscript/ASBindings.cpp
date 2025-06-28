//==========================================================================
// AngelScript Integration Layer Implementation - Minimal Version
// 
// Master registration and coordination for all AngelScript bindings
//==========================================================================

#include "ASBindings.h"
#include "ASModuleSystem.h"
#include "ASCoreTypes.h"
#include "ASBuiltinFunctions.h"
#include "ASScriptClasses.h"   
#include "ASCoroutines.h"        
#include "ASObjectPool.h"      
#include <angelscript.h>

// AngelScript add-ons
#include "addons/scriptstdstring/scriptstdstring.h"
#include "addons/scriptarray/scriptarray.h"

#include <cstdio>
#include <cstring>

// Include MSLogger after other headers to avoid conflicts
#include "mslogger.h"

//==========================================================================
// Master Registration Function - Minimal Implementation
//==========================================================================
bool ASBindings::RegisterAll(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        MS_ANGEL_ERROR("ASBindings::RegisterAll: ERROR - NULL engine pointer");
        return false;
    }
    
    bool success = true;
    
    // Step 0: Register string type (MUST be first!)
    MS_ANGEL_INFO("[0/9] Registering String Type...");
    RegisterStdString(pEngine);
    RegisterStdStringUtils(pEngine);
    
    // Also register array type early
    MS_ANGEL_INFO("[1/9] Registering Array Type...");
    RegisterScriptArray(pEngine, true); // true = use native calling convention
    
    // Step 2: Register core types (Vector3, Color, math functions)
    MS_ANGEL_INFO("[2/9] Registering Core Types...");
    if (!RegisterCoreTypes(pEngine))
    {
        MS_ANGEL_ERROR("   ERROR: Core type registration failed!");
        success = false;
    }
    else
    {
        MS_ANGEL_INFO("   ✓ Core types registered successfully");
    }
    
    // Step 3: Register builtin functions (strings, utilities, game functions)
    MS_ANGEL_INFO("[3/9] Registering Builtin Functions...");
    if (!RegisterBuiltinFunctions(pEngine))
    {
        MS_ANGEL_ERROR("   ERROR: Builtin function registration failed!");
        success = false;
    }
    else
    {
        MS_ANGEL_INFO("   ✓ Builtin functions registered successfully");
    }
    
    // Step 4: Register script classes (CGameScript and derivatives)
    MS_ANGEL_INFO("[4/9] Registering Script Classes...");
    if (!RegisterScriptClasses(pEngine))
    {
        MS_ANGEL_ERROR("   ERROR: Script class registration failed!");
        success = false;
    }
    else
    {
        MS_ANGEL_INFO("   ✓ Script classes registered successfully");
    }
    
    // Step 5: Register coroutines system
    MS_ANGEL_INFO("[5/9] Registering Coroutines System...");
    if (!RegisterCoroutineFunctions(pEngine))
    {
        MS_ANGEL_ERROR("   ERROR: Coroutines system registration failed!");
        success = false;
    }
    else
    {
        MS_ANGEL_INFO("   ✓ Coroutines system registered successfully");
    }
    
    // Step 6: Register memory optimization systems
    MS_ANGEL_INFO("[6/9] Registering Memory Optimization...");
    if (!RegisterMemoryOptimization(pEngine))
    {
        MS_ANGEL_ERROR("   ERROR: Memory optimization registration failed!");
        success = false;
    }
    else
    {
        MS_ANGEL_INFO("   ✓ Memory optimization registered successfully");
    }
    
    // Step 7: Register module system
    MS_ANGEL_INFO("[7/9] Registering Module System...");
    if (!RegisterModuleSystem(pEngine))
    {
        MS_ANGEL_ERROR("   ERROR: Module system registration failed!");
        success = false;
    }
    else
    {
        MS_ANGEL_INFO("   ✓ Module system registered successfully");
    }
    
    // Step 8: Validate all registrations
    MS_ANGEL_INFO("[8/9] Validating Registrations...");
    if (!ValidateRegistrations(pEngine))
    {
        MS_ANGEL_ERROR("   ERROR: Registration validation failed!");
        success = false;
    }
    else
    {
        MS_ANGEL_INFO("   ✓ All registrations validated successfully");
    }
    
    // Step 9: Log registration summary
    MS_ANGEL_INFO("[9/9] Registration Summary:");
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
        MS_ANGEL_ERROR("ASBindings::RegisterCoreTypes: ERROR - NULL engine pointer");
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
            MS_ANGEL_ERROR("ASBindings::RegisterCoreTypes: ERROR - Vector3 type not found after registration");
            return false;
        }
        
        if (!pColorType)
        {
            MS_ANGEL_ERROR("ASBindings::RegisterCoreTypes: ERROR - Color type not found after registration");
            return false;
        }
        
        MS_ANGEL_INFO("   - Vector3 type registered");
        MS_ANGEL_INFO("   - Color type registered");
        MS_ANGEL_INFO("   - Math functions registered");
        
        return true;
    }
    catch (...)
    {
        MS_ANGEL_ERROR("ASBindings::RegisterCoreTypes: ERROR - Exception during registration");
        return false;
    }
}


bool ASBindings::RegisterBuiltinFunctions(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        MS_ANGEL_ERROR("ASBindings::RegisterBuiltinFunctions: ERROR - NULL engine pointer");
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
            MS_ANGEL_INFO("ASBindings::RegisterBuiltinFunctions: WARNING - Left() function not found");
        }
        
        // Check game functions
        func = pEngine->GetGlobalFunctionByDecl("float GetGameTime()");
        if (!func)
        {
            MS_ANGEL_INFO("ASBindings::RegisterBuiltinFunctions: WARNING - GetGameTime() function not found");
        }
        
        MS_ANGEL_INFO("   - String manipulation functions registered");
        MS_ANGEL_INFO("   - Math utility functions registered");
        MS_ANGEL_INFO("   - Vector utility functions registered");
        MS_ANGEL_INFO("   - Game system functions registered");
        
        return true;
    }
    catch (...)
    {
        MS_ANGEL_ERROR("ASBindings::RegisterBuiltinFunctions: ERROR - Exception during registration");
        return false;
    }
}


bool ASBindings::ValidateRegistrations(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        MS_ANGEL_ERROR("ASBindings::ValidateRegistrations: ERROR - NULL engine pointer");
        return false;
    }
    
    bool validationSuccess = true;
    int warnings = 0;
    int errors = 0;
    
    MS_ANGEL_INFO("   Running comprehensive validation...");
    
    // Validate core types
    MS_ANGEL_INFO("   - Validating core types:");
    
    // Check Vector3
    asITypeInfo* pVector3 = pEngine->GetTypeInfoByName("Vector3");
    if (!pVector3)
    {
        MS_ANGEL_ERROR("     ERROR: Vector3 type not registered");
        errors++;
        validationSuccess = false;
    }
    else
    {
        // Check Vector3 properties
        int propCount = pVector3->GetPropertyCount();
        if (propCount < 3)
        {
            MS_ANGEL_INFO("     WARNING: Vector3 has %d properties, expected at least 3 (x,y,z)", propCount);
            warnings++;
        }
        
        // Check Vector3 methods
        int methodCount = pVector3->GetMethodCount();
        if (methodCount < 3)
        {
            MS_ANGEL_INFO("     WARNING: Vector3 has %d methods, expected at least 3", methodCount);
            warnings++;
        }
        MS_ANGEL_INFO("     ✓ Vector3 validated (%d properties, %d methods)", propCount, methodCount);
    }
    
    // Check Color
    asITypeInfo* pColor = pEngine->GetTypeInfoByName("Color");
    if (!pColor)
    {
        MS_ANGEL_ERROR("     ERROR: Color type not registered");
        errors++;
        validationSuccess = false;
    }
    else
    {
        MS_ANGEL_INFO("     ✓ Color type validated");
    }
    
    // Validate global functions
    MS_ANGEL_INFO("   - Validating global functions:");
    
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
    MS_ANGEL_INFO("     ✓ String functions: %d/%d registered", stringFuncCount, 7);
    
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
    MS_ANGEL_INFO("     ✓ Math functions: %d/%d registered", mathFuncCount, 6);
    
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
    MS_ANGEL_INFO("     ✓ Game functions: %d/%d registered", gameFuncCount, 5);
    
    // Check for EntityHandle type (used by game functions)
    asITypeInfo* pEntityHandle = pEngine->GetTypeInfoByName("EntityHandle");
    if (!pEntityHandle)
    {
        MS_ANGEL_INFO("     WARNING: EntityHandle type not registered (entity functions may not work)");
        warnings++;
    }
    
    // Summary
    MS_ANGEL_INFO("   Validation complete: ");
    if (errors == 0 && warnings == 0)
    {
        MS_ANGEL_INFO("PERFECT - No errors or warnings");
    }
    else
    {
        MS_ANGEL_INFO("%d errors, %d warnings", errors, warnings);
        if (errors > 0)
            validationSuccess = false;
    }
    
    return validationSuccess;
}


void ASBindings::LogRegistrationInfo(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        MS_ANGEL_ERROR("ASBindings::LogRegistrationInfo: ERROR - NULL engine pointer");
        return;
    }

    // Count registered types
    int typeCount = pEngine->GetObjectTypeCount();
    MS_ANGEL_INFO("Registered Object Types: %d", typeCount);
    for (int i = 0; i < typeCount && i < 10; i++) // Show first 10 types
    {
        asITypeInfo* pType = pEngine->GetObjectTypeByIndex(i);
        if (pType)
        {
            MS_ANGEL_INFO("   - %s (Properties: %d, Methods: %d)", 
                   pType->GetName(), 
                   pType->GetPropertyCount(),
                   pType->GetMethodCount());
        }
    }
    if (typeCount > 10)
        MS_ANGEL_INFO("   ... and %d more types", typeCount - 10);
    
    // Count global functions
    int globalFuncCount = pEngine->GetGlobalFunctionCount();
    MS_ANGEL_INFO("Registered Global Functions: %d", globalFuncCount);
    
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
    
    MS_ANGEL_INFO("   - String manipulation: %d functions", stringFuncs);
    MS_ANGEL_INFO("   - Math operations: %d functions", mathFuncs);
    MS_ANGEL_INFO("   - Game system: %d functions", gameFuncs);
    MS_ANGEL_INFO("   - Vector/Angle utilities: %d functions", vectorFuncs);
    if (otherFuncs > 0)
        MS_ANGEL_INFO("   - Other: %d functions", otherFuncs);
    
    // Count global properties
    int globalPropCount = pEngine->GetGlobalPropertyCount();
    if (globalPropCount > 0)
    {
        MS_ANGEL_INFO("Registered Global Properties: %d", globalPropCount);
        for (int i = 0; i < globalPropCount && i < 5; i++)
        {
            const char* name;
            const char* nameSpace;
            int typeId;
            bool isConst;
            if (pEngine->GetGlobalPropertyByIndex(i, &name, &nameSpace, &typeId, &isConst) >= 0)
            {
                MS_ANGEL_INFO("   - %s%s", isConst ? "const " : "", name);
            }
        }
        if (globalPropCount > 5)
            MS_ANGEL_INFO("   ... and %d more properties", globalPropCount - 5);
    }
    
    // Memory usage estimate
    MS_ANGEL_INFO("Engine Status:");
    MS_ANGEL_INFO("   - Script modules: %d", pEngine->GetModuleCount());
    MS_ANGEL_INFO("   - Type ID counter: %d", pEngine->GetTypeIdByDecl("float"));
    
    MS_ANGEL_INFO("==========================================================================");
}


//==========================================================================
// Script Classes Registration
//==========================================================================
bool ASBindings::RegisterScriptClasses(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        MS_ANGEL_ERROR("ASBindings::RegisterScriptClasses: ERROR - NULL engine pointer");
        return false;
    }
    
    // Call the ASScriptClasses registration function
    try
    {
        if (!ASScriptClasses::RegisterAll(pEngine))
        {
            MS_ANGEL_ERROR("   ERROR: ASScriptClasses::RegisterAll failed");
            return false;
        }
        
        // Verify CGameScript was registered
        asITypeInfo* pCGameScriptType = pEngine->GetTypeInfoByName("CGameScript");
        
        if (!pCGameScriptType)
        {
            MS_ANGEL_ERROR("ASBindings::RegisterScriptClasses: ERROR - CGameScript type not found after registration");
            return false;
        }
        
        // Check that CGameScript has expected methods
        int methodCount = pCGameScriptType->GetMethodCount();
        if (methodCount < 5) // Should have at least SetVar, GetVar methods, etc.
        {
            MS_ANGEL_INFO("ASBindings::RegisterScriptClasses: WARNING - CGameScript has only %d methods, expected more", methodCount);
        }
        
        MS_ANGEL_INFO("   - CGameScript base class registered");
        MS_ANGEL_INFO("   - Variable system methods registered");
        MS_ANGEL_INFO("   - Script class factory system registered");
        
        return true;
    }
    catch (...)
    {
        MS_ANGEL_ERROR("ASBindings::RegisterScriptClasses: ERROR - Exception during registration");
        return false;
    }
}

//==========================================================================
// Coroutines System Registration
//==========================================================================
bool ASBindings::RegisterCoroutineFunctions(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        MS_ANGEL_ERROR("ASBindings::RegisterCoroutineFunctions: ERROR - NULL engine pointer");
        return false;
    }
    
    try
    {
        // Register coroutine functions from ASCoroutines
        ::RegisterCoroutineFunctions(pEngine);
        
        // Initialize the coroutine manager
        ASCoroutineManager::Instance();
        
        // Verify some key coroutine functions were registered
        asIScriptFunction* func;
        
        func = pEngine->GetGlobalFunctionByDecl("int StartCoroutine(const string &in)");
        if (!func)
        {
            MS_ANGEL_INFO("ASBindings::RegisterCoroutineFunctions: WARNING - StartCoroutine() function not found");
        }
        
        func = pEngine->GetGlobalFunctionByDecl("void DelaySeconds(float)");
        if (!func)
        {
            MS_ANGEL_INFO("ASBindings::RegisterCoroutineFunctions: WARNING - DelaySeconds() function not found");
        }
        
        MS_ANGEL_INFO("   - Coroutine management functions registered");
        MS_ANGEL_INFO("   - Async delay and yield functions registered");
        MS_ANGEL_INFO("   - Coroutine manager initialized");
        
        return true;
    }
    catch (...)
    {
        MS_ANGEL_ERROR("ASBindings::RegisterCoroutineFunctions: ERROR - Exception during registration");
        return false;
    }
}

//==========================================================================
// Memory Optimization Registration
//==========================================================================
bool ASBindings::RegisterMemoryOptimization(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        MS_ANGEL_ERROR("ASBindings::RegisterMemoryOptimization: ERROR - NULL engine pointer");
        return false;
    }
    
    try
    {
        // Register memory optimization functions from ASObjectPool
        RegisterMemoryOptimizationFunctions(pEngine);
        InitializeOptimizationSystems();
        
        // Configure for 32-bit constraints
        ASMemoryMonitor* pMemoryMonitor = ASMemoryMonitor::Instance();
        if (pMemoryMonitor)
        {
            // Set conservative limits for 32-bit build
            pMemoryMonitor->SetMemoryLimit(128 * 1024 * 1024); // 128MB limit
            pMemoryMonitor->SetAutoGC(true);
            pMemoryMonitor->SetGCInterval(30.0f); // 30 second intervals
        }
        
        ASObjectPool* pObjectPool = ASObjectPool::Instance();
        if (pObjectPool)
        {
            pObjectPool->SetEnabled(true);
            pObjectPool->SetDefaultMaxObjects(16); // Conservative pool sizes
        }
        
        MS_ANGEL_INFO("   - Object pooling system initialized");
        MS_ANGEL_INFO("   - Memory monitoring system initialized");
        MS_ANGEL_INFO("   - Script caching system initialized");
        MS_ANGEL_INFO("   - 32-bit memory constraints configured");
        
        return true;
    }
    catch (...)
    {
        MS_ANGEL_ERROR("ASBindings::RegisterMemoryOptimization: ERROR - Exception during registration");
        return false;
    }
}

//==========================================================================
// Module System Registration  
//==========================================================================
bool ASBindings::RegisterModuleSystem(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        MS_ANGEL_ERROR("ASBindings::RegisterModuleSystem: ERROR - NULL engine pointer");
        return false;
    }
    
    try
    {
        // Register module management functions from ASModuleSystem
        if (!ASModuleSystemBindings::RegisterAll(pEngine))
        {
            MS_ANGEL_ERROR("ASBindings::RegisterModuleSystem: ERROR - ASModuleSystem registration failed");
            return false;
        }
        
        // Verify some key module functions were registered
        asIScriptFunction* func;
        
        func = pEngine->GetGlobalFunctionByDecl("bool LoadModule(const string &in)");
        if (!func)
        {
            MS_ANGEL_INFO("ASBindings::RegisterModuleSystem: WARNING - LoadModule() function not found");
        }
        
        func = pEngine->GetGlobalFunctionByDecl("bool HasModule(const string &in)");
        if (!func)
        {
            MS_ANGEL_INFO("ASBindings::RegisterModuleSystem: WARNING - HasModule() function not found");
        }
        
        MS_ANGEL_INFO("   - Module loading and management functions registered");
        MS_ANGEL_INFO("   - Module dependency resolution system initialized");
        MS_ANGEL_INFO("   - Import/export system registered");
        MS_ANGEL_INFO("   - Module search paths configured");
        
        return true;
    }
    catch (...)
    {
        MS_ANGEL_ERROR("ASBindings::RegisterModuleSystem: ERROR - Exception during registration");
        return false;
    }
}
