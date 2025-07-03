//==========================================================================
// ASCommandDispatcher - AngelScript Command System Implementation
// 
// Uses asbind20 for efficient C++ to AngelScript command dispatch
//==========================================================================

#include "ASCommandDispatcher.h"
#include "CAngelScriptManager.h"

// asbind20 includes
#include <asbind20/asbind.hpp>
#include <asbind20/memory.hpp>
#include <asbind20/invoke.hpp>

// AngelScript includes
#include "angelscript.h"

// Master Sword includes
#include "mslogger.h"

// Player/entity includes (check which header contains CBasePlayer)
#ifdef VALVE_DLL
    #include "player/player.h"  // Server build
#elif defined(CLIENT_DLL)
    #include "cl_entity.h"      // Client build - check actual header
#endif

//==========================================================================
// Static member initialization
//==========================================================================
ASCommandDispatcher* ASCommandDispatcher::s_pInstance = nullptr;

//==========================================================================
// Constructor
//==========================================================================
ASCommandDispatcher::ASCommandDispatcher()
    : m_pEngine(nullptr)
    , m_pProcessCommandFunc(nullptr)
    , m_bInitialized(false)
{
}

//==========================================================================
// Destructor
//==========================================================================
ASCommandDispatcher::~ASCommandDispatcher()
{
    Cleanup();
}

//==========================================================================
// Singleton access
//==========================================================================
ASCommandDispatcher* ASCommandDispatcher::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new ASCommandDispatcher();
    }
    return s_pInstance;
}

//==========================================================================
// Singleton shutdown
//==========================================================================
void ASCommandDispatcher::Shutdown()
{
    if (s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

//==========================================================================
// Initialize the command dispatcher
//==========================================================================
bool ASCommandDispatcher::Initialize()
{
    if (m_bInitialized)
    {
        return true;
    }
    
    // Get AngelScript engine from manager
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (!pManager || !pManager->IsInitialized())
    {
        SetError("AngelScript manager not initialized");
        return false;
    }
    
    m_pEngine = pManager->GetEngine();
    if (!m_pEngine)
    {
        SetError("Failed to get AngelScript engine");
        return false;
    }
    
    // Register command types using asbind20
    if (!RegisterCommandTypes())
    {
        SetError("Failed to register command types");
        return false;
    }
    
    // Load the command module
    if (!LoadCommandModule())
    {
        SetError("Failed to load command module");
        return false;
    }
    
    m_bInitialized = true;
    MS_ANGEL_INFO("ASCommandDispatcher initialized successfully");
    
    return true;
}

//==========================================================================
// Cleanup
//==========================================================================
void ASCommandDispatcher::Cleanup()
{
    m_bInitialized = false;
    m_pProcessCommandFunc = nullptr;
    m_pEngine = nullptr;
    m_LastError.clear();
}

//==========================================================================
// Dispatch command to AngelScript using asbind20
//==========================================================================
bool ASCommandDispatcher::DispatchCommand(CBasePlayer* pPlayer, const char* pszCommand, const char* pszArgs)
{
    // Null pointer safety
    if (!pPlayer || !pszCommand)
    {
        SetError("Null pointer: player or command");
        return false;
    }
    
    // Check initialization
    if (!m_bInitialized || !m_pEngine || !m_pProcessCommandFunc)
    {
        SetError("Command dispatcher not initialized");
        return false;
    }
    
    // Safe argument handling
    const char* safeArgs = pszArgs ? pszArgs : "";
    
    try
    {
        // Use asbind20's RAII context management
        asbind20::request_context ctx(m_pEngine);
        
        // Call AngelScript function using asbind20::script_invoke
        auto result = asbind20::script_invoke<bool>(
            ctx,                        // RAII context
            m_pProcessCommandFunc,      // Script function to call
            pPlayer,                    // CBasePlayer* (automatically bound)
            std::string(pszCommand),    // Command name (automatic string conversion)
            std::string(safeArgs)       // Command arguments (automatic conversion)
        );
        
        // Check if script execution succeeded
        if (result.has_value())
        {
            return result.value();
        }
        else
        {
            // Get error code and create descriptive message
            int errorCode = result.error();
            char errorMsg[256];
            snprintf(errorMsg, sizeof(errorMsg), "Script execution failed with error code: %d", errorCode);
            SetError(errorMsg);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        // Handle asbind20 exceptions
        char errorMsg[512];
        snprintf(errorMsg, sizeof(errorMsg), "Exception during command dispatch: %s", e.what());
        SetError(errorMsg);
        return false;
    }
    catch (...)
    {
        SetError("Unknown exception during command dispatch");
        return false;
    }
}

//==========================================================================
// Register command types using asbind20
//==========================================================================
bool ASCommandDispatcher::RegisterCommandTypes()
{
    if (!m_pEngine)
    {
        return false;
    }
    
    try
    {
        // Register CBasePlayer type using asbind20's automatic binding generation
        asbind20::ref_class<CBasePlayer>(m_pEngine, "CBasePlayer")
            // Essential identification methods
            .method("string DisplayName() const", &CBasePlayer::DisplayName)
            .method("int GetEntIndex() const", &CBasePlayer::GetEntIndex)
            
            // State checking methods
            .method("bool IsConnected() const", &CBasePlayer::IsConnected)
            .method("bool IsAlive() const", &CBasePlayer::IsAlive)
            .method("bool IsAdmin() const", &CBasePlayer::IsAdmin)
            
            // Add reference counting if CBasePlayer uses it
            // .addref(&CBasePlayer::AddRef)      // Uncomment if CBasePlayer has AddRef
            // .release(&CBasePlayer::Release)     // Uncomment if CBasePlayer has Release
            ;
        
        MS_ANGEL_INFO("Command types registered successfully");
        return true;
    }
    catch (const std::exception& e)
    {
        char errorMsg[512];
        snprintf(errorMsg, sizeof(errorMsg), "Exception during type registration: %s", e.what());
        SetError(errorMsg);
        return false;
    }
    catch (...)
    {
        SetError("Unknown exception during type registration");
        return false;
    }
}

//==========================================================================
// Load command module and get ProcessCommand function
//==========================================================================
bool ASCommandDispatcher::LoadCommandModule()
{
    if (!m_pEngine)
    {
        return false;
    }
    
    // Look for existing command module or create one
    asIScriptModule* pModule = m_pEngine->GetModule("CommandModule");
    if (!pModule)
    {
        // Try to create the module
        pModule = m_pEngine->GetModule("CommandModule", asGM_CREATE_IF_NOT_EXISTS);
        if (!pModule)
        {
            SetError("Failed to create CommandModule");
            return false;
        }
        
        // For now, we expect the module to be loaded externally
        // In the future, this could load command scripts from files
        MS_ANGEL_WARNING("CommandModule created but not populated - commands will not work until scripts are loaded");
    }
    
    // Look for the main ProcessCommand function
    m_pProcessCommandFunc = pModule->GetFunctionByDecl("bool ProcessCommand(CBasePlayer@, const string&in, const string&in)");
    if (!m_pProcessCommandFunc)
    {
        // Try alternative declaration
        m_pProcessCommandFunc = pModule->GetFunctionByName("ProcessCommand");
        if (!m_pProcessCommandFunc)
        {
            SetError("ProcessCommand function not found in CommandModule");
            return false;
        }
    }
    
    MS_ANGEL_INFO("Command module loaded successfully");
    return true;
}

//==========================================================================
// Set error message
//==========================================================================
void ASCommandDispatcher::SetError(const char* pszError)
{
    m_LastError = pszError ? pszError : "Unknown error";
    MS_ANGEL_ERROR("ASCommandDispatcher: %s", m_LastError.c_str());
}