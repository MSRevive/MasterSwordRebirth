#include <tier0/dbg.h>

#include <vgui/IPanel.h>
#include <vgui_controls/Controls.h>

#include "CClientViewport.h"
#include "chat/CChatPanel.h"

CClientViewport *g_pViewport = nullptr;

CClientViewport::CClientViewport(vgui2::VPANEL parent) : BaseClass(nullptr, "MSClientViewport")
{
	g_pViewport = this;

	SetParent(parent);
	SetSize(10, 10);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	SetPaintBackgroundEnabled(false);

	CreatePanels();
}

// New VGUI2 UI elements get created here.
void CClientViewport::CreatePanels()
{
	AddPanel(m_pChat = new CChat(this));
}

void CClientViewport::AddPanel(IViewportPanel *pPanel)
{
	m_Panels.push_back(pPanel);
}

void CClientViewport::Reset()
{
	for (IViewportPanel *pPanel : m_Panels)
		pPanel->Reset();
}

void CClientViewport::HideClientUI()
{
	for (IViewportPanel *pPanel : m_Panels)
		pPanel->OnHideClientUI();
}

void CClientViewport::OnThink()
{
	if (GetVParent())
	{
		int wide, tall, parentWide, parentTall;
		GetSize(wide, tall);
		vgui2::ipanel()->GetSize(GetVParent(), parentWide, parentTall);

		if (wide != parentWide || tall != parentTall)
			SetBounds(0, 0, parentWide, parentTall);
	}

	for (IViewportPanel *pPanel : m_Panels)
		pPanel->Think();

	BaseClass::OnThink();
}
