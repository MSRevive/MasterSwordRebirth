//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SERVER_ZPS_ANGELSCRIPT_SCRIPTAPI_CASCHAT_H
#define GAME_SERVER_ZPS_ANGELSCRIPT_SCRIPTAPI_CASCHAT_H

#include <string.h>

class asIScriptEngine;
class CBasePlayer;
class CTerrorPlayer;

enum ChatTeam
{
	chat_any,
	chat_survivor = 2,
	chat_zombie
};

class CASChat final
{
public:
	CASChat() = default;
	~CASChat() = default;

	void CenterMessage( ChatTeam iChatType, const std::string& strMsg );
	void HintMessage( ChatTeam iChatType, const std::string& strMsg );
	void PrintToChat( ChatTeam iChatType, const std::string& strMsg );
	void PrintToConsole( ChatTeam iChatType, const std::string& strMsg );
	void PrintToConsole2( const std::string& strMsg );

	void CenterMessage_Player( CBasePlayer *pPlayer, const std::string& strMsg );
	void HintMessage_Player( CBasePlayer *pPlayer, const std::string& strMsg );
	void PrintToChat_Player( CBasePlayer *pPlayer, const std::string& strMsg );
	void PrintToChat_PlayerTerror( CTerrorPlayer *pPlayer, const std::string& strMsg );
	void PrintToConsole_Player( CBasePlayer *pPlayer, const std::string& strMsg );
	void PrintToConsole_TerrorPlayer( CTerrorPlayer *pPlayer, const std::string& strMsg );

private:
	CASChat( const CASChat & ) = delete;
	CASChat& operator=( const CASChat & ) = delete;
};

void RegisterASChat( asIScriptEngine &engine );

#endif // GAME_SERVER_ZPS_ANGELSCRIPT_SCRIPTAPI_CASCHAT_H
