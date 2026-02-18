// vgui_MoveItemPanel by MiB, for the "Move X to hand" pop-up
#pragma once

#include "vgui_mscontrols.h"
//#include "vgui_container.h"
#include "vgui_teamfortressviewport.h"

class VGUI_ContainerPanel;

class VGUI_OrderItemPanel : public CTransparentPanel
{
private:
	mslist<MSButton*> mButtons;
	VGUI_ContainerPanel *mpParent;
	VGUI_ItemButton *mpCurrentItemButton;

public:
	VGUI_OrderItemPanel(VGUI_ContainerPanel *pParent);
	void ShowForButton(VGUI_ItemButton *pButton);
	void Move(int vNumMove);
	virtual void setVisible(bool bVisible);
};

class COrderItemSignal : public ActionSignal
{
private:
	VGUI_OrderItemPanel *mpOrderItemPanel;
	int m_iOrder;
	
public:
	COrderItemSignal(VGUI_OrderItemPanel *pOrderItemPanel, int m_iOrder);
	void actionPerformed(Panel *pPanel);
};