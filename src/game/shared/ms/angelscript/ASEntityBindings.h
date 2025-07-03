//==========================================================================
// ASEntityBindings.h
// 
// Entity-related type bindings for AngelScript integration
// Includes comprehensive player management functions for GameMaster system
//==========================================================================

#pragma once

class asIScriptEngine;

namespace ASEntityBindings
{
    void RegisterAll(asIScriptEngine* pEngine);
    void RegisterEntityTypes(asIScriptEngine* pEngine);
    void RegisterGlobalFunctions(asIScriptEngine* pEngine);
    
    // Player Management Functions Available in AngelScript:
    // - array<CBasePlayer@>@ GetAllPlayers() - Get all connected players
    // - int GetPlayerCount() - Get number of connected players
    // - CBasePlayer@ PlayerByIndex(int index) - Get player by index
    // - bool IsConnected(CBasePlayer@ player) - Check if player is connected
    // - string GetDisplayName(CBasePlayer@ player) - Get player display name
    // - string GetSteamID(CBasePlayer@ player) - Get player Steam ID
    // - bool IsAdmin(CBasePlayer@ player) - Check admin status
    // - string GetClientAddress(CBasePlayer@ player) - Get player IP address
    // - string GetPlayerQuestData(const string& playerID, const string& key) - Quest data retrieval
    // - void SetPlayerQuestData(const string& playerID, const string& key, const string& value) - Quest data setting
}