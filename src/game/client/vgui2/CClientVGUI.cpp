#include <tier0/dbg.h>

#include <tier1/interface.h>
#include <tier1/tier1.h>

#include <tier2/tier2.h>

#include <FileSystem.h>
#include <IEngineVGui.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Controls.h>

#include "CClientVGUI.h"
#include "CClientViewport.h"
#include "ms_vgui2.h"

#define CLIENT_SCHEME_FILE "resource/ClientScheme.res"

static vgui2::HScheme s_hClientScheme = 0;

namespace vgui2
{
	vgui2::HScheme VGui_GetDefaultScheme()
	{
		return s_hClientScheme;
	}
}

// Engine interface the SDK doesn't ship. The engine surface is
// BaseUISurface : vgui2::ISurface, IMouseControl, and ISurface has no data,
// so this part of the object sits one pointer after the ISurface.
class IMouseControl
{
public:
	virtual bool VGUI2MouseControl() = 0;
	virtual void SetVGUI2MouseControl(bool state) = 0;
};

// While VGUI2 has mouse control, VGUI1 can't show the cursor at all.
// A VGUI2 panel that needs the mouse should turn it on only while it is open.
static void SetVGUI2MouseControl(bool bState)
{
	if (!g_pVGuiSurface)
		return;

	IMouseControl *pMouseControl = reinterpret_cast<IMouseControl *>(reinterpret_cast<char *>(g_pVGuiSurface) + sizeof(void *));
	if (pMouseControl->VGUI2MouseControl() != bState)
		pMouseControl->SetVGUI2MouseControl(bState);
}

// Registered by the client's own interface.cpp (see cdll_int.cpp), not tier1's.
// Never freed: the engine keeps the pointer after the client has shut down.
void *MSVGUI2::GetClientVGUIInterface()
{
	static CClientVGUI *s_pClientVGUI = new CClientVGUI;
	return static_cast<IClientVGUI *>(s_pClientVGUI);
}

void CClientVGUI::Initialize(CreateInterfaceFn* factories, int count)
{
	ConnectTier1Libraries(factories, count);
	ConnectTier2Libraries(factories, count);

	if (!vgui2::VGui_InitInterfacesList("CLIENT", factories, count))
	{
		Error("Failed to intialize VGUI2!\n");
		return;
	}

	// Without the file, panels use the engine's default scheme
	if (g_pFullFileSystem && g_pFullFileSystem->FileExists(CLIENT_SCHEME_FILE))
		s_hClientScheme = g_pVGuiSchemeManager->LoadSchemeFromFile(CLIENT_SCHEME_FILE, "ClientScheme");

	m_bInitialized = true;
}

void CClientVGUI::Start()
{
	if (!m_bInitialized)
		return;

	// The engine turned this on right after Initialize
	SetVGUI2MouseControl(false);

	if (g_pViewport)
		return;

	vgui2::VPANEL parent = g_pEngineVGui ? g_pEngineVGui->GetPanel(PANEL_CLIENTDLL) : m_Parent;
	new CClientViewport(parent);
}

void CClientVGUI::SetParent(vgui2::VPANEL parent)
{
	m_Parent = parent;

	if (g_pViewport && !g_pViewport->GetVParent())
		g_pViewport->SetParent(parent);
}

int CClientVGUI::UseVGUI1()
{
	return true;
}

void CClientVGUI::HideScoreBoard()
{
}

void CClientVGUI::HideAllVGUIMenu()
{
}

void CClientVGUI::ActivateClientUI()
{
}

void CClientVGUI::HideClientUI()
{
	if (g_pViewport)
		g_pViewport->HideClientUI();
}

void CClientVGUI::Shutdown()
{
}
