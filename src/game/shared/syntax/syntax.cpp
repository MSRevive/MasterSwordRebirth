#include "inc_weapondefs.h"

static thread_local char ReturnString[255];

// TODO: switch to std::strings and std::format on dev branch with c++20
const char* SPEECH::ItemName(CGenericItem* pItem, bool fCapital)
{
	if (pItem->iQuantity > 1)
	{
		_snprintf(ReturnString, sizeof(ReturnString), "%i %s%s", pItem->iQuantity, pItem->DisplayName(), "s");
	}
	else
	{
		const char* prefix = pItem->DisplayPrefix.c_str();
		if (prefix && prefix[0])
		{
			_snprintf(ReturnString, sizeof(ReturnString), "%s %s", prefix, pItem->DisplayName());
		}
		else
		{
			return pItem->DisplayName();
		}
	}

	if (fCapital)
		ReturnString[0] = toupper(ReturnString[0]);

	return ReturnString;
}

// TODO: switch to std::strings and std::format on dev branch with c++20
const char* SPEECH::NPCName(CMSMonster* pMonster, bool fCapital)
{
	const char* prefix = pMonster->DisplayPrefix.c_str();
	const bool hasPrefix = prefix && prefix[0];

	if (hasPrefix)
	{
		_snprintf(ReturnString, sizeof(ReturnString), "%s %s", prefix, pMonster->DisplayName());
	}
	else
	{
		_snprintf(ReturnString, sizeof(ReturnString), "%s", pMonster->DisplayName());
	}

	if (fCapital)
		ReturnString[0] = toupper(ReturnString[0]);

	return ReturnString;
}

const char *SPEECH::HandName(int iHand, bool fCapital)
{
	if (iHand == 0)
		return fCapital ? "Left" : "left";
	if (iHand == 1)
		return fCapital ? "Right" : "right";
	return "invalid hand";
}