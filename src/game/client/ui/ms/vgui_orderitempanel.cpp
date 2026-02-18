#include "../hud.h"
#include "../cl_util.h"
#include "vgui_teamfortressviewport.h"
#include "sharedutil.h"
#include "vgui_orderitempanel.h"
#include "vgui_container.h"

VGUI_OrderItemPanel::VGUI_OrderItemPanel(VGUI_ContainerPanel *pParent) : CTransparentPanel(80, 0, 0, 0, 0)
{
	vgui::Font *pFont = g_FontSml;
	int vButtonHeight = g_FontSml->getTall();
	int y = 0;
	int w = 0;

	//for(int i = 0; i < gNumMoveNumbers + 1; i++)
	//{
	//	int vNumMove;
	//	msstring vsText;
	//	if (i == gNumMoveNumbers)
	//	{
	//		vNumMove = 0;
	//		vsText = "Cancel";
	//	}
	//	// else
	//	// {
	//	// 	vNumMove = gMoveNumbers[i];
	//	// 	vsText = msstring("Move ") + vNumMove + " To Hands";
	//	// }

	//	int vButtonW;
	//	int vButtonH;
	//	pFont->getTextSize(vsText, vButtonW, vButtonH);
	//	MSButton *pButton = new MSButton(this, "", XRES(5), y + YRES(5), vButtonW, vButtonH, COLOR(255, 0, 0, 0), COLOR(100, 100, 100, 0));
	//	pButton->setText(vsText);
	//	pButton->setFont(pFont);
	//	pButton->setContentAlignment(vgui::Label::Alignment::a_west);
	//	pButton->addActionSignal(new CMoveItemSignal(this, vNumMove));
	//	mButtons.add(pButton);
	//	pButton->setVisible(true);

	//	y += vButtonH;
	//	w = V_max(w, vButtonW);
	//}

	int vButtonW;
	int vButtonH;
	pFont->getTextSize("Cancel", vButtonW, vButtonH);
	MSButton *pButton = new MSButton(this, "", XRES(5), y + YRES(5), vButtonW, vButtonH, COLOR(255, 0, 0, 0), COLOR(100, 100, 100, 0));
	pButton->setText("Cancel");
	pButton->setFont(pFont);
	pButton->setContentAlignment(vgui::Label::Alignment::a_west);
	pButton->addActionSignal(new COrderItemSignal(this, 0));
	mButtons.add(pButton);
	pButton->setVisible(true);

	y += vButtonH;
	w = V_max(w, vButtonW);

	setSize(w + XRES(10), y + YRES(10));
	setParent(mpParent = pParent);
	setVisible(false);
	setBorder(new LineBorder(2, Color(0,128,0,128)));
}

void VGUI_OrderItemPanel::ShowForButton(VGUI_ItemButton *pButton)
{
	mpCurrentItemButton = pButton;
	int x, y, w, h;

	// Move to be over the clicked button
	pButton->getAbsExtents(x, y, w, h);
	setPos(x,y);

	/*for(int i = 0; i < gNumMoveNumbers; i++)
	{
		mButtons[i]->setEnabled(pButton->m_Data.Quantity >= gMoveNumbers[i]);
	}*/

	setVisible(true);
}

void VGUI_OrderItemPanel::Move(int vNumMove)
{
	/*if (vNumMove) 
		mpParent->MoveItem(mpCurrentItemButton, vNumMove);*/

	setVisible(false);
}

void VGUI_OrderItemPanel::setVisible(bool bVisible)
{
	CTransparentPanel::setVisible(bVisible);
	/*for(int i = 0; i < gNumMoveNumbers + 1; i++)
	{
		mButtons[i]->setArmed(false);
	}*/
}

COrderItemSignal::COrderItemSignal(VGUI_OrderItemPanel *pOrderItemPanel, int vNumMove)
{
	mpOrderItemPanel = pOrderItemPanel;
	m_iOrder = vNumMove;
}

void COrderItemSignal::actionPerformed(Panel *pPanel)
{
	mpOrderItemPanel->Move(m_iOrder);
}