#ifndef FN_DATA_HANDLER_H
#define FN_DATA_HANDLER_H

class CBasePlayer;

namespace FnDataHandler
{
	void LoadCharacter(CBasePlayer* pPlayer);
	void SaveCharacter(CBasePlayer* pPlayer);
	void CreateCharacter(CBasePlayer* pPlayer, unsigned char profile, const char* race, unsigned char gender, unsigned char* type);
	unsigned long long GetSteamID64(const char* id);
}

#endif // FN_DATA_HANDLER_H