#include "inc_weapondefs.h"
#include "logfile.h"

bool CItemList ::CanAddItem(CGenericItem *NewItem)
{
	if (!NewItem)
		return false;

	//This item is already in your pack
	if (ItemExists(NewItem))
		return FALSE;

	return TRUE;
}
bool CItemList ::AddItem(CGenericItem *NewItem)
{
	if (!CanAddItem(NewItem))
		return false;

	add(NewItem);

	return true;
}

bool CItemList ::ItemExists(CGenericItem *pItem)
{
	if (!pItem)
		return false;

	for (int i = 0; i < size(); i++)
		if (operator[](i) == pItem)
			return true;

	return false;
}
CGenericItem *CItemList ::GetItem(const char *pszName)
{
	for (int i = 0; i < size(); i++)
		if (strstr(operator[](i)->ItemName, pszName))
			return operator[](i);

	return NULL;
}
CGenericItem *CItemList ::GetItem(ulong lID)
{
	for (int i = 0; i < size(); i++)
		if (operator[](i)->m_iId == lID)
			return operator[](i);

	return NULL;
}

bool CItemList ::RemoveItem(CGenericItem *pDelItem)
{
	int Delidx = -1;
	for (int i = 0; i < size(); i++)
		if (operator[](i) == pDelItem)
		{
			Delidx = i;
			break;
		}

	if (Delidx < 0)
		return false;

	erase(Delidx);

	return true;
}
float CItemList ::FilledVolume()
{
	//FilledVolume... also known as weight
	float Volume = 0;
	for (int i = 0; i < size(); i++)
		Volume += operator[](i)->Weight();

	return Volume;
}