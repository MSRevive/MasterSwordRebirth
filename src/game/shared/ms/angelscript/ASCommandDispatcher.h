#ifndef ASCOMMANDDISPATCHER_H
#define ASCOMMANDDISPATCHER_H

//==========================================================================
// ASCommandDispatcher - AngelScript Command System with asbind20 Integration
// 
// Singleton class for dispatching game commands to AngelScript handlers
// Uses asbind20 for efficient C++ to script communication
//==========================================================================

#include <string>

// Forward declarations
class asIScriptEngine;
class asIScriptFunction;
class CBasePlayer;

//==========================================================================
// ASCommandDispatcher Class
//==========================================================================
class ASCommandDispatcher
{
private:
    static ASCommandDispatcher* s_pInstance;
    
    // AngelScript integration
    asIScriptEngine* m_pEngine;
    asIScriptFunction* m_pProcessCommandFunc;
    
    // State
    bool m_bInitialized;
    std::string m_LastError;
    
    // Private constructor for singleton
    ASCommandDispatcher();
    
public:
    ~ASCommandDispatcher();
    
    // Singleton access
    static ASCommandDispatcher* Instance();
    static void Shutdown();
    
    // Core functionality
    bool Initialize();
    void Cleanup();
    
    // Command dispatching
    bool DispatchCommand(CBasePlayer* pPlayer, const char* pszCommand, const char* pszArgs);
    
    // State queries
    bool IsInitialized() const { return m_bInitialized; }
    const char* GetLastError() const { return m_LastError.c_str(); }
    
    // AngelScript engine access
    asIScriptEngine* GetEngine() const { return m_pEngine; }
    
private:
    // Internal methods
    bool RegisterCommandTypes();
    bool LoadCommandModule();
    void SetError(const char* pszError);
    
    // Prevent copying
    ASCommandDispatcher(const ASCommandDispatcher&) = delete;
    ASCommandDispatcher& operator=(const ASCommandDispatcher&) = delete;
};

// Global convenience functions
inline ASCommandDispatcher* GetCommandDispatcher() { return ASCommandDispatcher::Instance(); }

#endif // ASCOMMANDDISPATCHER_H