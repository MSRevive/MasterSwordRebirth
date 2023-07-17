//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SHARED_ZPS_ANGELSCRIPT_SCRIPTAPI_CASEVENTS_H
#define GAME_SHARED_ZPS_ANGELSCRIPT_SCRIPTAPI_CASEVENTS_H

#include <AngelscriptUtils/event/CASEvent.h>
#include <AngelscriptUtils/event/CASEventCaller.h>

class asIScriptEngine;

HookCallResult CallGlobalEvent( CASEvent& event, CallFlags_t flags, ... );

// Player
extern CASEvent asevent_PlayerSpawn;
extern CASEvent asevent_PlayerInitSpawn;
extern CASEvent asevent_PlayerSay;
extern CASEvent asevent_PlayerConnect;
extern CASEvent asevent_PlayerConnected;
extern CASEvent asevent_PlayerDisconnected;
extern CASEvent asevent_PlayerKilled;
extern CASEvent asevent_PlayerKilledPost;
extern CASEvent asevent_PlayerDamaged;
extern CASEvent asevent_PlayerDroppedItem;

void RegisterASEvents( asIScriptEngine &engine );

#endif // GAME_SHARED_ZPS_ANGELSCRIPT_SCRIPTAPI_CASEVENTS_H
