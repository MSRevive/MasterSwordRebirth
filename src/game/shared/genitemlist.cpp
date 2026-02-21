#include "inc_weapondefs.h"
#include <algorithm>

bool CItemList::CanAddItem(CGenericItem *NewItem)
{
	if (!NewItem)
		return false;

	if (ItemExists(NewItem))
		return false;

	return true;
}

bool CItemList::AddItem(CGenericItem *NewItem)
{
	if (!CanAddItem(NewItem))
		return false;

	push_back(NewItem);
	return true;
}

bool CItemList::ItemExists(CGenericItem *pItem)
{
	if (!pItem)
		return false;

	return std::find(begin(), end(), pItem) != end();
}

CGenericItem *CItemList::GetItem(const char *pszName)
{
	for (CGenericItem *pItem : *this)
		if (strstr(pItem->ItemName.c_str(), pszName))
			return pItem;

	return nullptr;
}

CGenericItem *CItemList::GetItem(ulong lID)
{
	for (CGenericItem *pItem : *this)
		if (pItem->m_iId == lID)
			return pItem;

	return nullptr;
}

bool CItemList::RemoveItem(CGenericItem *pDelItem)
{
	auto it = std::find(begin(), end(), pDelItem);
	if (it == end())
		return false;

	erase(it);
	return true;
}

float CItemList::FilledVolume()
{
	float Volume = 0;
	for (CGenericItem *pItem : *this)
		Volume += pItem->Weight();

	return Volume;
}