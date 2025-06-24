//==========================================================================
// test_bindings.cpp
// 
//==========================================================================

#include <iostream>
#include <chrono>
#include "CAngelScriptManager.h"
#include "ASBindings.h"
#include "angelscript.h"

// Global print function for scripts
void ScriptPrint(const std::string& msg)
{
    std::cout << "[Script] " << msg << std::endl;
}

// Register test functions
void RegisterTestFunctions(asIScriptEngine* pEngine)
{
    pEngine->RegisterGlobalFunction("void print(const string &in)", 
                                   asFUNCTION(ScriptPrint), 
                                   asCALL_CDECL);
}

// Test Vector3 operations
bool TestVector3Operations(asIScriptEngine* pEngine)
{
    std::cout << "\n=== Testing Vector3 Operations ===" << std::endl;
    
    asIScriptModule* mod = pEngine->GetModule("test_vector3", asGM_CREATE_IF_NOT_EXISTS);
    
    const char* script = R"(
        void TestVector3()
        {
            // Test construction
            Vector3 v1(1.0f, 2.0f, 3.0f);
            Vector3 v2(4.0f, 5.0f, 6.0f);
            Vector3 v3;
            
            // Test properties
            print("v1 = (" + v1.x + ", " + v1.y + ", " + v1.z + ")");
            
            // Test operators
            v3 = v1 + v2;
            print("v1 + v2 = (" + v3.x + ", " + v3.y + ", " + v3.z + ")");
            
            v3 = v1 * 2.0f;
            print("v1 * 2 = (" + v3.x + ", " + v3.y + ", " + v3.z + ")");
            
            // Test methods
            float len = v1.Length();
            print("v1.Length() = " + len);
            
            // Test global functions
            Vector3 v4 = CreateVector(10.0f, 20.0f, 30.0f);
            print("CreateVector(10,20,30) = (" + v4.x + ", " + v4.y + ", " + v4.z + ")");
            
            float dist = Distance(v1, v2);
            print("Distance(v1, v2) = " + dist);
        }
    )";
    
    mod->AddScriptSection("test", script);
    if (mod->Build() < 0)
    {
        std::cerr << "Failed to build Vector3 test script" << std::endl;
        return false;
    }
    
    asIScriptFunction* func = mod->GetFunctionByName("TestVector3");
    if (!func)
    {
        std::cerr << "TestVector3 function not found" << std::endl;
        return false;
    }
    
    asIScriptContext* ctx = pEngine->CreateContext();
    ctx->Prepare(func);
    int r = ctx->Execute();
    ctx->Release();
    
    if (r == asEXECUTION_FINISHED)
    {
        std::cout << "✓ Vector3 operations test passed" << std::endl;
        return true;
    }
    else
    {
        std::cerr << "✗ Vector3 operations test failed" << std::endl;
        return false;
    }
}

// Test string functions
bool TestStringFunctions(asIScriptEngine* pEngine)
{
    std::cout << "\n=== Testing String Functions ===" << std::endl;
    
    asIScriptModule* mod = pEngine->GetModule("test_strings", asGM_CREATE_IF_NOT_EXISTS);
    
    const char* script = R"(
        void TestStrings()
        {
            string str = "Hello AngelScript World";
            
            string left = Left(str, 5);
            print("Left(5) = '" + left + "'");
            
            string right = Right(str, 5);
            print("Right(5) = '" + right + "'");
            
            string mid = Mid(str, 6, 11);
            print("Mid(6,11) = '" + mid + "'");
            
            int len = Length(str);
            print("Length = " + len);
            
            string upper = ToUpper(str);
            print("ToUpper = '" + upper + "'");
            
            string replaced = Replace(str, "AngelScript", "AS");
            print("Replace = '" + replaced + "'");
        }
    )";
    
    mod->AddScriptSection("test", script);
    if (mod->Build() < 0)
    {
        std::cerr << "Failed to build string test script" << std::endl;
        return false;
    }
    
    asIScriptFunction* func = mod->GetFunctionByName("TestStrings");
    if (!func) return false;
    
    asIScriptContext* ctx = pEngine->CreateContext();
    ctx->Prepare(func);
    int r = ctx->Execute();
    ctx->Release();
    
    if (r == asEXECUTION_FINISHED)
    {
        std::cout << "✓ String functions test passed" << std::endl;
        return true;
    }
    else
    {
        std::cerr << "✗ String functions test failed" << std::endl;
        return false;
    }
}

// Test math functions
bool TestMathFunctions(asIScriptEngine* pEngine)
{
    std::cout << "\n=== Testing Math Functions ===" << std::endl;
    
    asIScriptModule* mod = pEngine->GetModule("test_math", asGM_CREATE_IF_NOT_EXISTS);
    
    const char* script = R"(
        void TestMath()
        {
            float angle = 0.785398f; // 45 degrees
            print("sin(45°) = " + sin(angle));
            print("cos(45°) = " + cos(angle));
            print("sqrt(16) = " + sqrt(16.0f));
            print("abs(-42) = " + abs(-42.0f));
            print("min(10, 20) = " + min(10.0f, 20.0f));
            print("max(10, 20) = " + max(10.0f, 20.0f));
            print("PI = " + PI);
            print("E = " + E);
        }
    )";
    
    mod->AddScriptSection("test", script);
    if (mod->Build() < 0)
    {
        std::cerr << "Failed to build math test script" << std::endl;
        return false;
    }
    
    asIScriptFunction* func = mod->GetFunctionByName("TestMath");
    if (!func) return false;
    
    asIScriptContext* ctx = pEngine->CreateContext();
    ctx->Prepare(func);
    int r = ctx->Execute();
    ctx->Release();
    
    if (r == asEXECUTION_FINISHED)
    {
        std::cout << "✓ Math functions test passed" << std::endl;
        return true;
    }
    else
    {
        std::cerr << "✗ Math functions test failed" << std::endl;
        return false;
    }
}

// Test game functions
bool TestGameFunctions(asIScriptEngine* pEngine)
{
    std::cout << "\n=== Testing Game Functions ===" << std::endl;
    
    asIScriptModule* mod = pEngine->GetModule("test_game", asGM_CREATE_IF_NOT_EXISTS);
    
    const char* script = R"(
        void TestGame()
        {
            float time = GetGameTime();
            print("GetGameTime() = " + time);
            
            float randFloat = Random(0.0f, 100.0f);
            print("Random(0, 100) = " + randFloat);
            
            int randInt = RandomInt(1, 10);
            print("RandomInt(1, 10) = " + randInt);
            
            LogMessage("Test log message from script");
            DeveloperMessage(1, "Test developer message");
            
            int playerCount = GetPlayerCount();
            print("GetPlayerCount() = " + playerCount);
            
            Vector3 angles = CreateAngles(10.0f, 20.0f, 30.0f);
            print("CreateAngles(10,20,30) = (" + angles.x + ", " + angles.y + ", " + angles.z + ")");
        }
    )";
    
    mod->AddScriptSection("test", script);
    if (mod->Build() < 0)
    {
        std::cerr << "Failed to build game test script" << std::endl;
        return false;
    }
    
    asIScriptFunction* func = mod->GetFunctionByName("TestGame");
    if (!func) return false;
    
    asIScriptContext* ctx = pEngine->CreateContext();
    ctx->Prepare(func);
    int r = ctx->Execute();
    ctx->Release();
    
    if (r == asEXECUTION_FINISHED)
    {
        std::cout << "✓ Game functions test passed" << std::endl;
        return true;
    }
    else
    {
        std::cerr << "✗ Game functions test failed" << std::endl;
        return false;
    }
}

// Integration test combining multiple binding types
bool TestIntegration(asIScriptEngine* pEngine)
{
    std::cout << "\n=== Testing Integration ===" << std::endl;
    
    asIScriptModule* mod = pEngine->GetModule("test_integration", asGM_CREATE_IF_NOT_EXISTS);
    
    const char* script = R"(
        void TestIntegration()
        {
            print("=== Integration Test ===");
            
            // Vector3 + String operations
            Vector3 v(1.5f, 2.5f, 3.5f);
            string vecStr = "Vector: (" + v.x + ", " + v.y + ", " + v.z + ")";
            print(ToUpper(vecStr));
            
            // Math + Game operations
            float time = GetGameTime();
            float result = sqrt(time * PI);
            string resultStr = "Math result: " + result;
            LogMessage(resultStr);
            
            // Complex combination
            int randomNum = RandomInt(1, 100);
            Vector3 randomVec = CreateVector(randomNum, randomNum * 2, randomNum * 3);
            float distance = Distance(v, randomVec);
            string finalMsg = Replace("Distance from v to random: DISTANCE", "DISTANCE", distance);
            print(finalMsg);
            
            print("Integration test complete!");
        }
    )";
    
    mod->AddScriptSection("test", script);
    if (mod->Build() < 0)
    {
        std::cerr << "Failed to build integration test script" << std::endl;
        return false;
    }
    
    asIScriptFunction* func = mod->GetFunctionByName("TestIntegration");
    if (!func) return false;
    
    asIScriptContext* ctx = pEngine->CreateContext();
    ctx->Prepare(func);
    int r = ctx->Execute();
    ctx->Release();
    
    if (r == asEXECUTION_FINISHED)
    {
        std::cout << "✓ Integration test passed" << std::endl;
        return true;
    }
    else
    {
        std::cerr << "✗ Integration test failed" << std::endl;
        return false;
    }
}

// Main test program
int main(int argc, char* argv[])
{
    // Initialize AngelScript
    std::cout << "\n1. Initializing AngelScript..." << std::endl;
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (!pManager->Initialize())
    {
        std::cerr << "Failed to initialize AngelScript manager!" << std::endl;
        return 1;
    }
    std::cout << "   ✓ AngelScript initialized" << std::endl;
    
    // Register all bindings
    std::cout << "\n2. Registering bindings..." << std::endl;
    asIScriptEngine* pEngine = pManager->GetEngine();
    
    if (!ASBindings::RegisterAll(pEngine))
    {
        std::cerr << "Failed to register bindings!" << std::endl;
        return 1;
    }
    
    RegisterTestFunctions(pEngine);
    std::cout << "   ✓ All bindings registered successfully" << std::endl;
    
    // Run all tests
    std::cout << "\n3. Running comprehensive tests..." << std::endl;
    
    bool allTestsPassed = true;
    
    // Test each binding category
    if (!TestVector3Operations(pEngine)) allTestsPassed = false;
    if (!TestStringFunctions(pEngine)) allTestsPassed = false;
    if (!TestMathFunctions(pEngine)) allTestsPassed = false;
    if (!TestGameFunctions(pEngine)) allTestsPassed = false;
    if (!TestIntegration(pEngine)) allTestsPassed = false;
    
    // Performance test
    std::cout << "\n4. Running performance test..." << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    asIScriptModule* perfMod = pEngine->GetModule("perf_test", asGM_CREATE_IF_NOT_EXISTS);
    const char* perfScript = R"(
        void PerformanceTest()
        {
            // Vector operations
            for (int i = 0; i < 1000; i++)
            {
                Vector3 v1(1.0f, 2.0f, 3.0f);
                Vector3 v2(4.0f, 5.0f, 6.0f);
                Vector3 result = v1 + v2;
                float len = result.Length();
            }
            
            // String operations
            for (int i = 0; i < 100; i++)
            {
                string str = "Test " + i;
                string upper = ToUpper(str);
                string sub = Mid(str, 0, 4);
            }
        }
    )";
    
    perfMod->AddScriptSection("perf", perfScript);
    if (perfMod->Build() >= 0)
    {
        asIScriptFunction* perfFunc = perfMod->GetFunctionByName("PerformanceTest");
        if (perfFunc)
        {
            asIScriptContext* perfCtx = pEngine->CreateContext();
            perfCtx->Prepare(perfFunc);
            perfCtx->Execute();
            perfCtx->Release();
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    std::cout << "   ✓ Performance test completed in " << duration << " microseconds" << std::endl;
    
    // Cleanup
    std::cout << "\n5. Cleaning up..." << std::endl;
    pManager->Shutdown();
    std::cout << "   ✓ Cleanup complete" << std::endl;

    return allTestsPassed ? 0 : 1;
}