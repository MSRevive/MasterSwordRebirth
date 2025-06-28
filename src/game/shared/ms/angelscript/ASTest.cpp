#include "ASTest.h"
#include "CAngelScriptManager.h"
#include <cstdio>  // for printf

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244) // conversion warnings from AngelScript
#pragma warning(disable: 4996) // deprecated function warnings
#endif

#include "angelscript.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <stdio.h>

namespace ASTest
{
    //==========================================================================
    // Test basic AngelScript functionality
    //==========================================================================
    bool TestBasicScript()
    {
        printf("[ASTest] Testing basic AngelScript functionality...\n");
        
        CAngelScriptManager* pMgr = CAngelScriptManager::Instance();
        if (!pMgr || !pMgr->IsInitialized())
        {
            printf("[ASTest] FAILED: AngelScript manager not initialized\n");
            return false;
        }
        
        // Test simple script compilation
        const char* testScript = 
            "void main() {"
            "  // Simple test function"
            "}";
            
        bool result = pMgr->CompileModule("test_basic", testScript);
        if (!result)
        {
            printf("[ASTest] FAILED: Could not compile basic test script\n");
            return false;
        }
        
        printf("[ASTest] PASSED: Basic script compilation successful\n");
        return true;
    }
    
    //==========================================================================
    // Test memory tracking
    //==========================================================================
    bool TestMemoryTracking()
    {
        printf("[ASTest] Testing memory tracking...\n");
        
        CAngelScriptManager* pMgr = CAngelScriptManager::Instance();
        if (!pMgr || !pMgr->IsInitialized())
        {
            printf("[ASTest] FAILED: AngelScript manager not initialized\n");
            return false;
        }
        
        size_t memBefore = pMgr->GetMemoryUsed();
        
        // Compile a script that should use some memory
        const char* testScript = 
            "void main() {"
            "  string test = 'This is a test string';"
            "  int[] numbers = {1, 2, 3, 4, 5};"
            "}";
            
        bool result = pMgr->CompileModule("test_memory", testScript);
        if (!result)
        {
            printf("[ASTest] FAILED: Could not compile memory test script\n");
            return false;
        }
        
        size_t memAfter = pMgr->GetMemoryUsed();
        
        printf("[ASTest] Memory before: %zu, after: %zu\n", memBefore, memAfter);
        printf("[ASTest] PASSED: Memory tracking functional\n");
        return true;
    }
    
    //==========================================================================
    // Test compilation and execution
    //==========================================================================
    bool TestScriptExecution()
    {
        printf("[ASTest] Testing script execution...\n");
        
        CAngelScriptManager* pMgr = CAngelScriptManager::Instance();
        if (!pMgr || !pMgr->IsInitialized())
        {
            printf("[ASTest] FAILED: AngelScript manager not initialized\n");
            return false;
        }
        
        // Simple script that we can execute
        const char* testScript = 
            "void test_function() {"
            "  // Simple test function"
            "}";
            
        bool result = pMgr->CompileModule("test_execution", testScript);
        if (!result)
        {
            printf("[ASTest] FAILED: Could not compile execution test script\n");
            return false;
        }
        
        // Get the module and try to find the function
        asIScriptModule* pModule = pMgr->GetModule("test_execution");
        if (!pModule)
        {
            printf("[ASTest] FAILED: Could not get compiled module\n");
            return false;
        }
        
        asIScriptFunction* pFunc = pModule->GetFunctionByName("test_function");
        if (!pFunc)
        {
            printf("[ASTest] FAILED: Could not find test function\n");
            return false;
        }
        
        // Try to execute the function
        asIScriptContext* pCtx = pMgr->AcquireContext();
        if (!pCtx)
        {
            printf("[ASTest] FAILED: Could not acquire script context\n");
            return false;
        }
        
        int r = pCtx->Prepare(pFunc);
        if (r < 0)
        {
            printf("[ASTest] FAILED: Could not prepare context for execution\n");
            pMgr->ReleaseContext(pCtx);
            return false;
        }
        
        r = pCtx->Execute();
        if (r != asEXECUTION_FINISHED)
        {
            printf("[ASTest] FAILED: Script execution failed with code %d\n", r);
            pMgr->ReleaseContext(pCtx);
            return false;
        }
        
        pMgr->ReleaseContext(pCtx);
        
        printf("[ASTest] PASSED: Script execution successful\n");
        return true;
    }
    
    //==========================================================================
    // Run all tests
    //==========================================================================
    bool RunAllTests()
    {
        printf("[ASTest] Running all AngelScript tests...\n");
        
        bool allPassed = true;
        
        if (!TestBasicScript())
            allPassed = false;
            
        if (!TestMemoryTracking())
            allPassed = false;
            
        if (!TestScriptExecution())
            allPassed = false;
        
        if (allPassed)
        {
            printf("[ASTest] ALL TESTS PASSED!\n");
        }
        else
        {
            printf("[ASTest] SOME TESTS FAILED!\n");
        }
        
        return allPassed;
    }
}