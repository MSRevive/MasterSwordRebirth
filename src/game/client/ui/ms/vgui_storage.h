#include "vgui_storemainwin.h"

class CStoragePanel : public CStorePanel
{
public:
	mslist<containeritem_t> m_SelectedItems;
	ulong m_LastGearItem;

	//void TakeAll( );

	CStoragePanel(Panel *pParent);
	void AddInventoryItems();
	void Update();
	void Close();
	int ItemRetrievalCost(containeritem_t &Item);

	//Item callbacks
	void ItemSelectChanged(ulong ID, bool fSelected);
	void ItemHighlighted(void *pData);
	void GearItemSelected(ulong ID);
	bool GearItemClicked(ulong ID);
	bool checkValid(containeritem_t &Item); //MiB Feb2008a

	static msstring_ref Text_Subtitle_Storage;
	static msstring_ref Text_Subtitle_Inventory;
};
