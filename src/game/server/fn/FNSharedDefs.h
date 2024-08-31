//
// FN Shared Definitions
//

#ifndef FN_SHAREDDEFS_H
#define FN_SHAREDDEFS_H

#include "rapidjson/fwd.h"
#include "steam/steamtypes.h"

class CBasePlayer;

enum FnPlayerFlags
{
	FN_FLAG_BANNED = 0x01,
	FN_FLAG_ADMIN = 0x02,
};

namespace FNShared
{
	void Print(char* fmt, ...);
	bool IsSlotValid(int slot);
	bool IsEnabled(void);
	void Validate(void);
	int GetPlayerFlags(const JSONValue& doc);
	void LoadCharacter(CBasePlayer* pPlayer);
	void LoadCharacter(CBasePlayer* pPlayer, int slot);
	void CreateOrUpdateCharacter(CBasePlayer* pPlayer, int slot, uint8* data, size_t size, bool bIsUpdate);
	void DeleteCharacter(CBasePlayer* pPlayer, int slot);
}

#endif // FN_SHAREDDEFS_H