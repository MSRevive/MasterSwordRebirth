#include "hud.h"
#include "cl_util.h"
#include "const.h"

#include "vgui_int.h"
#include "vgui_teamfortressviewport.h"

//Master Sword
#include "../MSShared/sharedutil.h"
#include "../MSShared/vgui_menudefsshared.h"
#include "menu.h"
#include "logfile.h"

extern int g_iVisibleMouse;

//
//	VGUI Global Class
//

bool VGUI::ShowMenu(msstring_ref Name)
{
	if (!gViewPort)
		return false;

	VGUI_MainPanel *pPanel = FindPanel(Name);

	if (pPanel)
	{
		bool CanOpen = pPanel->CanOpen();
		if (CanOpen)
		{
			gViewPort->HideVGUIMenu();
			pPanel->Open();
			if (pPanel->isVisible())
				gViewPort->m_pCurrentMenu = dynamic_cast<CMenuPanel *>(pPanel);
		}

		return CanOpen;
	}

	return false;
}
bool VGUI::ToggleMenuVisible(msstring_ref Name)
{
	if (!gViewPort)
		return false;

	VGUI_MainPanel *pPanel = FindPanel(Name);

	if (pPanel)
	{
		if (!pPanel->isVisible())
			ShowMenu(Name);
		else
			HideMenu(pPanel);

		return true;
	}

	return false;
}
bool VGUI::HideMenu(VGUI_MainPanel *pPanel)
{
	if (!gViewPort || !pPanel)
		return false;

	pPanel->Close();

	if (gViewPort->m_pCurrentMenu == pPanel)
		gViewPort->SetCurrentMenu(NULL);

	gViewPort->UpdateCursorState();

	return false;
}

VGUI_MainPanel *VGUI::FindPanel(msstring_ref Name)
{
	for (int i = 0; i < gViewPort->m_Menus.size(); i++)
	{
		VGUI_MainPanel *pPanel = gViewPort->m_Menus[i];
		if (pPanel->m_Name == Name)
			return pPanel;
	}
	return NULL;
}

//
//	VGUI_MainPanel
//

bool VGUI_MainPanel::CanOpen()
{
	bool CanOpen = !gViewPort->m_pCurrentMenu; //No current menu up

	return CanOpen;
}

bool VGUI_MainPanel::UpdateCursorState()
{
	if (m_NoMouse)
	{
		//if( !gEngfuncs.pDemoAPI->IsPlayingback() ) { IN_ResetMouse(); }

		g_iVisibleMouse = false;
		App::getInstance()->setCursorOveride(App::getInstance()->getScheme()->getCursor(Scheme::SchemeCursor::scu_none));
		return false;
	}

	g_iVisibleMouse = true;
	App::getInstance()->setCursorOveride(App::getInstance()->getScheme()->getCursor(Scheme::SchemeCursor::scu_arrow));

	return true;
}
