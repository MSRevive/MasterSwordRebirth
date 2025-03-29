#include "vgui_storemainwin.h"

class CStoreSellPanel : public CStorePanel
{
public:
	mslist<containeritem_t> m_SelectedItems;

	bool InterestedInItem(const char* pszItemName);
	void SellAll();

	CStoreSellPanel(Panel *pParent);

	//Item callbacks
	void ItemCreated(void *pData);
	void ItemSelectChanged(ulong ID, bool fSelected);
	void ItemHighlighted(void *pData);
};
