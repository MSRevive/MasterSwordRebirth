#include "vgui_storemainwin.h"
#include "vgui_moveitempanel.h"

class CStoragePanel : public CStorePanel
{
private:
	VGUI_MoveItemPanel *mpTakePanel, *mpStorePanel;

	void HideMovePanels();

public:
	mslist<containeritem_t> m_SelectedItems;
	ulong m_LastGearItem;

	//void TakeAll( );

	CStoragePanel(Panel *pParent);
	void AddInventoryItems();
	void Update();
	void Close();

	//Item callbacks
	void ItemSelectChanged(ulong ID, bool fSelected);
	void ItemHighlighted(void *pData);
	void GearItemSelected(ulong ID);
	bool GearItemClicked(ulong ID);
	void ItemDoubleclicked(ulong ID);
	bool ItemClicked(void *pData);
	void ItemRightClicked(void *pData);
	void MoveItem(VGUI_ItemButton *pButton, int vNumMove);
	void InvTypeChanged(int vInvType);
	void AlphabeticChanged(bool bAlphabetic);
	bool checkValid(containeritem_t &Item); //MiB Feb2008a

	static const char* Text_Subtitle_Storage;
	static const char* Text_Subtitle_Inventory;
};
