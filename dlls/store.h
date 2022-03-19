#include "../msshared/storeshared.h"

class CStore
{
	mslist<storeitem_t> Items;

public:
	void SetName(const char *pszName);
	void Offer(edict_t *pePlayer, int iBuyFlags, CBaseMonster *pVendor);
	bool AddItem(const char *pszItemName, int iQuantity, int CostPercent, float flSellRatio, int iBundleAmt);
	storeitem_t *GetItem(const char *pszItemName);
	storeitem_t *GetItem(int idx);
	string_i m_Name;
	void Deactivate();
	void RemoveItem(msstring_ref Name);
	void RemoveAllItems();

	static CStore *GetStoreByName(msstring_ref Name)
	{
		for (int i = 0; i < m_gStores.size(); i++)
			if (FStrEq(m_gStores[i]->m_Name, Name))
				return m_gStores[i];
		return NULL;
	}
	static void RemoveAllStores()
	{
		while (m_gStores.size())
			m_gStores[0]->Deactivate();
	}
	static mslist<CStore *> m_gStores;
};
