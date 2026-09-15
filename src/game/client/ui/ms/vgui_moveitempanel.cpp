#include "../hud.h"
#include "../cl_util.h"
#include "vgui_teamfortressviewport.h"
#include "sharedutil.h"
#include "vgui_moveitempanel.h"
#include "vgui_containerlist.h"

VGUI_MoveItemPanel::VGUI_MoveItemPanel(CContainerPanel *pParent) : CTransparentPanel(80, 0, 0, 0, 0)
{
	vgui::Font *pFont = g_FontSml;
	int vButtonHeight = g_FontSml->getTall();
	int y = 0;
	int w = 0;

	for(int i = 0; i < gNumMoveNumbers + 1; i++)
	{
		int vNumMove;
		msstring vsText;
		if (i == gNumMoveNumbers)
		{
			vNumMove = 0;
			vsText = "Cancel";
		}
		else
		{
			vNumMove = gMoveNumbers[i];
			vsText = msstring("Move ") + vNumMove + " To Hands";
		}

		int vButtonW;
		int vButtonH;
		pFont->getTextSize(vsText, vButtonW, vButtonH);
		MSButton *pButton = new MSButton(this, "", XRES(5), y + YRES(5), vButtonW, vButtonH, COLOR(255, 0, 0, 0), COLOR(100, 100, 100, 0));
		pButton->setText(vsText);
		pButton->setFont(pFont);
		pButton->setContentAlignment(vgui::Label::Alignment::a_west);
		pButton->addActionSignal(new CMoveItemSignal(this, vNumMove));
		mButtons.add(pButton);
		pButton->setVisible(true);

		y += vButtonH;
		w = V_max(w, vButtonW);
	}

	setSize(w + XRES(10), y + YRES(10));
	setParent(mpParent = pParent);
	setVisible(false);
	setBorder(new LineBorder(2, Color(0,128,0,128)));
}

void VGUI_MoveItemPanel::ShowForButton(VGUI_ItemButton *pButton)
{
	mpCurrentItemButton = pButton;
	int x, y, w, h;

	// Move to be over the clicked button
	pButton->getAbsExtents(x, y, w, h);
	setPos(x,y);

	for(int i = 0; i < gNumMoveNumbers; i++)
	{
		mButtons[i]->setEnabled(pButton->m_Data.Quantity >= gMoveNumbers[i]);
	}

	setVisible(true);
}

void VGUI_MoveItemPanel::Move(int vNumMove)
{
	if (vNumMove) 
		mpParent->MoveItem(mpCurrentItemButton, vNumMove);

	setVisible(false);
}

void VGUI_MoveItemPanel::setVisible(bool bVisible)
{
	CTransparentPanel::setVisible(bVisible);
	for(int i = 0; i < gNumMoveNumbers + 1; i++)
	{
		mButtons[i]->setArmed(false);
	}
}

CMoveItemSignal::CMoveItemSignal(VGUI_MoveItemPanel *pMoveItemPanel, int vNumMove)
{
	mpMoveItemPanel = pMoveItemPanel;
	mNumMove = vNumMove;
}

void CMoveItemSignal::actionPerformed(Panel *pPanel)
{
	mpMoveItemPanel->Move(mNumMove);
}