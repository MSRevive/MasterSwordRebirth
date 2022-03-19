#ifndef ITEMLIST_H
#define ITEMLIST_H

class CGenericItem;

class CItemList : public mslist<CGenericItem *>
{
public:
	bool CanAddItem(CGenericItem *NewItem);
	bool AddItem(CGenericItem *NewItem);
	bool ItemExists(CGenericItem *pItem);

	CGenericItem *GetItem(const char *pszName);
	CGenericItem *GetItem(ulong lID);

	bool RemoveItem(CGenericItem *pDelItem);

	float FilledVolume();
};

#endif //ITEMLIST_H