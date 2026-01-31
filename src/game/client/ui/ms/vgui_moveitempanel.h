// vgui_MoveItemPanel by MiB, for the "Move X to hand" pop-up
#ifndef MOVEITEMPANEL_H
#define MOVEITEMPANEL_H

#include "vgui_mscontrols.h"

static const int gNumMoveNumbers = 5;
static const int gMoveNumbers[ gNumMoveNumbers ] = { 5, 10, 25, 50, 100 };

class VGUI_MoveItemPanel : public CTransparentPanel
{
private:
	mslist<MSButton*> mButtons;
	CContainerPanel *mpParent;
	VGUI_ItemButton *mpCurrentItemButton;

public:
	VGUI_MoveItemPanel(CContainerPanel *pParent);
	void ShowForButton(VGUI_ItemButton *pButton);
	void Move(int vNumMove);
	virtual void setVisible(bool bVisible);
};

class CMoveItemSignal : public ActionSignal
{
private:
	VGUI_MoveItemPanel *mpMoveItemPanel;
	int mNumMove;
	
public:
	CMoveItemSignal(VGUI_MoveItemPanel *pMoveItemPanel, int vNumMove);
	void actionPerformed(Panel *pPanel);
};

#endif // MOVEITEMPANEL_H