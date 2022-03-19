#include "msdllheaders.h"
#include "player/player.h"
#include "weapons/genericitem.h"
#include "msitemdefs.h"

#include "magic.h"

bool Magic::Prepare(const char *pszName, CBasePlayer *pPlayer)
{
	CGenericItem *pItem = NewGenericItem(pszName);
	if (!pItem)
		return false;

	//TODO: Check conditions for preparing the spell

	char cErrorString[256] = "";
	int Error = pPlayer->NewItemHand(pItem, false, false, true, cErrorString);
	if (Error < 0)
	{
		if (Error == -2)
		{
			msstring_ref Hands = "a hand";
			if (pItem->m_PrefHand == BOTH_HANDS)
				Hands = "both hands";

			pPlayer->SendEventMsg(HUDEVENT_UNABLE, msstring("You need ") + Hands + " available to cast " + pItem->DisplayName());
		}

		pItem->SUB_Remove(); //Player failed to receive item
		return false;
	}

	pItem->GiveTo(pPlayer, false, false);

	if (!pItem->m_pPlayer)
		pItem->SUB_Remove(); //Player failed to receive item
	else
		pItem->Spell_Prepare();

	return true;
}
