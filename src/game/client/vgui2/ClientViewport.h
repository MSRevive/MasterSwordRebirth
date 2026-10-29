#pragma once

#include <vector>

#include <vgui_controls/Panel.h>

#include "IViewportPanel.h"

class CChat;

class CClientViewport : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE(CClientViewport, vgui2::Panel);

public:
	CClientViewport(vgui2::VPANEL parent);

	void AddPanel(IViewportPanel *pPanel);
	void Reset();
	void HideClientUI();

	CChat *GetChat() { return m_pChat; }

protected:
	void OnThink() override;

private:
	void CreatePanels();

	std::vector<IViewportPanel *> m_Panels;
	CChat *m_pChat = nullptr;
};

extern CClientViewport *g_pViewport;
