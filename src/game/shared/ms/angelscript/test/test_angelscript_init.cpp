//==========================================================================
// test_angelscript_init.cpp
// 
// Test program to initialize AngelScript and load/execute test scripts
//==========================================================================

#include <iostream>
#include <chrono>
#include <thread>
#include "CAngelScriptManager.h"
#include "IAngelScript.h"
#include "ASBindings.h"
#include "angelscript.h"
#include "test_base_entity.h"

// Test entity class for script attachment
class CTestEntity : public CBaseEntity
{
public:
    CTestEntity() 
    {
        m_pScripts = new IAngelScript();
    }
    
    ~CTestEntity()
    {
        delete m_pScripts;
    }
    
    IAngelScript* GetScripts() { return m_pScripts; }
    
    // Override for test purposes
    const char* GetClassname() override { return "test_entity"; }
    
private:
    IAngelScript* m_pScripts;
};

// Global print function for scripts
void ScriptPrint(const std::string& msg)
{
    std::cout << "[Script] " << msg << std::endl;
}

// Register minimal functions needed for test script
void RegisterTestFunctions(asIScriptEngine* pEngine)
{
    // Register print function
    int r = pEngine->RegisterGlobalFunction("void print(const string &in)", 
                                           asFUNCTION(ScriptPrint), 
                                           asCALL_CDECL);
    if (r < 0)
    {
        std::cerr << "Failed to register print function" << std::endl;
    }
}

// Test Vector3 operations
bool TestVector3Operations(asIScriptEngine* pEngine)
{
    std::cout << "
=== Testing Vector3 Operations ===" << std::endl;
    
    // Create a test module
    asIScriptModule* mod = pEngine->GetModule("test", asGM_CREATE_IF_NOT_EXISTS);
    if (!mod)
    {
        std::cerr << "Failed to create test module" << std::endl;
        return false;
    }
    
    // Test script for Vector3
    const char* script = R"(
        void TestVector3()
        {
            // Test construction
            Vector3 v1(1.0f, 2.0f, 3.0f);
            Vector3 v2(4.0f, 5.0f, 6.0f);
            Vector3 v3;
            
            // Test properties
            print("v1.x = " + v1.x + ", v1.y = " + v1.y + ", v1.z = " + v1.z);
            
            // Test operators
            v3 = v1 + v2;
            print("v1 + v2 = (" + v3.x + ", " + v3.y + ", " + v3.z + ")");
            
            v3 = v1 - v2;
            print("v1 - v2 = (" + v3.x + ", " + v3.y + ", " + v3.z + ")");
            
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
            
            float dot = DotProduct(v1, v2);
            print("DotProduct(v1, v2) = " + dot);
            
            Vector3 cross = CrossProduct(v1, v2);
            print("CrossProduct(v1, v2) = (" + cross.x + ", " + cross.y + ", " + cross.z + ")");
        }
    )";
    
    // Add the script
    int r = mod->AddScriptSection("test", script);
    if (r < 0)
    {
        std::cerr << "Failed to add script section" << std::endl;
        return false;
    }
    
    // Build the module
    r = mod->Build();
    if (r < 0)
    {
        std::cerr << "Failed to build module" << std::endl;
        return false;
    }
    
    // Execute the test function
    asIScriptFunction* func = mod->GetFunctionByName("TestVector3");
    if (!func)
    {
        std::cerr << "Failed to find TestVector3 function" << std::endl;
        return false;
    }
    
    asIScriptContext* ctx = pEngine->CreateContext();
    ctx->Prepare(func);
    r = ctx->Execute();
    if (r != asEXECUTION_FINISHED)
    {
        std::cerr << "Script execution failed" << std::endl;
        ctx->Release();
        return false;
    }
    ctx->Release();
    
    std::cout << "✓ Vector3 operations test passed" << std::endl;
    return true;
}

// Test string functions
bool TestStringFunctions(asIScriptEngine* pEngine)
{
    std::cout << "
=== Testing String Functions ===" << std::endl;
    
    asIScriptModule* mod = pEngine->GetModule("test", asGM_CREATE_IF_NOT_EXISTS);
    
    const char* script = R"(
        void TestStrings()
        {
            string str = "Hello AngelScript World";
            
            // Test string functions
            string left = Left(str, 5);
            print("Left('" + str + "', 5) = '" + left + "'");
            
            string right = Right(str, 5);
            print("Right('" + str + "', 5) = '" + right + "'");
            
            string mid = Mid(str, 6, 11);
            print("Mid('" + str + "', 6, 11) = '" + mid + "'");
            
            int len = Length(str);
            print("Length('" + str + "') = " + len);
            
            string upper = ToUpper(str);
            print("ToUpper('" + str + "') = '" + upper + "'");
            
            string lower = ToLower(str);
            print("ToLower('" + str + "') = '" + lower + "'");
            
            string replaced = Replace(str, "AngelScript", "AS");
            print("Replace AngelScript->AS = '" + replaced + "'");
        }
    )";
    
    mod->AddScriptSection("test_strings", script);
    mod->Build();
    
    asIScriptFunction* func = mod->GetFunctionByName("TestStrings");
    asIScriptContext* ctx = pEngine->CreateContext();
    ctx->Prepare(func);
    int r = ctx->Execute();
    ctx->Release();
    
    std::cout << "✓ String functions test passed" << std::endl;
    return r == asEXECUTION_FINISHED;
}

// Test math functions
bool TestMathFunctions(asIScriptEngine* pEngine)
{
    std::cout << "
=== Testing Math Functions ===" << std::endl;
    
    asIScriptModule* mod = pEngine->GetModule("test", asGM_CREATE_IF_NOT_EXISTS);
    
    const char* script = R"(
        void TestMath()
        {
            // Test trig functions
            float angle = 0.785398f; // 45 degrees in radians
            print("sin(45°) = " + sin(angle));
            print("cos(45°) = " + cos(angle));
            
            // Test other math functions
            print("sqrt(16) = " + sqrt(16.0f));
            print("abs(-42) = " + abs(-42.0f));
            print("min(10, 20) = " + min(10.0f, 20.0f));
            print("max(10, 20) = " + max(10.0f, 20.0f));
            
            // Test constants
            print("PI = " + PI);
            print("E = " + E);
        }
    )";
    
    mod->AddScriptSection("test_math", script);
    mod->Build();
    
    asIScriptFunction* func = mod->GetFunctionByName("TestMath");
    asIScriptContext* ctx = pEngine->CreateContext();
    ctx->Prepare(func);
    int r = ctx->Execute();
    ctx->Release();
    
    std::cout << "✓ Math functions test passed" << std::endl;
    return r == asEXECUTION_FINISHED;
}

// Test game functions
bool TestGameFunctions(asIScriptEngine* pEngine)
{
    std::cout << "
=== Testing Game Functions ===" << std::endl;
    
    asIScriptModule* mod = pEngine->GetModule("test", asGM_CREATE_IF_NOT_EXISTS);
    
    const char* script = R"(
        void TestGame()
        {
            // Test time function
            float time = GetGameTime();
            print("GetGameTime() = " + time);
            
            // Test random functions
            float randFloat = Random(0.0f, 100.0f);
            print("Random(0, 100) = " + randFloat);
            
            int randInt = RandomInt(1, 10);
            print("RandomInt(1, 10) = " + randInt);
            
            // Test logging
            LogMessage("This is a test log message");
            DeveloperMessage(1, "This is a developer message");
            
            // Test player count
            int playerCount = GetPlayerCount();
            print("GetPlayerCount() = " + playerCount);
            
            // Test angle functions
            Vector3 angles = CreateAngles(10.0f, 20.0f, 30.0f);
            print("CreateAngles(10, 20, 30) = (" + angles.x + ", " + angles.y + ", " + angles.z + ")");
            
            float pitch = GetAnglePitch(angles);
            float yaw = GetAngleYaw(angles);
            float roll = GetAngleRoll(angles);
            print("Pitch: " + pitch + ", Yaw: " + yaw + ", Roll: " + roll);
        }
    )";
    
    mod->AddScriptSection("test_game", script);
    mod->Build();
    
    asIScriptFunction* func = mod->GetFunctionByName("TestGame");
    asIScriptContext* ctx = pEngine->CreateContext();
    ctx->Prepare(func);
    int r = ctx->Execute();
    ctx->Release();
    
    std::cout << "✓ Game functions test passed" << std::endl;
    return r == asEXECUTION_FINISHED;
}

// Main test function
int main(int argc, char* argv[])
{
    std::cout << "=== AngelScript Test Program ===" << std::endl;
    
    // Step 1: Initialize AngelScript Manager
    std::cout << "1. Initializing AngelScript Manager..." << std::endl;
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (!pManager->Initialize())
    {
        std::cerr << "Failed to initialize AngelScript manager!" << std::endl;
        return 1;
    }
    std::cout << "   ✓ Manager initialized successfully" << std::endl;
    
    // Step 2: Register test functions
    std::cout << "\n2. Registering test functions..." << std::endl;
    asIScriptEngine* pEngine = pManager->GetEngine();
    RegisterTestFunctions(pEngine);
    std::cout << "   ✓ Functions registered" << std::endl;
    
    // Step 3: Create test entity
    std::cout << "\n3. Creating test entity..." << std::endl;
    CTestEntity testEntity;
    IAngelScript* pScripts = testEntity.GetScripts();
    std::cout << "   ✓ Test entity created" << std::endl;
    
    // Step 4: Load test script
    std::cout << "\n4. Loading test script..." << std::endl;
    const char* scriptPath = "test_angelscript_simple.as";
    CAngelScript* pScript = pScripts->Script_Add(scriptPath, &testEntity);
    if (!pScript)
    {
        std::cerr << "Failed to load script: " << scriptPath << std::endl;
        return 1;
    }
    std::cout << "   ✓ Script loaded: " << scriptPath << std::endl;
    
    // Step 5: Test immediate event execution
    std::cout << "\n5. Testing immediate event execution..." << std::endl;
    pScripts->CallScriptEvent("test_event");
    std::cout << "   ✓ Immediate event called" << std::endl;
    
    // Step 6: Test timed event execution
    std::cout << "\n6. Testing timed event execution..." << std::endl;
    pScripts->CallScriptEventTimed("delayed_event", 2.0f); // 2 second delay
    std::cout << "   ✓ Timed event scheduled (2 second delay)" << std::endl;
    
    // Step 7: Test repeating event
    std::cout << "\n7. Testing repeating event execution..." << std::endl;
    ASEventParams params;
    params.Add("Repeating test");
    pScripts->CallScriptEventRepeating("test_event", 1.0f, 1.5f, &params);
    std::cout << "   ✓ Repeating event scheduled (1s initial, 1.5s repeat)" << std::endl;
    
    // Step 8: Run event loop to process timed events
    std::cout << "\n8. Running event loop for 8 seconds..." << std::endl;
    auto startTime = std::chrono::steady_clock::now();
    int frameCount = 0;
    
    while (true)
    {
        // Simulate frame update
        pScripts->Think();
        pManager->Think(); // Run garbage collection
        
        frameCount++;
        
        // Check elapsed time
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        
        if (elapsed >= 8)
            break;
            
        // Simulate 60 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    std::cout << "   ✓ Event loop completed (" << frameCount << " frames)" << std::endl;
    
    // Step 9: Cancel remaining events
    std::cout << "\n9. Canceling remaining timed events..." << std::endl;
    pScripts->CancelTimedEvents();
    std::cout << "   ✓ Events canceled" << std::endl;
    
    // Step 10: Test variable system
    std::cout << "\n10. Testing variable system..." << std::endl;
    pScripts->SetScriptVar("test_string", "Hello AngelScript!");
    pScripts->SetScriptVar("test_int", 42);
    pScripts->SetScriptVar("test_float", 3.14f);
    
    std::cout << "    String var: " << pScripts->GetScriptVar("test_string") << std::endl;
    std::cout << "    Int var: " << pScripts->GetScriptVar("test_int") << std::endl;
    std::cout << "    Float var: " << pScripts->GetScriptVar("test_float") << std::endl;
    std::cout << "   ✓ Variable system working" << std::endl;
    
    // Step 11: Cleanup
    std::cout << "\n11. Cleaning up..." << std::endl;
    pScripts->Deactivate();
    pManager->Shutdown();
    std::cout << "   ✓ Cleanup complete" << std::endl;
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    
    return 0;
}