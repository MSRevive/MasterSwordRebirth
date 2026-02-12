#include "inc_weapondefs.h"

#define MAX_SIZE 4096
char ReturnString[MAX_SIZE];

const char *SPEECH::ItemName(CGenericItem *pItem, bool fCapital)
{
	bool bNotUnique = (pItem->iQuantity > 1);
	char pchQuantity[32]; pchQuantity[0] = 0;
	const char *pchDisplayOther = pItem->DisplayPrefix.c_str();
	if (pItem->iQuantity > 1)
		_snprintf(pchQuantity, sizeof(pchQuantity), "%i", pItem->iQuantity);
	else if (pchDisplayOther && pchDisplayOther[0])
		_snprintf(pchQuantity, sizeof(pchQuantity), "%s", pchDisplayOther);
	_snprintf(ReturnString, MAX_SIZE, "%s %s%s", pchQuantity, pItem->DisplayName(), (bNotUnique ? "s" : ""));
	if (fCapital)
		ReturnString[0] = toupper(ReturnString[0]);
	return &ReturnString[0];
}

const char *SPEECH::NPCName(CMSMonster *pMonster, bool fCapital)
{
	msstring Prefix = pMonster->DisplayPrefix.len() ? (pMonster->DisplayPrefix + " ") : ("");
	_snprintf(ReturnString, MAX_SIZE, "%s%s", Prefix.c_str(), pMonster->DisplayName());
	if (fCapital)
		ReturnString[0] = toupper(ReturnString[0]);
	return &ReturnString[0];
}

const char *SPEECH::HandName(int iHand, bool fCapital)
{
	if (iHand == 0)
		return fCapital ? "Left" : "left";
	if (iHand == 1)
		return fCapital ? "Right" : "right";
	return "invalid hand";
}