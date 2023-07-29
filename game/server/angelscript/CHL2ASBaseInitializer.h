//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SHARED_ZPS_ANGELSCRIPT_CHL2ASBASEINITIALIZER_H
#define GAME_SHARED_ZPS_ANGELSCRIPT_CHL2ASBASEINITIALIZER_H

#include <AngelscriptUtils/IASInitializer.h>

class CHL2ASManager;

class CHL2ASBaseInitializer : public IASInitializer
{
public:
	CHL2ASBaseInitializer( CHL2ASManager& manager )
		: m_Manager( manager )
	{
	}

	bool UseEventManager() override { return true; }

	bool GetMessageCallback( asSFuncPtr& outFuncPtr, void*& pOutObj, asDWORD& outCallConv ) override;

	bool RegisterCoreAPI( CASManager& manager ) override;

	bool RegisterAPI( CASManager& manager ) override;

	bool AddEvents( CASManager& manager, CASEventManager& eventManager ) override;

protected:
	CHL2ASManager& m_Manager;
};

#endif //GAME_SHARED_ZPS_ANGELSCRIPT_CHL2ASBASEINITIALIZER_H
