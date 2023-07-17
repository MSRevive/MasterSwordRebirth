//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: VGUI Shop screen
//
//=============================================================================

#include "inc_weapondefs.h"

#include "../hud.h"
#include "../cl_util.h"
#include "../vgui_teamfortressviewport.h"
	
//Master Sword
#include "vgui_storebuy.h"
#include "logger.h"

//------------
const char *CStoreBuyPanel::Text_BuySubtitle = "#BUY_SUBTITLE";
const char *CStoreBuyPanel::Text_InvSubtitle = "#INV_SUBTITLE";

// Creation
CStoreBuyPanel::CStoreBuyPanel(int iTrans, int iRemoveMe, int x, int y, int wide, int tall) : CStorePanel()
{
	m_ActButton->setVisible(false);
	m_SaleLabel->setVisible(false);
}

// Update
void CStoreBuyPanel::AddInventoryItems()
{
	gearitem_t GearItem;
	GearItem.Name = "Store";
	GearItem.ID = 0;

	//Initialize one container
	VGUI_Inv_GearItem *pGearItemButton = m_GearPanel->AddGearItem(GearItem);
	pGearItemButton->setVisible(false);

	//Add store items with at least 1 in inventory to the container
	for (int i = 0; i < StoreItems.size(); i++)
		if (StoreItems[i].Quantity > 0)
			pGearItemButton->m_ItemContainer->AddItem(StoreItems[i]);

	//Add a 'gold' item, if this is a chest with gold in it
	if (StoreGold)
	{
		storeitem_t GoldItem;
		clrmem(GoldItem);
		GoldItem.Name = Text_StoreGold;
		GoldItem.setFullName(msstring("") + (int)StoreGold + " gold coins"); // MiB FEB2015_07 - Updated to play nice with new FullName things
		GoldItem.SpriteName = "items/640_gold";
		pGearItemButton->m_ItemContainer->AddItem(GoldItem);
	}

	if (FBitSet(iStoreBuyFlags, STORE_INV))
		m_pSubtitle->setText(Localized(Text_InvSubtitle));
	else
		m_pSubtitle->setText(Localized(Text_BuySubtitle));

	return;
}
bool CStoreBuyPanel::ItemClicked(void *pData)
{
	VGUI_ItemButton &ItemButton = *(VGUI_ItemButton *)pData;

	msstring Command = msstring("trade buy ") + ItemButton.m_Data.Name + "\n";
	ClientCmd(Command);
	Close();
	return true;
}
void CStoreBuyPanel::ItemHighlighted(void *pData)
{
	CStorePanel::ItemHighlighted(pData);

	VGUI_ItemButton &ItemButton = *(VGUI_ItemButton *)pData;
	if (ItemButton.m_Highlighted)
	{
		if (ItemButton.m_Data.Name == STRING(Text_StoreGold))
		{
			m_InfoPanel->m_Quantity->setVisible(false);
			m_InfoPanel->m_SaleText->setVisible(false);
		}
		else
		{
			m_InfoPanel->m_SaleText->setText("Price");

			containeritem_t &Item = ItemButton.m_Data;
			for (int s = 0; s < CStorePanel::StoreItems.size(); s++)
			{
				storeitem_t &StoreItem = CStorePanel::StoreItems[s];
				if (Item.Name != StoreItem.Name)
					continue;

				char cTemp[256];
				 _snprintf(cTemp, sizeof(cTemp),  Localized("#ITEM_COST"),  StoreItem.iCost );
				m_InfoPanel->m_SaleText->setText(cTemp);
				break;
			}

			m_InfoPanel->m_SaleText->setVisible(true);
			m_InfoPanel->m_Quantity->setVisible(true);
		}
	}
}

void Update_StoreBuy()
{
	if (gViewPort->m_pStoreBuyMenu)
	{
		gViewPort->m_pStoreBuyMenu->m_AllowUpdate = true;
		gViewPort->m_pStoreBuyMenu->Update();
	}
}
