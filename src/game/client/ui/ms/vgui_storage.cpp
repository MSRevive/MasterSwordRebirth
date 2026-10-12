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

const char* CStoragePanel::Text_Subtitle_Storage = "#STORAGE_SUBTITLE_STORAGE";
const char* CStoragePanel::Text_Subtitle_Inventory = "#STORAGE_SUBTITLE_INVENTORY";

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
	//Add the 'storage' container
	gearitem_t GearItem;
	GearItem.Name = "Storage";
	GearItem.ID = 1;

	VGUI_Inv_GearItem *pGearItemButton = m_GearPanel->AddGearItem(GearItem);

	//Add the storage items
	for (int i = 0; i < player.m_Storage.size(); i++)
	{
		storageitem_t &Item = player.m_Storage[i];

		//Storage only knows the item's name, so the rest comes from the global item
		CGenericItem *pGlobalItem = CGenericItemMgr::GetGlobalGenericItemByName(Item.Name, true);
		if (!pGlobalItem)
			continue;

		containeritem_t vNewItem = containeritem_t(pGlobalItem);
		vNewItem.ID = Item.ID;
		vNewItem.Quantity = Item.Quantity;
		vNewItem.Quality = Item.Quality;
		if (CBasePlayer::Storage_CombinesSwigs(Item.Name.c_str(), vNewItem.Properties))
			vNewItem.MaxQuality = STORAGE_MAX_SWIGS; //Shown as how full it is, instead of against one potion
		else if (Item.MaxQuality)
			vNewItem.MaxQuality = Item.MaxQuality;
		if (Item.Quantity > 1)
			vNewItem.setFullName(msstring() + (int)Item.Quantity + " " + pGlobalItem->DisplayName() + "s"); //Named the way SPEECH::ItemName does

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

	m_pTitle->setText(player.m_CurrentStorage.DisplayName + " (" + player.Storage_ItemCount() + "/" + STORAGE_MAX_ITEMS + ")");
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
		if (!AddOrRemove || checkValid(m_SelectedItems[i])) //Check it against the bank mask (MiB Feb2008a)
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

void CStoragePanel::ItemDoubleclicked(ulong ID)
{
	//Double clicking an item in storage takes it out and puts it in my hands
	if (m_LastGearItem != 1)
		return;

	UnSelectAllItems();
	m_InfoPanel->setVisible(false);

	msstring Command = msstring("storage remove ") + (int)ID + " hands\n";
	gEngfuncs.pfnClientCmd(Command);
}

bool CStoragePanel::ItemClicked(void *pData)
{
	//Only so many items can be moved at once, so don't let more than that be selected
	VGUI_ItemButton &ItemButton = *(VGUI_ItemButton *)pData;
	return !ItemButton.m_Selected && (int)m_SelectedItems.size() >= STORAGE_MAX_TRANSFER;
}

//MiB FEB2008a
bool CStoragePanel::checkValid(containeritem_t &item)
{
	//The server checks this too
	return CBasePlayer::Storage_CanStoreItem(item.Name.c_str(), item.Properties);
}

void CStoragePanel::ItemHighlighted(void *pData)
{
	CStorePanel::ItemHighlighted(pData);

	m_InfoPanel->m_SaleText->setVisible(false);
	m_InfoPanel->setVisible(true);
}

//Item selected
void CStoragePanel::ItemSelectChanged(ulong ID, bool fSelected)
{
	m_SelectedItems.clear();

	m_SaleLabel->setVisible(true);

	for (int g = 0; g < m_GearPanel->GearItemButtonTotal; g++)
	{
		VGUI_Inv_GearItem &GearItem = *m_GearPanel->GearItemButtons[g];
		for (int i = 0; i < GearItem.m_ItemContainer->m_ItemButtonTotal; i++)
		{
			VGUI_ItemButton &ItemButton = *GearItem.m_ItemContainer->m_ItemButtons[i];
			if (!ItemButton.m_Selected)
				continue;

			m_SelectedItems.add(ItemButton.m_Data);
		}
	}

	if (m_SelectedItems.size())
	{
		msstring Text;
		if (m_LastGearItem == 1)
			Text = msstring("Removing ") + (int)m_SelectedItems.size() + " items from storage";
		else
			Text = msstring("Moving ") + (int)m_SelectedItems.size() + " items into storage";

		if ((int)m_SelectedItems.size() >= STORAGE_MAX_TRANSFER)
			Text += " (max)";

		m_SaleLabel->setText(Text);
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
		gViewPort->m_pStoreStorageMenu->Update();
	}
}

//Item msg
void Storage_ItemReset()
{
	player.m_Storage.clear();
	Storage_Update();
}

//Items were added to my storage
void Storage_ItemMsg()
{
	int Count = READ_BYTE();
	for (int i = 0; i < Count; i++)
	{
		storageitem_t Item;
		Item.ID = READ_LONG();
		int idx = READ_LONG();
		Item.Quantity = READ_SHORT();
		Item.Quality = READ_SHORT();
		Item.MaxQuality = READ_SHORT();

		//Everything is read before the item is checked, so the next item can still be read
		if (idx < 0 || idx >= CGenericItemMgr::ItemCount())
			continue;

		Item.Name = CGenericItemMgr::Item(idx)->Name;
		player.m_Storage.add(Item);
	}

	Storage_Update();
}

//An item was removed from my storage
void Storage_ItemRemoveMsg()
{
	ulong ID = READ_LONG();
	for (int i = 0; i < player.m_Storage.size(); i++)
		if (player.m_Storage[i].ID == ID)
		{
			player.m_Storage.erase(i);
			break;
		}

	Storage_Update();
}

//The quantity or quality of an item in my storage changed
void Storage_ItemUpdateMsg()
{
	ulong ID = READ_LONG();
	int Quantity = READ_SHORT();
	int Quality = READ_SHORT();
	for (int i = 0; i < player.m_Storage.size(); i++)
		if (player.m_Storage[i].ID == ID)
		{
			player.m_Storage[i].Quantity = Quantity;
			player.m_Storage[i].Quality = Quality;
			break;
		}

	Storage_Update();
}

void Storage_Show(const char* DisplayName)
{
	player.m_CurrentStorage.Active = true;
	player.m_CurrentStorage.DisplayName = DisplayName;
	ShowVGUIMenu(MENU_STORAGE);
}