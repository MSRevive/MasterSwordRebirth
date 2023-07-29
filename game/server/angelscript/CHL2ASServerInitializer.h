//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SERVER_ZPS_ANGELSCRIPT_CHL2ASSERVERINITIALIZER_H
#define GAME_SERVER_ZPS_ANGELSCRIPT_CHL2ASSERVERINITIALIZER_H

#include "angelscript/CHL2ASBaseInitializer.h"

class CHL2ASServerInitializer final : public CHL2ASBaseInitializer
{
public:
	using CHL2ASBaseInitializer::CHL2ASBaseInitializer;

	CHL2ASServerInitializer( CHL2ASManager& manager ) : CHL2ASBaseInitializer( manager ) {}

	bool UseEventManager() override { return true; }

	bool RegisterCoreAPI( CASManager& manager ) override;

	bool AddEvents( CASManager& manager, CASEventManager& eventManager ) override;
};

#endif //GAME_SERVER_ZPS_ANGELSCRIPT_CHL2ASSERVERINITIALIZER_H
