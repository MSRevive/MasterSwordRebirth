//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SHARED_ANGELSCRIPT_CHLASMANAGER_H
#define GAME_SHARED_ANGELSCRIPT_CHLASMANAGER_H

#include <angelscript/angelscript.h>

#include <AngelscriptUtils/CASManager.h>

class IASInitializer;

/**
*	Manages the Angelscript engine.
*/
class CHL2ASManager
{
public:
	CHL2ASManager() = default;
	~CHL2ASManager() = default;

	void MessageCallback( asSMessageInfo* pMsg ) const;

	/**
	*	Initializes Angelscript, registers the API and module types.
	*	@return true on success, false on failure.
	*/
	virtual bool Initialize() = 0;

	CASManager &GetManager() { return m_Manager; }

protected:
	bool InitializeManager( IASInitializer& initializer );

	/**
	*	Shuts down Angelscript.
	*/
	virtual void Shutdown();

	CASManager m_Manager;

private:
	CHL2ASManager( const CHL2ASManager& ) = delete;
	CHL2ASManager& operator=( const CHL2ASManager& ) = delete;
};

#endif // GAME_SHARED_ZPS_ANGELSCRIPT_CHL2ASMANAGER_H
