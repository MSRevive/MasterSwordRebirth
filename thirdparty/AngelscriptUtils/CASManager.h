#ifndef ANGELSCRIPT_CASMANAGER_H
#define ANGELSCRIPT_CASMANAGER_H

#include <memory>

#include "util/ASPlatform.h"

#include "CASModuleManager.h"
#include "event/CASEventManager.h"

class asIScriptEngine;
struct asSMessageInfo;

class IASInitializer;

/**
*	@defgroup ASManager Angelscript Manager
*
*	@{
*/

/**
*	Manages the Angelscript engine instance, the module and event managers.
*	Multiple instances of this class can exist. In that case, you will have to activate the manager before using it.
*	Code that accesses the active manager will need it to be activated in order to work properly.
*/
class CASManager final
{
public:
	/**
	*	Gets the currently active manager.
	*	@see ActivateManager
	*	@see Activate
	*/
	static CASManager* GetActiveManager();

	/**
	*	Makes the given manager the active manager. Can be null.
	*/
	static void ActivateManager( CASManager* pManager );

	/**
	*	Makes this the active manager.
	*/
	void Activate();

	/**
	*	If this manager is the active manager, deactivates it.
	*/
	void Deactivate();

	/**
	*	Constructor.
	*/
	CASManager();

	/**
	*	Destructor.
	*/
	~CASManager();

	/**
	*	@return The script engine.
	*/
	asIScriptEngine* GetEngine() { return m_pScriptEngine; }

	/**
	*	@return The module manager.
	*/
	CASModuleManager& GetModuleManager() { return *m_ModuleManager; }

	/**
	*	@return The event manager.
	*/
	CASEventManager* GetEventManager() { return m_EventManager.get(); }

	/**
	*	Initializes the manager.
	*	On success, makes this the active manager.
	*	@param initializer Initializer to use.
	*	@return true on success, false otherwise.
	*/
	bool Initialize( IASInitializer& initializer );

	/**
	*	Shuts down the manager.
	*	Will set the active manager to null.
	*/
	void Shutdown();

private:
	/**
	*	@see asIScriptEngine::SetMessageCallback
	*/
	void MessageCallback( const asSMessageInfo* pMsg );

private:
	static CASManager* m_pActiveManager;

	asIScriptEngine* m_pScriptEngine = nullptr;

	std::unique_ptr<CASModuleManager> m_ModuleManager;
	std::shared_ptr<CASEventManager> m_EventManager;

private:
	CASManager( const CASManager& ) = delete;
	CASManager& operator=( const CASManager& ) = delete;
};

/** @} */

#endif //ANGELSCRIPT_CASMANAGER_H