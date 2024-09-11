//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: NPC Store sell menu
//
// $Workfile:     $
// $Date: 2005/01/17 13:16:49 $
//
//-----------------------------------------------------------------------------
// $Log: vgui_Storage.cpp,v $
// Revision 1.1  2005/01/17 13:16:49  dogg
// Brand new inventory VGUI, revised item system, Magic, Mp3 support and item storage
//
// Revision 1.4  2004/11/07 01:06:28  dogg
// New Event Console
//
// Revision 1.3  2004/09/18 10:41:53  dogg
// Magic update
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

#include "inc_weapondefs.h"
#undef DLLEXPORT

#include "../hud.h"
#include "../cl_util.h"
#include "../parsemsg.h"
#include "../vgui_teamfortressviewport.h"

//Master Sword
#include "vgui_storage.h"
#include "vgui_menudefsshared.h"
#include "logger.h"

msstring_ref CStoragePanel::Text_Subtitle_Storage = "#STORAGE_SUBTITLE_STORAGE";
msstring_ref CStoragePanel::Text_Subtitle_Inventory = "#STORAGE_SUBTITLE_INVENTORY";

/*class CAction_Take : public ActionSignal
{
protected:
	CStoragePanel *m_Panel;
public:
	CAction_Take( CStoragePanel *pPanel ) { m_Panel = pPanel; }
	virtual void actionPerformed(Panel* panel) { m_Panel->TakeAll( ); }
};*/

//------------

// Creation
CStoragePanel::CStoragePanel(Panel *pParent) : CStorePanel()
{
	setParent(pParent);

	m_pSubtitle->setText(Localized(Text_Subtitle_Storage));

	m_ActButton->setVisible(false);
	m_pCancelButton->setText(Localized("#CLOSE"));

	/*m_ActButton->setText( Localized("#TAKE") );
	m_ActButton->addActionSignal( new CAction_Take(this) );*/
}

//Update
void CStoragePanel::AddInventoryItems()
{
	//Find the storage on the player
	storage_t *pStorage = NULL;
	for (int s = 0; s < player.m_Storages.size(); s++)
		if (player.m_Storages[s].Name == player.m_CurrentStorage.StorageName)
		{
			pStorage = &player.m_Storages[s];
			break;
		}

	if (!pStorage)
	{
		//If the storage doesn't exist, create it real quick.  This should never happen.
		storage_t Storage;
		Storage.Name = player.m_CurrentStorage.StorageName;
		pStorage = player.Storage_CreateAccount(Storage);
	}

	//Add the 'storage' container
	gearitem_t GearItem;
	GearItem.Name = "Storage";
	GearItem.ID = 1;

	VGUI_Inv_GearItem *pGearItemButton = m_GearPanel->AddGearItem(GearItem);

	//Add the storage items
	storage_t &Storage = *pStorage;
	for (int i = 0; i < Storage.Items.size(); i++)
	{
		containeritem_t vNewItem = containeritem_t(Storage.Items[i]);
		pGearItemButton->m_ItemContainer->AddItem(vNewItem);
	}

	//Add my local inventory items
	CStorePanel::AddInventoryItems();

	return;
}
void CStoragePanel::Update()
{
	if (!player.m_CurrentStorage.Active)
		return;

	m_LastGearItem = -1;
	m_SelectedItems.clear();
	m_SaleLabel->setVisible(false);

	CStorePanel::Update();

	m_pTitle->setText(player.m_CurrentStorage.DisplayName);
}

void CStoragePanel::GearItemSelected(ulong ID)
{
	if (m_LastGearItem == ID)
		return;

	//Set the proper subtitle
	if (ID == 1)
	{
		//Unselect all items when going to storage from inventory
		UnSelectAllItems();
		m_pSubtitle->setText(Localized(Text_Subtitle_Storage));
	}
	else
	{
		if (m_LastGearItem == 1)
		{
			//Unselect all items when going to inventory from storage
			UnSelectAllItems();
		}
		m_pSubtitle->setText(Localized(Text_Subtitle_Inventory));
	}

	m_LastGearItem = ID;
}
bool CStoragePanel::GearItemClicked(ulong ID)
{
	if (!m_SelectedItems.size())
		return false;

	//If the last gearitem and this one are f the same category (storage or inventory), don't try to move the items
	if ((m_LastGearItem == 1) == (ID == 1))
		return false;

	m_InfoPanel->setVisible(false);

	//Move the selected items either from inventory to storage, or storage to inventory
	bool AddOrRemove = (ID == 1) ? true : false;
	msstring Command;

	bool leftAnItemOff = false;

	for (int i = 0; i < m_SelectedItems.size(); i++)
	{
		if (checkValid(m_SelectedItems[i])) //Check it against the bank mask (MiB Feb2008a)
		{
			//Send the command
			if (AddOrRemove)
				Command = msstring("storage add ") + (int)m_SelectedItems[i].ID + "\n";
			else
				Command = msstring("storage remove ") + (int)m_SelectedItems[i].ID + " " + (int)ID + "\n";

			gEngfuncs.pfnClientCmd(Command);
		}
		else
			leftAnItemOff = true;
	}

	return true;
}

//MiB FEB2008a
bool CStoragePanel::checkValid(containeritem_t &item)
{
	msstring name = item.Name.m_string;
	if (name.starts_with("pack_"))
		return false;
	if (name.starts_with("sheath_"))
		return false;
	if (name.starts_with("mana_"))
		return false;
	if (name.starts_with("health_"))
		return false;
	if (name.starts_with("proj_"))
		return false;

	return true;
}

//Item selected
int CStoragePanel::ItemRetrievalCost(containeritem_t &Item)
{
	return int(Item.Value * player.m_CurrentStorage.flFeeRatio);
}
void CStoragePanel::ItemHighlighted(void *pData)
{
	CStorePanel::ItemHighlighted(pData);

	if (m_LastGearItem != 1)
	{
		m_InfoPanel->m_SaleText->setVisible(false);
		m_InfoPanel->setVisible(true);
		return;
	}

	VGUI_ItemButton &ItemButton = *(VGUI_ItemButton *)pData;
	if (ItemButton.m_Highlighted)
	{
		int Value = ItemRetrievalCost(ItemButton.m_Data);
		if (Value)
		{
			char cTemp[256];
			 _snprintf(cTemp, sizeof(cTemp),  Localized("#STORAGE_ITEM_COST"),  Value );
			m_InfoPanel->m_SaleText->setText(cTemp);
		}
		else
			m_InfoPanel->m_SaleText->setText(Localized("#STORAGE_ITEM_FREE"));
		m_InfoPanel->m_SaleText->setVisible(true);
	}
	m_InfoPanel->m_Scroll->validate();
	m_InfoPanel->setVisible(true);
}

//Item selected
void CStoragePanel::ItemSelectChanged(ulong ID, bool fSelected)
{
	m_SelectedItems.clear();

	m_SaleLabel->setVisible(true);

	int Valuetotal = 0;

	for (int g = 0; g < m_GearPanel->GearItemButtonTotal; g++)
	{
		VGUI_Inv_GearItem &GearItem = *m_GearPanel->GearItemButtons[g];
		for (int i = 0; i < GearItem.m_ItemContainer->m_ItemButtonTotal; i++)
		{
			VGUI_ItemButton &ItemButton = *GearItem.m_ItemContainer->m_ItemButtons[i];
			if (!ItemButton.m_Selected)
				continue;

			m_SelectedItems.add(ItemButton.m_Data);

			if (m_LastGearItem == 1)
				Valuetotal += ItemRetrievalCost(ItemButton.m_Data);
		}
	}

	if (m_SelectedItems.size())
	{
		if (m_LastGearItem == 1)
			m_SaleLabel->setText(msstring("Removing ") + (int)m_SelectedItems.size() + " items from storage for " + Valuetotal + " gold");
		else
			m_SaleLabel->setText(msstring("Moving ") + (int)m_SelectedItems.size() + " items into storage");
		m_SaleLabel->setVisible(true);
	}
	else
		m_SaleLabel->setVisible(false);
}

//Take button pressed
/*void CStoragePanel::TakeAll( )
{
	msstring CommandString = "trade stop"; //If no items selected

	if( m_SelectedItems.size() > 0 )
	{
		CommandString = "trade sell";
		 for (int i = 0; i < m_SelectedItems.size(); i++) 
		{
			CommandString += " ";
			CommandString += (int)m_SelectedItems[i].ID;
		}
		CommandString += "\n";
	}

	gEngfuncs.pfnClientCmd( CommandString );
}*/

void CStoragePanel::Close()
{
	player.m_CurrentStorage.Active = false;
	ClientCmd("storage stop");
	VGUI_ContainerPanel::Close(); //Skip CStorePanel::Close()
}

void Storage_Update()
{
	if (gViewPort && gViewPort->m_pStoreStorageMenu)
	{
		gViewPort->m_pStoreStorageMenu->m_AllowUpdate = true;
		gViewPort->m_pStoreStorageMenu->Update();
	}
}

//Item msg
void Storage_ItemReset()
{
	player.m_Storages.clear();
	Storage_Update();
}

void Storage_ItemMsg()
{
	msstring StorageName = READ_STRING();
	storage_t *pStorage = NULL;

	for (int s = 0; s < player.m_Storages.size(); s++)
		if (player.m_Storages[s].Name == StorageName)
		{
			pStorage = &player.m_Storages[s];
			break;
		}

	if (!pStorage)
	{
		storage_t Storage;
		Storage.Name = StorageName;
		pStorage = player.Storage_CreateAccount(Storage);
	}

	bool AddItem = READ_BYTE() ? true : false;

	if (AddItem)
	{
		CGenericItem *pItem = ReadGenericItem(true);
		if (pItem)
			pStorage->Items.add(genericitem_full_t(pItem));

		pItem->SUB_Remove();
	}

	Storage_Update();
}

void Storage_Show(msstring_ref DisplayName, msstring_ref StorageName, float flFeeRatio)
{
	player.m_CurrentStorage.Active = true;
	player.m_CurrentStorage.DisplayName = DisplayName;
	player.m_CurrentStorage.StorageName = StorageName;
	player.m_CurrentStorage.flFeeRatio = flFeeRatio;
	ShowVGUIMenu(MENU_STORAGE);
}