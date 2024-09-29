//
// FN Shared Definitions
//

#ifndef FN_SHAREDDEFS_H
#define FN_SHAREDDEFS_H

#include "rapidjson/fwd.h"
#include "steam/steamtypes.h"

class CBasePlayer;

enum FNPlayerFlags
{
	FN_FLAG_BANNED = (1 << 0),
	FN_FLAG_DONOR = (1 << 1),
	FN_FLAG_DEV = (1 << 2),
	FN_FLAG_ADMIN = (1 << 3),
};

namespace FNShared
{
	void Print(const char* fmt, ...);
	bool IsSlotValid(int slot);
	bool IsEnabled(void);
	void Validate(void);
	void ValidateFN(void);
	int GetPlayerFlags(const JSONValue& doc);
	void LoadCharacter(CBasePlayer* pPlayer);
	void LoadCharacter(CBasePlayer* pPlayer, int slot);
	void CreateOrUpdateCharacter(CBasePlayer* pPlayer, int slot, uint8* data, size_t size, bool bIsUpdate);
	void DeleteCharacter(CBasePlayer* pPlayer, int slot);
}

#endif // FN_SHAREDDEFS_H