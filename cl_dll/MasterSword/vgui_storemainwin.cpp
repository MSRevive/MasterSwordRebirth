//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: TFC Class Menu
//
// $Workfile:     $
// $Date: 2005/01/17 13:16:49 $
//
//-----------------------------------------------------------------------------
// $Log: vgui_StoreMainwin.cpp,v $
// Revision 1.3  2005/01/17 13:16:49  dogg
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

#include "inc_weapondefs.h"
#undef DLLEXPORT

#include "../hud.h"
#include "../cl_util.h"
#include "../parsemsg.h"

#include "../vgui_TeamFortressViewport.h"
#include "../vgui_ServerBrowser.h"

//Master Sword
#include "vgui_StoreMainwin.h"

msstring CStorePanel::StoreVendorName;
mslist<storeitem_t> CStorePanel::StoreItems;
unsigned long CStorePanel::StoreGold;
int CStorePanel::iStoreBuyFlags, CStorePanel::StoreItemMsgCount;
string_i CStorePanel::Text_StoreGold;

static COLOR Color_TitleText = COLOR(255, 0, 0, 0), //255, 100, 100
	Color_SubtitleText = COLOR(160, 160, 160, 0),
			 Color_TransparentTextBG = COLOR(0, 0, 0, 255);

// Creation
CStorePanel::CStorePanel() : VGUI_ContainerPanel()
{
#define TITLE_X ITEM_CONTAINER_X
#define TITLE_Y YRES(48)

//Create the sell calculation
#define SELLLABEL_X ITEM_CONTAINER_X
#define SELLLABEL_Y ITEM_CONTAINER_Y + ITEM_CONTAINER_SIZE_Y

	m_SaleLabel = new MSLabel(this, "", SELLLABEL_X, SELLLABEL_Y);
	m_SaleLabel->setFont(g_FontTitle);
	m_SaleLabel->SetFGColorRGB(Color_SubtitleText);
	m_SaleLabel->setText("Selling 0 Items (0 Gold)");

	//Set up the cancel button
	m_pCancelButton->addActionSignal(new CMenuHandler_StringCommand("trade stop", TRUE));
	m_pCancelButton->setText(Localized("#CLOSE"));

	Text_StoreGold = "GOLD";

	setVisible(false);
}

// Update
void CStorePanel::Update()
{
	//Special circumstance: inventory items for packs not being held (STORE_INV)
	if (FBitSet(iStoreBuyFlags, STORE_INV))
		m_pTitle->setText(StoreVendorName);
	else
		m_pTitle->setText("%s's Shop", StoreVendorName.c_str());

	VGUI_ContainerPanel::Update();
}
// Close
void CStorePanel::Close()
{
	ClientCmd("trade stop");
	StoreItems.clear();
	VGUI_ContainerPanel::Close();
}

void MsgFunc_Store()
{
	CStorePanel::iStoreBuyFlags = READ_BYTE();
	CStorePanel::StoreVendorName = READ_STRING();
	CStorePanel::StoreItemMsgCount = READ_BYTE();
	CStorePanel::StoreGold = 0;
	CStorePanel::StoreItems.clear();
}

void Update_StoreBuy();
void Storage_ItemReset();
void Storage_ItemMsg();

int __MsgFunc_StoreItem(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	byte Type = READ_BYTE();

	switch (Type)
	{
	case 0:
		if ((signed)CStorePanel::StoreItems.size() < CStorePanel::StoreItemMsgCount)
		{
			string_t ItemName = ALLOC_STRING(READ_STRING());
			CGenericItem *pItem = NewGenericItem(STRING(ItemName));
			if (!pItem)
				return 0;

			storeitem_t StoreItem(pItem);
			pItem->SUB_Remove();

			//Need this reminder, because people forget to set the trade sprite
			if (!StoreItem.SpriteName)
				Print("Erorr: Store Item %s has no trade sprite\n", StoreItem.Name.c_str());

			StoreItem.ID = CStorePanel::StoreItems.size();
			StoreItem.Quantity = READ_SHORT();
			StoreItem.iCost = READ_LONG(); //MiB JAN2010 - Shop Prices over 32k.rtf
			StoreItem.flSellRatio = READ_SHORT() * 0.01;
			StoreItem.iBundleAmt = READ_SHORT();

			CStorePanel::StoreItems.push_back(StoreItem);

			Update_StoreBuy();
			return 1;
		}

		if (!strcmp(READ_STRING(), "gold"))
		{
			CStorePanel::StoreGold = READ_LONG();
			Update_StoreBuy();
			return 1;
		}
		break;
	case 1:
		Storage_ItemReset();
		break;
	case 2:
		Storage_ItemMsg();
		break;
	}
	return 0;
}
