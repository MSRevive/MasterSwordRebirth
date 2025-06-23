#include "CAngelScript.h"
#include "CAngelScriptManager.h"
#include "angelscript/angelscript.h"
#include "../global.h"
#include "../sharedutil.h"
#include <algorithm>

//==========================================================================
// Constructor
//==========================================================================
CAngelScript::CAngelScript()
    : m_pOwner(nullptr)
    , m_pModule(nullptr)
    , m_pScriptObject(nullptr)
    , m_bRunning(false)
    , m_Scope(AS_EVENTSCOPE_SERVER)
{
}

//==========================================================================
// Destructor
//==========================================================================
CAngelScript::~CAngelScript()
{
    Cleanup();
}

//==========================================================================
// Initialize script
//==========================================================================
bool CAngelScript::Init(const char* scriptName, CBaseEntity* pOwner)
{
    if (!scriptName || !scriptName[0])
    {
        SetError("Invalid script name");
        return false;
    }
    
    m_ScriptName = scriptName;
    m_pOwner = pOwner;
    
    // Load the script file
    char fullPath[256];
    snprintf(fullPath, sizeof(fullPath), "scripts/%s.script", scriptName);
    
    if (!LoadScript(fullPath))
    {
        return false;
    }
    
    m_bRunning = true;
    return true;
}

//==========================================================================
// Cleanup
//==========================================================================
void CAngelScript::Cleanup()
{
    m_bRunning = false;
    
    // Release script object
    if (m_pScriptObject)
    {
        m_pScriptObject->Release();
        m_pScriptObject = nullptr;
    }
    
    // Clear events
    m_Events.clear();
    m_EventTimers.clear();
    
    // Clear variables
    m_Variables.clear();
    
    // Module will be managed by AngelScript engine
    m_pModule = nullptr;
    
    m_LastError.clear();
    m_ReturnData.clear();
}

//==========================================================================
// Load script from file
//==========================================================================
bool CAngelScript::LoadScript(const char* fileName)
{
    if (!fileName || !fileName[0])
    {
        SetError("Invalid file name");
        return false;
    }
    
    // Read script file content
    // For now, we'll use a simple file reading approach
    // In production, this should use the game's file system
    FILE* file = fopen(fileName, "rb");
    if (!file)
    {
        SetError("Failed to open script file");
        return false;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Read content
    std::string scriptContent;
    scriptContent.resize(fileSize);
    size_t bytesRead = fread(&scriptContent[0], 1, fileSize, file);
    fclose(file);
    
    if (bytesRead != fileSize)
    {
        SetError("Failed to read script file");
        return false;
    }
    
    // Compile the script
    return CompileScript(scriptContent.c_str());
}

//==========================================================================
// Compile script content
//==========================================================================
bool CAngelScript::CompileScript(const char* scriptContent)
{
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (!pManager || !pManager->IsInitialized())
    {
        SetError("AngelScript manager not initialized");
        return false;
    }
    
    asIScriptEngine* pEngine = pManager->GetEngine();
    if (!pEngine)
    {
        SetError("No AngelScript engine available");
        return false;
    }
    
    // Create module name based on script name
    std::string moduleName = "MS_" + m_ScriptName;
    
    // Get or create module
    m_pModule = pEngine->GetModule(moduleName.c_str(), asGM_ALWAYS_CREATE);
    if (!m_pModule)
    {
        SetError("Failed to create script module");
        return false;
    }
    
    // Add the script section
    int r = m_pModule->AddScriptSection("script", scriptContent);
    if (r < 0)
    {
        SetError("Failed to add script section");
        return false;
    }
    
    // Build the module
    r = m_pModule->Build();
    if (r < 0)
    {
        SetError("Failed to compile script");
        return false;
    }
    
    // Parse for events
    if (!ParseScriptForEvents())
    {
        return false;
    }
    
    // Create script object if we have a main class
    CreateScriptObject();
    
    return true;
}

//==========================================================================
// Register event
//==========================================================================
bool CAngelScript::RegisterEvent(const char* eventName, const char* functionDecl)
{
    if (!eventName || !eventName[0] || !m_pModule)
        return false;
        
    // Find the function in the module
    asIScriptFunction* pFunc = m_pModule->GetFunctionByDecl(functionDecl);
    if (!pFunc)
    {
        // Try finding by name only
        pFunc = m_pModule->GetFunctionByName(eventName);
        if (!pFunc)
            return false;
    }
    
    // Create event entry
    ASScriptEvent event;
    event.name = eventName;
    event.pFunction = pFunc;
    event.scope = m_Scope;
    event.fNextExecutionTime = 0;
    event.fRepeatDelay = 0;
    
    // Add to map
    m_Events[eventName] = event;
    
    return true;
}

//==========================================================================
// Get event by name
//==========================================================================
ASScriptEvent* CAngelScript::GetEvent(const char* eventName)
{
    if (!eventName || !eventName[0])
        return nullptr;
        
    auto it = m_Events.find(eventName);
    if (it != m_Events.end())
        return &it->second;
        
    return nullptr;
}

//==========================================================================
// Check if event exists
//==========================================================================
bool CAngelScript::HasEvent(const char* eventName) const
{
    if (!eventName || !eventName[0])
        return false;
        
    return m_Events.find(eventName) != m_Events.end();
}

//==========================================================================
// Remove event
//==========================================================================
void CAngelScript::RemoveEvent(const char* eventName)
{
    m_Events.erase(eventName);
}

//==========================================================================
// Execute event
//==========================================================================
bool CAngelScript::ExecuteEvent(const char* eventName, ASEventParams* pParams)
{
    ASScriptEvent* pEvent = GetEvent(eventName);
    if (!pEvent || !pEvent->pFunction)
        return false;
        
    return ExecuteFunction(pEvent->pFunction, pParams);
}

//==========================================================================
// Execute function
//==========================================================================
bool CAngelScript::ExecuteFunction(asIScriptFunction* pFunc, ASEventParams* pParams)
{
    if (!pFunc)
        return false;
        
    asIScriptContext* pCtx = GetContext();
    if (!pCtx)
        return false;
        
    // Prepare the context
    int r = pCtx->Prepare(pFunc);
    if (r < 0)
    {
        ReleaseContext(pCtx);
        return false;
    }
    
    // Set object pointer if this is a method
    if (m_pScriptObject && pFunc->GetObjectType())
    {
        pCtx->SetObject(m_pScriptObject);
    }
    
    // Set parameters
    if (pParams)
    {
        int paramCount = pFunc->GetParamCount();
        for (int i = 0; i < paramCount && i < (int)pParams->Count(); i++)
        {
            // For now, pass all as strings
            // TODO: Implement proper type handling
            pCtx->SetArgObject(i, (void*)pParams->Get(i));
        }
    }
    
    // Execute
    r = pCtx->Execute();
    
    // Check result
    bool bSuccess = (r == asEXECUTION_FINISHED);
    
    if (bSuccess)
    {
        // Get return value if any
        if (pFunc->GetReturnTypeId() != asTYPEID_VOID)
        {
            // Handle return value based on type
            // For now, convert to string
            if (pFunc->GetReturnTypeId() == asTYPEID_INT32)
            {
                int retVal = pCtx->GetReturnDWord();
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%d", retVal);
                m_ReturnData = buffer;
            }
            else if (pFunc->GetReturnTypeId() == asTYPEID_FLOAT)
            {
                float retVal = pCtx->GetReturnFloat();
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%.2f", retVal);
                m_ReturnData = buffer;
            }
            // Add more type handling as needed
        }
    }
    else
    {
        SetError("Script execution failed");
    }
    
    ReleaseContext(pCtx);
    return bSuccess;
}

//==========================================================================
// Find variable
//==========================================================================
ASScriptVar* CAngelScript::FindVar(const char* name)
{
    if (!name || !name[0])
        return nullptr;
        
    for (auto& var : m_Variables)
    {
        if (var.name == name)
            return &var;
    }
    
    return nullptr;
}

//==========================================================================
// Get variable value
//==========================================================================
const char* CAngelScript::GetVar(const char* name)
{
    ASScriptVar* pVar = FindVar(name);
    return pVar ? pVar->value.c_str() : "";
}

//==========================================================================
// Set variable (string)
//==========================================================================
void CAngelScript::SetVar(const char* name, const char* value)
{
    if (!name || !name[0])
        return;
        
    ASScriptVar* pVar = FindVar(name);
    if (pVar)
    {
        pVar->value = value ? value : "";
    }
    else
    {
        m_Variables.push_back(ASScriptVar(name, value ? value : ""));
    }
}

//==========================================================================
// Set variable (int)
//==========================================================================
void CAngelScript::SetVar(const char* name, int value)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", value);
    SetVar(name, buffer);
}

//==========================================================================
// Set variable (float)
//==========================================================================
void CAngelScript::SetVar(const char* name, float value)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.2f", value);
    SetVar(name, buffer);
}

//==========================================================================
// Legacy compatibility - run event
//==========================================================================
void CAngelScript::RunScriptEventByName(const char* eventName, msstringlist* legacyParams)
{
    ASEventParams asParams;
    if (ConvertLegacyParams(legacyParams, asParams))
    {
        ExecuteEvent(eventName, &asParams);
    }
    else
    {
        ExecuteEvent(eventName);
    }
}

//==========================================================================
// Parse script for events (temporary implementation)
//==========================================================================
bool CAngelScript::ParseScriptForEvents()
{
    if (!m_pModule)
        return false;
        
    // For now, just register standard game events if they exist
    // Later this will parse the actual script content
    const char* standardEvents[] = {
        "game_spawn",
        "game_death", 
        "game_damaged",
        "game_damaged_other",
        "game_dodamage",
        "game_takedamage",
        "game_think",
        "game_touch",
        "game_use",
        "game_activate",
        "game_deactivate",
        nullptr
    };
    
    for (int i = 0; standardEvents[i]; i++)
    {
        asIScriptFunction* pFunc = m_pModule->GetFunctionByName(standardEvents[i]);
        if (pFunc)
        {
            RegisterEvent(standardEvents[i], pFunc->GetDeclaration());
        }
    }
    
    return true;
}

//==========================================================================
// Create script object
//==========================================================================
bool CAngelScript::CreateScriptObject()
{
    // This will be implemented when we define script classes
    // For now, scripts are just collections of functions
    return true;
}

//==========================================================================
// Set error message
//==========================================================================
void CAngelScript::SetError(const char* error)
{
    m_LastError = error ? error : "";
}

//==========================================================================
// Get context from pool
//==========================================================================
asIScriptContext* CAngelScript::GetContext()
{
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    return pManager ? pManager->AcquireContext() : nullptr;
}

//==========================================================================
// Release context to pool
//==========================================================================
void CAngelScript::ReleaseContext(asIScriptContext* pCtx)
{
    if (!pCtx)
        return;
        
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (pManager)
        pManager->ReleaseContext(pCtx);
}

//==========================================================================
// Convert legacy parameters
//==========================================================================
bool CAngelScript::ConvertLegacyParams(msstringlist* legacyParams, ASEventParams& asParams)
{
    if (!legacyParams || legacyParams->size() == 0)
        return false;
        
    for (size_t i = 0; i < legacyParams->size(); i++)
    {
        asParams.Add((*legacyParams)[i].c_str());
    }
    
    return true;
}

//==========================================================================
// Global helper - find script
//==========================================================================
CAngelScript* FindScript(CBaseEntity* pEntity, const char* scriptName)
{
    // This will be implemented based on how entities store scripts
    return nullptr;
}

//==========================================================================
// Global helper - execute event on all scripts
//==========================================================================
bool ExecuteEventOnScripts(CBaseEntity* pEntity, const char* eventName, ASEventParams* pParams)
{
    // This will be implemented based on how entities store scripts
    return false;
}