#include <iostream>
#include <fstream>
#include <string>
#include "thirdparty/angelscript/angelscript.h"
#include "src/game/shared/ms/angelscript/addons/scriptbuilder/scriptbuilder.h"

// Message callback for AngelScript
void MessageCallback(const asSMessageInfo *msg, void *param)
{
    const char *type = "ERR ";
    if (msg->type == asMSGTYPE_WARNING)
        type = "WARN";
    else if (msg->type == asMSGTYPE_INFORMATION)
        type = "INFO";
    
    printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

// Simple print function for testing
void ScriptPrint(const std::string& str)
{
    std::cout << "Script: " << str << std::endl;
}

int main()
{
    // Create the AngelScript engine
    asIScriptEngine* engine = asCreateScriptEngine();
    if (!engine)
    {
        std::cerr << "Failed to create AngelScript engine" << std::endl;
        return 1;
    }
    
    // Set message callback
    engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);
    
    // Register string type (basic)
    engine->RegisterObjectType("string", 0, asOBJ_REF);
    
    // Register print function
    engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(ScriptPrint), asCALL_CDECL);
    
    std::cout << "\n=== Testing WITHOUT CScriptBuilder (current approach) ===" << std::endl;
    
    // Method 1: Direct engine usage (current approach) - THIS WILL FAIL
    {
        std::ifstream file("test_include_test.as");
        if (file.is_open())
        {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
            
            asIScriptModule* mod = engine->GetModule("test_direct", asGM_ALWAYS_CREATE);
            int r = mod->AddScriptSection("main", content.c_str());
            if (r < 0)
            {
                std::cerr << "Failed to add script section" << std::endl;
            }
            else
            {
                r = mod->Build();
                if (r < 0)
                {
                    std::cerr << "Failed to build module (expected - #include not supported)" << std::endl;
                }
                else
                {
                    std::cout << "Module built successfully (unexpected)" << std::endl;
                }
            }
        }
    }
    
    std::cout << "\n=== Testing WITH CScriptBuilder (proper approach) ===" << std::endl;
    
    // Method 2: Using CScriptBuilder (proper approach) - THIS WILL WORK
    {
        CScriptBuilder builder;
        
        int r = builder.StartNewModule(engine, "test_builder");
        if (r < 0)
        {
            std::cerr << "Failed to start new module" << std::endl;
        }
        else
        {
            r = builder.AddSectionFromFile("test_include_test.as");
            if (r < 0)
            {
                std::cerr << "Failed to add section from file" << std::endl;
            }
            else
            {
                r = builder.BuildModule();
                if (r < 0)
                {
                    std::cerr << "Failed to build module with CScriptBuilder" << std::endl;
                }
                else
                {
                    std::cout << "Module built successfully with CScriptBuilder!" << std::endl;
                    
                    // Try to execute the test function
                    asIScriptModule* mod = builder.GetModule();
                    if (mod)
                    {
                        asIScriptFunction* func = mod->GetFunctionByName("TestMainFunction");
                        if (func)
                        {
                            asIScriptContext* ctx = engine->CreateContext();
                            if (ctx)
                            {
                                ctx->Prepare(func);
                                int execResult = ctx->Execute();
                                if (execResult == asEXECUTION_FINISHED)
                                {
                                    std::cout << "Script executed successfully!" << std::endl;
                                }
                                else
                                {
                                    std::cerr << "Script execution failed" << std::endl;
                                }
                                ctx->Release();
                            }
                        }
                        else
                        {
                            std::cerr << "TestMainFunction not found" << std::endl;
                        }
                    }
                }
            }
        }
    }
    
    // Cleanup
    engine->ShutDownAndRelease();
    
    return 0;
}