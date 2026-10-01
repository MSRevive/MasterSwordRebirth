#pragma once

#include <vgui/VGUI2.h>

#include <IClientVGUI.h>

class CClientVGUI : public IClientVGUI
{
public:
	void Initialize(CreateInterfaceFn* factories, int count);
	void Start() override;
	void SetParent(vgui2::VPANEL parent);
	int UseVGUI1() override;
	void HideScoreBoard();
	void HideAllVGUIMenu();
	void ActivateClientUI();
	void HideClientUI();
	void Shutdown();
};