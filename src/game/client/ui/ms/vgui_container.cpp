//=========== Master Sword ===========
//
//-----------------------------------------------------------------------------
// $Log: vgui_Container.cpp,v $
// Revision 1.1  2005/01/17 13:16:49  dogg
// Brand new inventory VGUI, revised item system, Magic, Mp3 support and item storage
//
// Revision 1.2  2004/09/11 22:21:03  dogg
// fixed the <break> problems in cl_dll/MasterSword here!
//
// Revision 1.1  2004/09/07 17:06:01  reddog
// First commit! ;-)
//
//
// $NoKeywords: $
//=============================================================================

//Main inventory screen - includes Gear Item panel and info panel

#include "inc_weapondefs.h"
#undef DLLEXPORT

#include "../hud.h"
#include "../cl_util.h"
#include "../vgui_teamfortressviewport.h"

//Master Sword
#include "vgui_container.h"
#include "logger.h"

// Menu Dimensions
#define GEARPNL_PACK_SPACER_Y YRES(2)

#define LINESPACER_Y YRES(10)

COLOR Color_GearSelected = COLOR(255, 0, 0, 0),
	  Color_GearNormal = COLOR(255, 255, 255, 0),
	  Color_GearNonContainer = COLOR(160, 160, 160, 0),
	  Color_TextHighlighted = COLOR(255, 255, 255, 0),
	  Color_TextNormal = COLOR(100, 100, 100, 0);

class CHandler_GearButton : public InputSignal
{
public:
	VGUI_ItemCallbackPanel *m_Callback;
	VGUI_DoubleClickDetector *mpDoubleClickDetector;
	int m_Idx;
	VGUI_Inv_GearItem* mpHighlightCallback;

	CHandler_GearButton(VGUI_ItemCallbackPanel *pParent, VGUI_DoubleClickDetector *pDoubleClickDetector, VGUI_Inv_GearItem *pHighlightCallback, int idx)
	{
		m_Callback = pParent;
		mpDoubleClickDetector = pDoubleClickDetector;
		mpHighlightCallback = pHighlightCallback;
		m_Idx = idx;
	}

	void mousePressed(MouseCode code, Panel *panel)
	{
		if ( code == MOUSE_LEFT )
		{
			if ( mpDoubleClickDetector->Click( this, code ) )
			{
				mouseDoublePressed( code, panel );
			}
			else
			{
				m_Callback->GearItemClicked( m_Idx );
			}
		}
	}

	void cursorEntered(Panel *panel) { mpHighlightCallback->mbMouseOver = true; }
	void cursorMoved(int x, int y, Panel *panel) {};
	void mouseReleased(MouseCode code, Panel *panel) {};
	void mouseDoublePressed(MouseCode code, Panel *panel)
	{
		m_Callback->GearItemDoubleClicked(m_Idx);
	};
	void cursorExited(Panel *panel) { mpHighlightCallback->mbMouseOver = false; };
	void mouseWheeled(int delta, Panel *panel) {};
	void keyPressed(KeyCode code, Panel *panel) {};
	void keyTyped(KeyCode code, Panel *panel) {};
	void keyReleased(KeyCode code, Panel *panel) {};
	void keyFocusTicked(Panel *panel) {};
};

VGUI_Inv_GearItem::VGUI_Inv_GearItem(Panel *pContainerParent, VGUI_ItemCallbackPanel *pItemCallbackPanel, VGUI_ItemCallbackPanel *pGearCallback, Panel *pParent) : 
	CTransparentPanel(255, 0, 0, GEARPNL_SIZE_X, YRES(13))
{
	setParent(pParent);
	m_ContainerParent = pContainerParent;

	m_Name = new MSLabel(this, "", 0, 0, getWide(), getTall(), MSLabel::a_center);
	m_Name->addInputSignal(m_pSignal = new CHandler_GearButton(pGearCallback, pGearCallback, this, m_Idx));
	m_ItemContainer = new VGUI_Container(ITEM_CONTAINER_X, ITEM_CONTAINER_Y, ITEM_CONTAINER_SIZE_X, ITEM_CONTAINER_SIZE_Y, pItemCallbackPanel, pContainerParent);
	mbMouseOver = false;
}

VGUI_Inv_GearItem::~VGUI_Inv_GearItem()
{
	/*m_ContainerParent->removeChild( m_ItemContainer );
	delete m_ItemContainer;
	m_Name->removeInputSignal( m_pSignal );
	delete m_pSignal;
	removeChild( m_Name );
	delete m_Name;*/
}

void VGUI_Inv_GearItem::Update(gearitem_t &GearItem, int idx)
{
	m_Idx = idx;
	m_pSignal->m_Idx = m_Idx;
	m_GearItemID = GearItem.ID;
	m_GearItem = GearItem;
	setPos(0, GEARPNL_PACK_SPACER_Y + (getTall() + GEARPNL_PACK_SPACER_Y) * idx);
	setVisible(true);

	m_Name->setText(GearItem.Name);

	DeSelect();
	//Delete all items in the container
	m_ItemContainer->PurgeButtons();
}

void VGUI_Inv_GearItem::Reset()
{
	//Delete all items in the container
	m_ItemContainer->PurgeButtons();

	setVisible(false);
	mbMouseOver = false;
	DeSelect();
}

void VGUI_Inv_GearItem::Select()
{
	m_Name->SetFGColorRGB(Color_GearSelected);

	m_ItemContainer->setVisible(true);
	if (m_GearItem.IsContainer)
	{
		m_ItemContainer->m_pInvTypePanel->setVisible(true);
		m_ItemContainer->Update();
	}
}

void VGUI_Inv_GearItem::DeSelect()
{
	if (m_GearItem.IsContainer)
		m_Name->SetFGColorRGB(Color_GearNormal);
	else
		m_Name->SetFGColorRGB(Color_GearNonContainer);

	m_ItemContainer->setVisible(false);
	m_ItemContainer->m_pInvTypePanel->setVisible(false);
}

VGUI_InventoryPanel::VGUI_InventoryPanel(VGUI_ItemCallbackPanel *pCallbackPanel, Panel *pParent) : 
	CTransparentPanel(INVENTORY_TRANSPARENCY, GEARPNL_X, GEARPNL_Y, GEARPNL_SIZE_X, GEARPNL_SIZE_Y),
	VGUI_ItemCallbackPanel()
{
	setParent(pParent);
	m_pCallbackPanel = pCallbackPanel;
	m_Selected = 0;
	m_InitializedItemButtons = 0;

	m_Scroll = new CTFScrollPanel(0, 0, getWide(), getTall());
	m_Scroll->setParent(this);
	m_Scroll->setScrollBarAutoVisible(false, true);
	m_Scroll->setScrollBarVisible(false, false);
	m_Scroll->validate();

	mbScrollHighlighted = false;
    m_Scroll->addInputSignal( new CSimpleInputSignal( this, NULL, NULL ) );
}

VGUI_Inv_GearItem *VGUI_InventoryPanel::AddGearItem(gearitem_t &GearItem)
{
	if (GearItemButtonTotal >= MAX_CONTAINERS)
		return nullptr;

	//Add a new gear item

	if (GearItemButtonTotal >= m_InitializedItemButtons)
	{
		//If this GearItemButton hasn't been initialized yet, initialize it.
		//I do this here, so I only initialize the ones that get used
		GearItemButtons.add(new VGUI_Inv_GearItem(getParent(), m_pCallbackPanel, this, m_Scroll->getClient()));
		m_InitializedItemButtons++;
	}

	VGUI_Inv_GearItem &GearItemButton = *GearItemButtons[GearItemButtonTotal];
	GearItemButton.Update(GearItem, GearItemButtonTotal);
	GearItemButtonTotal++;

	//int x, y;
	//GearItemButton.getPos( x, y );

	//setSize( getWide(), y + GearItemButton.getTall() );	//Resize the Inventory panel to hold the last gear item
	m_Scroll->validate();

	Select(m_Selected);

	return &GearItemButton;
}

void VGUI_InventoryPanel::Reset()
{
	//Delete the old
	for (int i = 0; i < GearItemButtonTotal; i++)
		GearItemButtons[i]->Reset();
	GearItemButtonTotal = 0;
}

void VGUI_InventoryPanel::Select(int Idx)
{
	if (Idx < 0 || Idx >= GearItemButtonTotal)
		return;

	for (int i = 0; i < GearItemButtonTotal; i++)
		GearItemButtons[i]->DeSelect();

	m_Selected = Idx;
	VGUI_Inv_GearItem &GearItemButton = *GearItemButtons[Idx];
	GearItemButton.Select();

	if (m_pCallbackPanel)
		m_pCallbackPanel->GearItemSelected(GearItemButton.m_GearItemID);
}

bool VGUI_InventoryPanel::GearItemClicked(ulong ID)
{
	if (ID < 0 || ID >= (unsigned)GearItemButtonTotal)
		return true;

	//Item was selected by user click.  The user might be trying to move an item to this pack
	if (!m_pCallbackPanel || !m_pCallbackPanel->GearItemClicked(GearItemButtons[ID]->m_GearItemID))
	{
		Select(ID);
	}

	return false;
}

CTFScrollPanel* VGUI_InventoryPanel::GetScrollForStepInput()
{
	if ( mbScrollHighlighted ) return m_Scroll;

	for (int i = 0; i < GearItemButtons.size(); i++)
	{
		if (GearItemButtons[i]->IsMouseOver())
		{
			return m_Scroll;
		}
	}

	if (static_cast<int>(GearItemButtons.size()) > m_Selected && GearItemButtons[m_Selected]->m_ItemContainer)
		return GearItemButtons[m_Selected]->m_ItemContainer->GetScrollForStepInput();
	return nullptr;
}

bool VGUI_InventoryPanel::GearItemDoubleClicked(ulong ID)
{
	if (ID < 0 || ID >= (unsigned)GearItemButtonTotal)
		return true;

	//Item was selected by user click.  The user might be trying to move an item to this pack
	if (m_pCallbackPanel)
		return m_pCallbackPanel->GearItemDoubleClicked(GearItemButtons[ID]->m_GearItemID);

	return false;
}

// wtf is with the overuse of macros here.......
#define GOLDLABEL_SPACER_Y YRES(5)
#define GOLDLABEL_X GEARPNL_X
#define GOLDLABEL_Y (GEARPNL_Y + GEARPNL_SIZE_Y + GOLDLABEL_SPACER_Y)
#define GOLDLABEL_SIZE_X GEARPNL_SIZE_X
#define GOLDLABEL_SIZE_Y YRES(12)

#define INFOPANEL_X	GEARPNL_X
#define INFOPANEL_Y	(GOLDLABEL_Y + GOLDLABEL_SIZE_Y + GOLDLABEL_SPACER_Y)
#define INFOPANEL_SIZE_X GEARPNL_SIZE_X
#define INFOPANEL_SIZE_Y ((ITEM_CONTAINER_Y + ITEM_CONTAINER_SIZE_Y) - INFOPANEL_Y)

//Info panel
VGUI_ItemInfoPanel::VGUI_ItemInfoPanel(Panel *pParent) : CTransparentPanel(INVENTORY_TRANSPARENCY, INFOPANEL_X, INFOPANEL_Y, INFOPANEL_SIZE_X, INFOPANEL_SIZE_Y)
{
	setParent(pParent);

#define INFOPANEL_LABEL_X XRES(0)
#define INFOPANEL_LABEL_SIZE_X getWide()
#define INFOPANEL_LABEL_SIZE_Y YRES(12)
#define INFOPANEL_LABEL_SPACER1_Y YRES(20)
#define INFOPANEL_LABEL_SPACER2_Y YRES(0)

	int iy = 0;
	m_Scroll = new CTFScrollPanel(0, 0, getWide(), getTall());
	m_Scroll->setParent(this);

	// NAME
	iy = INFOPANEL_LABEL_SPACER1_Y;
	m_Name = new MSLabel(m_Scroll->getClient(), "", INFOPANEL_LABEL_X, iy, INFOPANEL_LABEL_SIZE_X, INFOPANEL_LABEL_SIZE_Y, MSLabel::a_center);
	m_Name->SetFGColorRGB(Color_TextHighlighted);

	//WEIGHT
	iy += m_Name->getTall() + INFOPANEL_LABEL_SPACER2_Y;
	m_Weight = new MSLabel(m_Scroll->getClient(), "", INFOPANEL_LABEL_X, iy, INFOPANEL_LABEL_SIZE_X, INFOPANEL_LABEL_SIZE_Y, MSLabel::a_center);
	m_Weight->SetFGColorRGB(Color_TextNormal);

	// QUANTITY
	iy += m_Weight->getTall() + INFOPANEL_LABEL_SPACER2_Y;
	m_Quantity = new MSLabel(m_Scroll->getClient(), "", INFOPANEL_LABEL_X, iy, INFOPANEL_LABEL_SIZE_X, INFOPANEL_LABEL_SIZE_Y, MSLabel::a_center);
	m_Quantity->SetFGColorRGB(Color_TextNormal);

	//QUALITY
	iy += m_Quantity->getTall() + INFOPANEL_LABEL_SPACER2_Y;
	m_Quality = new MSLabel(m_Scroll->getClient(), "", INFOPANEL_LABEL_X, iy, INFOPANEL_LABEL_SIZE_X, INFOPANEL_LABEL_SIZE_Y, MSLabel::a_center);
	m_Quality->SetFGColorRGB(Color_TextNormal);

	// SALES
	iy += m_Quality->getTall() + INFOPANEL_LABEL_SPACER2_Y;
	m_SaleText = new MSLabel(m_Scroll->getClient(), "", INFOPANEL_LABEL_X, iy, INFOPANEL_LABEL_SIZE_X, INFOPANEL_LABEL_SIZE_Y, MSLabel::a_center);
	m_SaleText->SetFGColorRGB(Color_TextNormal);

	m_Scroll->setScrollBarAutoVisible(false, true);
	m_Scroll->setScrollBarVisible(false, false);
	m_Scroll->validate();
}

void VGUI_ItemInfoPanel::Update(containeritem_t &Item)
{
	m_Name->setText(Item.getFullName());

	m_Quantity->setText(Localized("#ITEMINFO_QUANTITY"), Item.Quantity);
	m_Quality->setText(Localized("#ITEMINFO_QUALITY"), Item.Quality);
	m_Quality->setVisible(false);

	char weight[50];
	_snprintf(weight, sizeof(weight), "Weight: %.2f", Item.Weight);
	m_Weight->setText(weight);
	m_Scroll->validate();
}

// Creation
VGUI_ContainerPanel::VGUI_ContainerPanel() : CMenuPanel(100, FALSE, 0, 0, ScreenWidth, ScreenHeight), VGUI_ItemCallbackPanel()
{
	COLOR Color_TitleText = COLOR(255, 100, 100, 0),
		  Color_SubtitleText = COLOR(160, 160, 160, 0),
		  Color_GoldText = COLOR(255, 255, 0, 0),
		  Color_TransparentTextBG = COLOR(0, 0, 0, 255),
		  Color_Red = COLOR(255, 0, 0, 0),
		  Color_White = COLOR(255, 255, 255, 0);

	// Create the title
	m_pTitle = new MSLabel(this, "Inventory", ITEM_CONTAINER_X, 0 + YRES(15));
	m_pTitle->setFont(g_FontTitle);
	m_pTitle->SetFGColorRGB(Color_TitleText);

	// Create the Label
	m_pSubtitle = new MSLabel(this, "", ITEM_CONTAINER_X, 0 + m_pTitle->getTall() + YRES(15));
	m_pSubtitle->SetFGColorRGB(Color_SubtitleText);

	//Create the panel showing all the gear I'm wearing
	m_GearPanel = new VGUI_InventoryPanel(this, this);

	//Create the Info panel
	m_InfoPanel = new VGUI_ItemInfoPanel(this);

	m_ActButton = new MSButton(this, Localized("#REMOVE"), ACTBTN_X, ACTBTN_Y, ACTBTN_SIZE_X, ACTBTN_SIZE_Y);
	m_ActButton->setFont(g_FontID);
	m_ActButton->setContentAlignment(vgui::Label::a_east);
	m_ActButton->SetArmedColor(Color_Red);
	m_ActButton->SetUnArmedColor(Color_White);

	#define BUTTON_CANCEL_SIZE_X XRES(40)
	#define BUTTON_CANCEL_SIZE_Y YRES(15)

	//Create the Gold display labal
	m_GoldLabel = new MSLabel(this, "", GOLDLABEL_X, GOLDLABEL_Y, GOLDLABEL_SIZE_X, GOLDLABEL_SIZE_Y, MSLabel::a_center);
	m_GoldLabel->setFont(g_FontTitle);
	m_GoldLabel->SetFGColorRGB(Color_GoldText);

	// Create the Cancel button
	m_pCancelButton = new MSButton(this, Localized("#CANCEL"), (ITEM_CONTAINER_X + ITEM_CONTAINER_SIZE_X) - BUTTON_CANCEL_SIZE_X, ITEM_CONTAINER_Y - BUTTON_CANCEL_SIZE_Y, BUTTON_CANCEL_SIZE_X, BUTTON_CANCEL_SIZE_Y);
	m_pCancelButton->setFont(g_FontID);
	m_pCancelButton->addActionSignal(new CMenuHandler_TextWindow(HIDE_TEXTWINDOW));
	m_pCancelButton->SetArmedColor(Color_Red);
	m_pCancelButton->SetUnArmedColor(Color_White);

	m_AllowUpdate = false;
}

// Update
void VGUI_ContainerPanel::Update()
{
	if (!m_AllowUpdate)
		return;

	m_GearPanel->Reset();
	AddInventoryItems();

	//Move the Cancel button on top
	removeChild(m_pCancelButton);
	m_pCancelButton->setParent(this);

	char sGold[255];
	snprintf(sGold, 255, Localized("#PLAYER_GOLD"), player.m_Gold);
	m_GoldLabel->setText(sGold);
}

void VGUI_ContainerPanel::AddInventoryItems()
{
	gearitem_t GearItem;

	GearItem.Name = Localized("#PLAYER_HANDS");
	GearItem.ID = 0;
	GearItem.IsContainer = true;
	VGUI_Inv_GearItem *pGearItemButton = m_GearPanel->AddGearItem(GearItem);
	for (int i = 0; i < MAX_PLAYER_HANDS; i++)
	{
		if (player.Hand(i))
		{
			containeritem_t item(player.Hand(i));
			pGearItemButton->m_ItemContainer->AddItem(item);
		}
	}

	const int gearSize = player.Gear.size();
	std::vector<CGenericItem *> sorted;
	sorted.reserve(gearSize);

	for (int g = 0; g < gearSize; g++)
	{
		CGenericItem *pGearItem = player.Gear[g];
		if (!FBitSet(pGearItem->MSProperties(), ITEM_CONTAINER) && pGearItem->m_Location <= ITEMPOS_HANDS)
			continue;

		if (FBitSet(pGearItem->MSProperties(), ITEM_CONTAINER))
			sorted.push_back(pGearItem);
	}

	for (int g = 0; g < gearSize; g++)
	{
		CGenericItem *pGearItem = player.Gear[g];
		if (!FBitSet(pGearItem->MSProperties(), ITEM_CONTAINER) && pGearItem->m_Location <= ITEMPOS_HANDS)
			continue;

		if (!FBitSet(pGearItem->MSProperties(), ITEM_CONTAINER))
			sorted.push_back(pGearItem);
	}

	for (int i = 0; i < sorted.size(); i++)
	{
		CGenericItem *pGearItem = sorted[i];
		const auto props = pGearItem->MSProperties(); // cache property lookup
		GearItem.Name = pGearItem->DisplayName();
		GearItem.ID = pGearItem->m_iId;
		GearItem.IsContainer = FBitSet(props, ITEM_CONTAINER) ? true : false;
		pGearItemButton = m_GearPanel->AddGearItem(GearItem);

		if (GearItem.IsContainer)
		{
			const int itemCount = pGearItem->Container_ItemCount();
			for (int j = 0; j < itemCount; j++)
			{
				containeritem_t item(pGearItem->Container_GetItem(j));
				pGearItemButton->m_ItemContainer->AddItem(item);
			}
		}
	}
}

// MiB FEB2015_07 - Inventory type buttons
class InvTypeButtonSignal : public ActionSignal
{
private:
	int m_iInvType;
	class VGUI_Container *m_pCallbackPanel;

public:
	InvTypeButtonSignal(int iInvType, VGUI_Container *pCallbackPanel) : ActionSignal()
	{
		m_iInvType = iInvType;
		m_pCallbackPanel = pCallbackPanel;
	}

	virtual void actionPerformed(Panel *panel)
	{
		gEngfuncs.Cvar_SetValue("ms_invtype", m_iInvType);
		m_pCallbackPanel->Update();
		m_pCallbackPanel->m_pScrollPanel->setScrollValue(0, 0); // Reset scroll bar
		m_pCallbackPanel->m_CallbackPanel->InvTypeChanged( m_iInvType );
	}
};

// MIB FEB2019_25 [ALPHABETICAL_INVENTORY]
class AlphabetizeCheckBoxHandler : public VGUI_CheckBoxHandler
{
private:
	class VGUI_Container *m_pCallbackPanel;
public:
	AlphabetizeCheckBoxHandler( VGUI_Container *pCallbackPanel )
	{
		m_pCallbackPanel = pCallbackPanel;
	}

	virtual void CheckChange(VGUI_CheckBox *pButton)
	{
		gEngfuncs.Cvar_SetValue("ms_alpha_inventory", pButton->IsChecked() ? 1 : 0);
		m_pCallbackPanel->Update();
		m_pCallbackPanel->m_CallbackPanel->AlphabeticChanged(pButton->IsChecked());
	}
};

// MIB FEB2019_25 [ALPHABETICAL_INVENTORY]
#define INVTYPE_PANEL_X	ITEM_CONTAINER_X
#define INVTYPE_PANEL_Y	(ITEM_CONTAINER_Y + ITEM_CONTAINER_SIZE_Y) + YRES(10)
#define INVTYPE_PANEL_SIZE_X ITEM_CONTAINER_SIZE_X - ACTBTN_SIZE_X
#define INVTYPE_PANEL_SIZE_Y YRES(64)
#define INVTYPE_BUTTON_SIZE_X XRES(80)
#define INVTYPE_BUTTON_SIZE_Y YRES(15)
#define INVTYPE_BUTTON_X_SPACER	XRES(12)
#define INVTYPE_BUTTON_Y_SPACER	YRES(12)

// MiB FEB2015_07 - Inventory type buttons
VGUI_InvTypePanel::VGUI_InvTypePanel(Panel *pParent, VGUI_Container *pCallback) : CTransparentPanel(0, INVTYPE_PANEL_X, INVTYPE_PANEL_Y, INVTYPE_PANEL_SIZE_X, INVTYPE_PANEL_SIZE_Y)
{
	setParent(pParent);
	const char ButtonText[INVTYPE_BUTTONS_TOTAL][16] = {"Tiled", "Small", "Descriptions"};

	pAlphaCheckBox = new VGUI_CheckBox("Alphabetic", INVTYPE_BUTTON_X_SPACER, INVTYPE_BUTTON_SIZE_Y + INVTYPE_BUTTON_Y_SPACER, INVTYPE_BUTTON_SIZE_X, INVTYPE_BUTTON_SIZE_Y, pCallback->IsAlphabetical());
	pAlphaCheckBox->setVisible(true);
	pAlphaCheckBox->SetHandler(new AlphabetizeCheckBoxHandler(pCallback));
	addChild(pAlphaCheckBox);

	for (int i = 0; i < INVTYPE_BUTTONS_TOTAL; i++)
	{
		int x = 0, y;

		if (i)
		{
			ClassButton *pLast = InvTypeButtons[i-1];
			pLast->getPos(x, y);
			x += pLast->getWide();
		}

		ClassButton *pCur = new ClassButton(0, ButtonText[i], x + INVTYPE_BUTTON_X_SPACER, 0, INVTYPE_BUTTON_SIZE_X, INVTYPE_BUTTON_SIZE_Y, false);
		pCur->setContentAlignment(vgui::Label::Alignment::a_center);
		pCur->setVisible(true);
		pCur->addActionSignal(new InvTypeButtonSignal(i, pCallback));
		InvTypeButtons.add(pCur);
		addChild(pCur);
	}

	setVisible(true);
}

void VGUI_ContainerPanel::Open(void)
{
	// Update before opening
	m_AllowUpdate = true;
	Update();
	CMenuPanel::Open();
}

void VGUI_ContainerPanel::Close(void)
{
	player.BlockButton(IN_ATTACK);
	m_pCancelButton->setArmed(false); //If the user presses cancel, the cancel button doesn't automaticaly get unarmed... so manually do it
	m_ActButton->setArmed(false);
	CMenuPanel::Close();
	m_AllowUpdate = false;
}

bool VGUI_ContainerPanel::SlotInput(int iSlot)
{
	iSlot--;

	if (iSlot < 0 || iSlot >= m_GearPanel->GearItemButtonTotal)
		return false;

	m_GearPanel->Select(iSlot);

	return true;
}
//-----------------------------------------------------------------------------
// Purpose: Called each time a new level is started.
//-----------------------------------------------------------------------------
void VGUI_ContainerPanel::Initialize(void)
{
	setVisible(false);
}

//======================================
// Mouse is over a button, bring up the info
void VGUI_ContainerPanel::ItemHighlighted(void *pData)
{
	VGUI_ItemButton &ItemButton = *(VGUI_ItemButton *)pData;
	if (!ItemButton.m_Highlighted)
		return;

	m_InfoPanel->Update(ItemButton.m_Data);
}

bool VGUI_ContainerPanel::HasSelectedItems()
{
	for(int i = 0; i < m_GearPanel->GearItemButtonTotal; i++)
	{
		VGUI_Inv_GearItem &GearItem = *m_GearPanel->GearItemButtons[i];
		for(int g = 0; g < GearItem.m_ItemContainer->m_ItemButtonTotal; g++)
		{
			VGUI_ItemButton &ItemButton = *GearItem.m_ItemContainer->m_ItemButtons[g];
			if(ItemButton.m_Selected) 
				return true;
		}
	}

    return false;
}

void VGUI_ContainerPanel::GetSelectedItems(mslist<VGUI_ItemButton *> &SelectedItems)
{
	SelectedItems.clear();

	for (int g = 0; g < m_GearPanel->GearItemButtonTotal; g++)
	{
		VGUI_Inv_GearItem &GearItem = *m_GearPanel->GearItemButtons[g];
		for (int i = 0; i < GearItem.m_ItemContainer->m_ItemButtonTotal; i++)
		{
			VGUI_ItemButton &ItemButton = *GearItem.m_ItemContainer->m_ItemButtons[i];
			if (!ItemButton.m_Selected)
				continue;

			SelectedItems.push_back(&ItemButton);
		}
	}
}
void VGUI_ContainerPanel::UnSelectAllItems()
{
	mslist<VGUI_ItemButton *> SelectedItems;
	GetSelectedItems(SelectedItems);
	for (int i = 0; i < SelectedItems.size(); i++)
		SelectedItems[i]->Select(false);
}