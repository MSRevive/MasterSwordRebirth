#ifndef VGUI_CONTAINER_H
#define VGUI_CONTAINER_H

#include "sharedutil.h"
#include "vgui_mscontrols.h"

#define CLASSMENU_BUTTON_SPACER_X XRES(6)
#define CLASSMENU_BUTTON_SPACER_Y ((LABEL_ITEMNAME_SIZE_Y + LABEL_ITEM_SPACER_Y) * STORE_INFO_LABELS + YRES(6))

#define GEARPNL_X XRES(0)
#define GEARPNL_Y YRES(110)
#define GEARPNL_SIZE_X (128 + XRES(10))
#define GEARPNL_SIZE_Y YRES(145)

#define GEARPNL_SPACER_X XRES(15)

#define ITEM_CONTAINER_X (GEARPNL_X + GEARPNL_SIZE_X) + GEARPNL_SPACER_X
#define ITEM_CONTAINER_Y GEARPNL_Y
#define ITEM_CONTAINER_SIZE_X XRES(440)
#define ITEM_CONTAINER_SIZE_Y YRES(260)

#define MAX_CONTAINERS 32

class VGUI_ContainerPanel;

struct gearitem_t
{
	msstring Name;
	ulong ID;
	bool IsContainer;
};

class VGUI_Inv_GearItem : public CTransparentPanel
{
public:
	MSLabel *m_Name;
	int m_Idx;
	int m_GearItemID;
	Panel *m_ContainerParent;
	VGUI_Container *m_ItemContainer;
	class CHandler_GearButton *m_pSignal;
	gearitem_t m_GearItem; //Determines where to obtain my list of items

	VGUI_Inv_GearItem(Panel *pContainerParent, VGUI_ItemCallbackPanel *pItemCallbackPanel, VGUI_ItemCallbackPanel *pGearCallback, Panel *pParent);
	~VGUI_Inv_GearItem();
	virtual void Update(gearitem_t &GearItem, int idx);

	void Reset();
	void Select();
	void DeSelect();
};

class VGUI_InventoryPanel : public CTransparentPanel, public VGUI_ItemCallbackPanel
{
public:
	mslist<VGUI_Inv_GearItem *> GearItemButtons;
	int GearItemButtonTotal;
	int m_Selected;
	int m_InitializedItemButtons;
	VGUI_ItemCallbackPanel *m_pCallbackPanel;
	CTFScrollPanel *m_Scroll;

	VGUI_InventoryPanel(VGUI_ItemCallbackPanel *pCallbackPanel, Panel *pParent);

	virtual VGUI_Inv_GearItem *AddGearItem(gearitem_t &GearItem);
	virtual void Select(int Idx);
	virtual void Reset();

	//Callbacks
	bool GearItemClicked(ulong ID);
	bool GearItemDoubleClicked(ulong ID);

	void StepInput(bool bDirUp); // MIB FEB2015_21 [INV_SCROLL]
};

class VGUI_ItemInfoPanel : public CTransparentPanel
{
public:
	CImageDelayed *m_Image;
	MSLabel *m_Name, *m_Quantity, *m_Quality, *m_SaleText;
	CTFScrollPanel *m_Scroll;

	VGUI_ItemInfoPanel(Panel *pParent);

	void Update(containeritem_t &Item);
};

class VGUI_ContainerPanel : public CMenuPanel, public VGUI_ItemCallbackPanel
{
protected:
	Label *m_pTitle,
		*m_pSubtitle;
	MSLabel *m_GoldLabel;
	MSButton *m_pCancelButton;
	VGUI_InventoryPanel *m_GearPanel;
	VGUI_ItemInfoPanel *m_InfoPanel;
	MSButton *m_ActButton;

public:
	static const char *m_Text_DoubleClick;

	VGUI_ContainerPanel();
	virtual void GetSelectedItems(mslist<VGUI_ItemButton *> &SelectedItems); //in, out
	virtual void UnSelectAllItems();
	virtual void AddInventoryItems();

	virtual void Open(void);
	virtual void Close(void);
	virtual void Update(void);
	virtual void Initialize(void);
	virtual bool SlotInput(int iSlot);

	//Callbacks
	virtual void ItemHighlighted(void* pData);
	virtual void ItemCreated(void* pData) { m_AllowUpdate = true; }

	bool m_AllowUpdate;
};

#endif //VGUI_CONTAINER_H
