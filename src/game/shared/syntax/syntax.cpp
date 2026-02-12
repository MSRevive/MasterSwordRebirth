#include "inc_weapondefs.h"

const char *SPEECH::ItemName(CGenericItem *pItem, bool fCapital)
{
	char returnStr[255];
	const char *pchDisplayOther = pItem->DisplayPrefix.c_str();

	if (pItem->iQuantity > 1)
	{
		_snprintf(returnStr, sizeof(returnStr), "%i %s%s", pItem->iQuantity, pItem->DisplayName(), "s");
		return std::string(returnStr).c_str(); // we do this to avoid manual free of heap allocation.
	}
	else if (pchDisplayOther && pchDisplayOther[0])
	{
		_snprintf(returnStr, sizeof(returnStr), "%s %s", pchDisplayOther, pItem->DisplayName());
		return std::string(returnStr).c_str(); // we do this to avoid manual free of heap allocation.
	}
	else
	{
		return pItem->DisplayName();
	}

	return "NullItem";
}

const char *SPEECH::NPCName(CMSMonster *pMonster, bool fCapital)
{
	char returnStr[255];

	msstring Prefix = pMonster->DisplayPrefix.len() ? (pMonster->DisplayPrefix + " ") : ("");
	_snprintf(returnStr, sizeof(returnStr), "%s%s", Prefix.c_str(), pMonster->DisplayName());

	if (fCapital)
		returnStr[0] = toupper(returnStr[0]);

	return std::string(returnStr).c_str(); // we do this to avoid manual free of heap allocation.
}

const char *SPEECH::HandName(int iHand, bool fCapital)
{
	if (iHand == 0)
		return fCapital ? "Left" : "left";
	if (iHand == 1)
		return fCapital ? "Right" : "right";
	return "invalid hand";
}