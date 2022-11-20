//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Client DLL VGUI Viewport
//
// $Workfile:     $
// $Date: 2005/01/17 13:16:49 $
//
//-----------------------------------------------------------------------------
// $Log: vgui_TeamFortressViewport.cpp,v $
// Revision 1.8  2005/01/17 13:16:49  dogg
// Brand new inventory VGUI, revised item system, Magic, Mp3 support and item storage
//
// Revision 1.7  2004/11/07 01:06:28  dogg
// New Event Console
//
// Revision 1.6  2004/10/19 23:14:54  dogg
// BIG update
// Memory leak fixes
//
// Revision 1.5  2004/10/16 11:47:01  dogg
// no message
//
// Revision 1.4  2004/10/14 12:15:31  dogg
// no message
//
// Revision 1.3  2004/09/12 00:11:59  dogg
// no message
//
// Revision 1.2  2004/09/08 03:20:54  dogg
// no message
//
//
// $NoKeywords: $
//=============================================================================
/*#include<vgui_cursor.h>
#include<vgui_frame.h>
#include<vgui_label.h>
#include<vgui_surface.h>
#include<vgui_borderlayout.h>
#include<vgui_panel.h>
#include<vgui_imagepanel.h>
#include<vgui_button.h>
#include<vgui_actionsignal.h>
#include<vgui_inputsignal.h>
#include<vgui_menuseparator.h>
#include<vgui_textpanel.h>
#include<vgui_loweredborder.h>
#include<vgui_lineborder.h>
#include<vgui_scheme.h>
#include<vgui_font.h>
#include<vgui_app.h>
#include<VGUI_BuildGroup.h>*/

#include "hud.h"
#include "cl_util.h"
//#include "camera.h"
//#include "kbutton.h"
//#include "cvardef.h"
//#include "usercmd.h"
#include "const.h"
//#include "in_defs.h"
#include "parsemsg.h"
#include "../engine/keydefs.h"
//#include "demo.h"
#include "demo_api.h"

#include "vgui_int.h"
#include "vgui_teamfortressviewport.h"
#include "vgui_serverbrowser.h"
#include "vgui_scorepanel.h"
#include "vgui_spectatorpanel.h"
#include "pm_shared.h"

//Master Sword
#include "sharedutil.h"
#include "vgui_menudefsshared.h"
#include "menu.h"
#include "ms/vgui_mscontrols.h"
#include "ms/vgui_storebuy.h"
#include "ms/vgui_storesell.h"
#include "ms/vgui_storage.h"
#include "ms/vgui_containerlist.h"
//#include "ms/vgui_ChooseCharacter.h"
#include "ms/vgui_stats.h"
#include "ms/vgui_spawn.h"
#include "ms/vgui_hud.h"
#include "ms/vgui_menubase.h"
#include "logger.h"

#include "ms/vgui_localizedpanel.h" // MiB MAR2015_01 [LOCAL_PANEL] - Include for local panel
//-----------

void VGUI_Think()
{
	if (!gViewPort)
		return;

	/*
	 for (int i = 0; i < gViewPort->m_Menus.size(); i++) 
		gViewPort->m_Menus[i]->Think( );
	*/

	startdbg;
	for (int i = 0; i < gViewPort->m_Menus.size(); i++)
	{
		dbg(i);
		gViewPort->m_Menus[i]->Think();
	}
	enddbg;
}
class CMenuPanel *CreateNewCharacterPanel();

extern int g_iVisibleMouse;
class CCommandMenu;
int g_iPlayerClass;
int g_iTeamNumber;
int g_iUser1;
int g_iUser2;
int g_iUser3;

void IN_ResetMouse(void);
extern CMenuPanel *CMessageWindowPanel_Create(const char *szMOTD, const char *szTitle, int iShadeFullscreen, int iRemoveMe, int x, int y, int wide, int tall);

using namespace vgui;

// Team Colors
/*int iTeamColors[5][3] =
{
	{ 255, 255, 255 },
	{ 66, 115, 247 },
	{ 220, 51, 38 },
	{ 240, 135, 0 },
	{ 115, 240, 115 },
};*/

// Used for Class specific buttons
/*char *sTFClasses[] =
{
	"",
	"SCOUT",
	"SNIPER",
	"SOLDIER",
	"DEMOMAN",
	"MEDIC",
	"HWGUY",
	"PYRO",
	"SPY",
	"ENGINEER",
	"CIVILIAN",
};*/

char *sLocalisedClasses[] =
	{
		"Buy",
		"Sell",
};

char *sTFClassSelection[] =
	{
		"Buy",
		"Sell",
};

// This maps class numbers to the Invalid Class bit.
// This is needed for backwards compatability in maps that were finished before
// all the classes were in TF. Hence the wacky sequence.
int sTFValidClassInts[] =
	{
		0,
		TF_ILL_SCOUT,
		TF_ILL_SNIPER,
		TF_ILL_SOLDIER,
		TF_ILL_DEMOMAN,
		TF_ILL_MEDIC,
		TF_ILL_HVYWEP,
		TF_ILL_PYRO,
		TF_ILL_SPY,
		TF_ILL_ENGINEER,
		TF_ILL_RANDOMPC,
};

// Get the name of TGA file, based on GameDir
char *GetVGUITGAName(const char *pszName)
{
	char sz[256];
	static char gd[256];
	const char *gamedir;

	//Always use 640 res
	 _snprintf(sz, sizeof(sz),  pszName,  640 );

	gamedir = gEngfuncs.pfnGetGameDirectory();
	 _snprintf(gd, sizeof(gd),  "%s/gfx/vgui/%s.tga",  gamedir,  sz );

	return gd;
}

//================================================================
// COMMAND MENU
//================================================================
void CCommandMenu::AddButton(CommandButton *pButton)
{
	if (m_iButtons >= MAX_BUTTONS)
		return;

	m_aButtons[m_iButtons] = pButton;
	m_iButtons++;
	pButton->setParent(this);
	pButton->setFont(Scheme::sf_primary3);

	// give the button a default key binding
	if (m_iButtons < 10)
	{
		pButton->setBoundKey(m_iButtons + '0');
	}
	else if (m_iButtons == 10)
	{
		pButton->setBoundKey('0');
	}
}

//-----------------------------------------------------------------------------
// Purpose: Tries to find a button that has a key bound to the input, and
//			presses the button if found
// Input  : keyNum - the character number of the input key
// Output : Returns true if the command menu should close, false otherwise
//-----------------------------------------------------------------------------
bool CCommandMenu::KeyInput(int keyNum)
{
	// loop through all our buttons looking for one bound to keyNum
	for (int i = 0; i < m_iButtons; i++)
	{
		if (!m_aButtons[i]->IsNotValid())
		{
			if (m_aButtons[i]->getBoundKey() == keyNum)
			{
				// hit the button
				if (m_aButtons[i]->GetSubMenu())
				{
					// open the sub menu
					gViewPort->SetCurrentCommandMenu(m_aButtons[i]->GetSubMenu());
					return false;
				}
				else
				{
					// run the bound command
					m_aButtons[i]->fireActionSignal();
					return true;
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: clears the current menus buttons of any armed (highlighted)
//			state, and all their sub buttons
//-----------------------------------------------------------------------------
void CCommandMenu::ClearButtonsOfArmedState(void)
{
	for (int i = 0; i < GetNumButtons(); i++)
	{
		m_aButtons[i]->setArmed(false);

		if (m_aButtons[i]->GetSubMenu())
		{
			m_aButtons[i]->GetSubMenu()->ClearButtonsOfArmedState();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pSubMenu -
// Output : CommandButton
//-----------------------------------------------------------------------------
CommandButton *CCommandMenu::FindButtonWithSubmenu(CCommandMenu *pSubMenu)
{
	for (int i = 0; i < GetNumButtons(); i++)
	{
		if (m_aButtons[i]->GetSubMenu() == pSubMenu)
			return m_aButtons[i];
	}

	return NULL;
}

// Recalculate the visible buttons
bool CCommandMenu::RecalculateVisibles(int iNewYPos, bool bHideAll)
{
	int iCurrentY = 0;
	int iXPos, iYPos;
	bool bHasButton = false;

	if (iNewYPos)
		setPos(_pos[0], iNewYPos);

	// Cycle through all the buttons in this menu, and see which will be visible
	for (int i = 0; i < m_iButtons; i++)
	{
		int iClass = m_aButtons[i]->GetPlayerClass();
		if ((iClass && iClass != g_iPlayerClass) || (m_aButtons[i]->IsNotValid()) || bHideAll)
		{
			m_aButtons[i]->setVisible(false);
			if (m_aButtons[i]->GetSubMenu() != NULL)
			{
				(m_aButtons[i]->GetSubMenu())->RecalculateVisibles(_pos[1] + iCurrentY, true);
			}
		}
		else
		{
			// If it's got a submenu, force it to check visibilities
			if (m_aButtons[i]->GetSubMenu() != NULL)
			{
				if (!(m_aButtons[i]->GetSubMenu())->RecalculateVisibles(_pos[1] + iCurrentY, false))
				{
					// The submenu had no visible buttons, so don't display this button
					m_aButtons[i]->setVisible(false);
					continue;
				}
			}

			m_aButtons[i]->setVisible(true);

			// Make sure it's at the right Y position
			m_aButtons[i]->getPos(iXPos, iYPos);
			m_aButtons[i]->setPos(iXPos, iCurrentY);

			iCurrentY += BUTTON_SIZE_Y - 1;
			bHasButton = true;
		}
	}

	// Set Size
	setSize(_size[0], iCurrentY + 1);

	return bHasButton;
}

// Make sure all submenus can fit on the screen
void CCommandMenu::RecalculatePositions(int iYOffset)
{
	int iNewYPos = _pos[1] + iYOffset;
	int iAdjust = 0;

	// Calculate if this is going to fit onscreen, and shuffle it up if it won't
	int iBottom = iNewYPos + _size[1];
	if (iBottom > ScreenHeight)
	{
		// Move in increments of button sizes
		while (iAdjust < (iBottom - ScreenHeight))
		{
			iAdjust += BUTTON_SIZE_Y - 1;
		}
		iNewYPos -= iAdjust;

		// Make sure it doesn't move off the top of the screen (the menu's too big to fit it all)
		if (iNewYPos < 0)
		{
			iAdjust -= (0 - iNewYPos);
			iNewYPos = 0;
		}
	}

	// We need to force all menus below this one to update their positions now, because they
	// might have submenus riding off buttons in this menu that have just shifted.
	for (int i = 0; i < m_iButtons; i++)
		m_aButtons[i]->UpdateSubMenus(iAdjust);

	setPos(_pos[0], iNewYPos);
}

// Make this menu and all menus above it in the chain visible
void CCommandMenu::MakeVisible(CCommandMenu *pChildMenu)
{
	/*
	// Push down the button leading to the child menu
	for (int i = 0; i < m_iButtons; i++)
	{
		if ( (pChildMenu != NULL) && (m_aButtons[i]->GetSubMenu() == pChildMenu) )
		{
			m_aButtons[i]->setArmed( true );
		}
		else
		{
			m_aButtons[i]->setArmed( false );
		}
	}
*/

	setVisible(true);
	if (m_pParentMenu)
		m_pParentMenu->MakeVisible(this);
}

//================================================================
// CreateSubMenu
CCommandMenu *TeamFortressViewport::CreateSubMenu(CommandButton *pButton, CCommandMenu *pParentMenu)
{
	int iXPos = 0;
	int iYPos = 0;
	int iWide = CMENU_SIZE_X;
	int iTall = 0;

	if (pParentMenu)
	{
		iXPos = pParentMenu->GetXOffset() + CMENU_SIZE_X - 1;
		iYPos = pParentMenu->GetYOffset() + BUTTON_SIZE_Y * (m_pCurrentCommandMenu->GetNumButtons() - 1);
	}

	CCommandMenu *pMenu = new CCommandMenu(pParentMenu, iXPos, iYPos, iWide, iTall);
	pMenu->setParent(this);
	pButton->AddSubMenu(pMenu);
	pButton->setFont(Scheme::sf_primary3);

	// Create the Submenu-open signal
	InputSignal *pISignal = new CMenuHandler_PopupSubMenuInput(pButton, pMenu);
	pButton->addInputSignal(pISignal);

	// Put a > to show it's a submenu
	CImageLabel *pLabel = new CImageLabel("arrow", CMENU_SIZE_X - SUBMENU_SIZE_X, SUBMENU_SIZE_Y);
	pLabel->setParent(pButton);
	pLabel->addInputSignal(pISignal);

	// Reposition
	pLabel->getPos(iXPos, iYPos);
	pLabel->setPos(CMENU_SIZE_X - pLabel->getImageWide(), (BUTTON_SIZE_Y - pLabel->getImageTall()) / 2);

	// Create the mouse off signal for the Label too
	if (!pButton->m_bNoHighlight)
		pLabel->addInputSignal(new CHandler_CommandButtonHighlight(pButton));

	return pMenu;
}

//-----------------------------------------------------------------------------
// Purpose: Makes sure the memory allocated for TeamFortressViewport is nulled out
// Input  : stAllocateBlock -
// Output : void *
//-----------------------------------------------------------------------------
void *TeamFortressViewport::operator new(size_t stAllocateBlock)
{
	//	void *mem = Panel::operator new( stAllocateBlock );
	void *mem = ::operator new(stAllocateBlock);
	memset(mem, 0, stAllocateBlock);
	return mem;
}

//-----------------------------------------------------------------------------
// Purpose: InputSignal handler for the main viewport
//-----------------------------------------------------------------------------
class CViewPortInputHandler : public InputSignal
{
public:
	CViewPortInputHandler()
	{
	}

	virtual void cursorMoved(int x, int y, Panel *panel) {}
	virtual void cursorEntered(Panel *panel) {}
	virtual void cursorExited(Panel *panel) {}
	virtual void mousePressed(MouseCode code, Panel *panel)
	{
		if (code != MOUSE_LEFT)
		{
			// send a message to close the command menu
			// this needs to be a message, since a direct call screws the timing
			gEngfuncs.pfnClientCmd("ForceCloseCommandMenu\n");
		}
	}
	virtual void mouseReleased(MouseCode code, Panel *panel)
	{
	}

	virtual void mouseDoublePressed(MouseCode code, Panel *panel) {}
	virtual void mouseWheeled(int delta, Panel *panel) {}
	virtual void keyPressed(KeyCode code, Panel *panel) {}
	virtual void keyTyped(KeyCode code, Panel *panel) {}
	virtual void keyReleased(KeyCode code, Panel *panel) {}
	virtual void keyFocusTicked(Panel *panel) {}
};

//================================================================
Font *g_FontSml, *g_FontTitle, *g_FontID;
TeamFortressViewport::TeamFortressViewport(int x, int y, int wide, int tall) : Panel(x, y, wide, tall), m_SchemeManager(wide, tall)
{
	startdbg;
	gViewPort = this;
	m_iInitialized = false;
	//Master Sword
	m_pStoreMenu = NULL;
	m_pStoreBuyMenu = NULL;
	m_pStoreSellMenu = NULL;
	m_pStoreStorageMenu = NULL;
	m_pContainerMenu = NULL;
	m_pStatMenu = NULL;
	m_pSpawnScreen = NULL;
	m_pLocalizedMenu = NULL; // MiB MAR2015_01 [LOCAL_PANEL] - NULL out the pointer
	m_pHUDPanel = NULL;
	m_pMainMenu = NULL;
	//------------
	m_pScoreBoard = NULL;
	m_pSpectatorPanel = NULL; //SDK 2.3
	m_pSpectatorMenu = NULL;
	m_pCurrentMenu = NULL;
	m_pCurrentCommandMenu = NULL;

	dbg("Call Initialize() (first time)");
	logfile << Logger::LOG_INFO << "[TeamFortressViewport: Initialize]\n";
	Initialize();

	addInputSignal(new CViewPortInputHandler);

	int r, g, b, a;

	dbg("Setup Schemes");
	Scheme *pScheme = App::getInstance()->getScheme();

	// primary text color
	// Get the colors
	//!! two different types of scheme here, need to integrate
	SchemeHandle_t hPrimaryScheme = m_SchemeManager.getSchemeHandle("Primary Button Text");
	{
		// font
		pScheme->setFont(Scheme::sf_primary1, m_SchemeManager.getFont(hPrimaryScheme));

		// text color
		m_SchemeManager.getFgColor(hPrimaryScheme, r, g, b, a);
		pScheme->setColor(Scheme::sc_primary1, r, g, b, a); // sc_primary1 is non-transparent orange

		// background color (transparent black)
		m_SchemeManager.getBgColor(hPrimaryScheme, r, g, b, a);
		pScheme->setColor(Scheme::sc_primary3, r, g, b, a);

		// armed foreground color
		m_SchemeManager.getFgArmedColor(hPrimaryScheme, r, g, b, a);
		pScheme->setColor(Scheme::sc_secondary2, r, g, b, a);

		// armed background color
		m_SchemeManager.getBgArmedColor(hPrimaryScheme, r, g, b, a);
		pScheme->setColor(Scheme::sc_primary2, r, g, b, a);

		//!! need to get this color from scheme file
		// used for orange borders around buttons
		m_SchemeManager.getBorderColor(hPrimaryScheme, r, g, b, a);
		// pScheme->setColor(Scheme::sc_secondary1, r, g, b, a );
		pScheme->setColor(Scheme::sc_secondary1, 255 * 0.7, 170 * 0.7, 0, 0);
	}

	// Change the second primary font (used in the scoreboard)
	SchemeHandle_t hScoreboardScheme = m_SchemeManager.getSchemeHandle("Scoreboard Text");
	{
		pScheme->setFont(Scheme::sf_primary2, m_SchemeManager.getFont(hScoreboardScheme));
	}

	// Change the third primary font (used in command menu)
	SchemeHandle_t hCommandMenuScheme = m_SchemeManager.getSchemeHandle("CommandMenu Text");
	{
		pScheme->setFont(Scheme::sf_primary3, m_SchemeManager.getFont(hCommandMenuScheme));
	}

	//CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	dbg("Setup MS Fonts");
	g_FontSml = m_SchemeManager.getFont(m_SchemeManager.getSchemeHandle("Briefing Text"));
	g_FontTitle = m_SchemeManager.getFont(m_SchemeManager.getSchemeHandle("Title Font"));
	g_FontID = m_SchemeManager.getFont(m_SchemeManager.getSchemeHandle("ID Text"));

	App::getInstance()->setScheme(pScheme);

	// VGUI MENUS
	dbg("Call CreateVGUIMenus");
	CreateVGUIMenus();

	dbg("Call CreateScoreBoard");
	CreateScoreBoard();
	//CreateCommandMenu();

	dbg("Call CreateServerBrowser");
	CreateServerBrowser();
	//logfile << "[CreateServerBrowser: Complete]" << endl;

	// Create the spectator Panel
	dbg("Call SpectatorPanel()");
	m_pSpectatorPanel = new SpectatorPanel(0, 0, ScreenWidth, ScreenHeight);
	m_pSpectatorPanel->setParent(this);
	m_pSpectatorPanel->setVisible(false);

	dbg("Call m_pSpectatorPanel->Initialize()");
	//logfile << "[m_pSpectatorPanel->Initialize]" << endl;
	m_pSpectatorPanel->Initialize();

	logfile << Logger::LOG_INFO << "[TeamFortressViewport: Complete]\n";

	//CreateSpectatorMenu();

	m_iInitialized = true;
	enddbg;
}

//-----------------------------------------------------------------------------
// Purpose: Called everytime a new level is started. Viewport clears out it's data.
//-----------------------------------------------------------------------------
void TeamFortressViewport::Initialize(void)
{
	startdbg;
	// Force each menu to Initialize
	//Master Sword
	//Turn off HUDs
	gHUD.m_iHideHUDDisplay |= HIDEHUD_ALL;

	dbg("Initialize MS VGUI Menus");
	if (m_pStoreMenu)
		m_pStoreMenu->Initialize();
	if (m_pStoreBuyMenu)
		m_pStoreBuyMenu->Initialize();
	if (m_pStoreSellMenu)
		m_pStoreSellMenu->Initialize();
	if (m_pStoreStorageMenu)
		m_pStoreStorageMenu->Initialize();
	if (m_pContainerMenu)
		m_pContainerMenu->Initialize();
	if (m_pStatMenu)
		m_pStatMenu->Initialize();
	if (m_pSpawnScreen)
		m_pSpawnScreen->Initialize();
	if (m_pLocalizedMenu)
		m_pLocalizedMenu->Initialize(); // MiB MAR2015_01 [LOCAL_PANEL] - Initialize local panel
	if (m_pHUDPanel)
		m_pHUDPanel->Initialize();
	if (m_pMainMenu)
		m_pMainMenu->Initialize();

	//------------
	dbg("Call m_pScoreBoard->Initialize");
	if (m_pScoreBoard)
	{
		m_pScoreBoard->Initialize();
		HideScoreBoard();
	}

	dbg("Call m_pSpectatorPanel->setVisible");
	if (m_pSpectatorPanel)
	{
		// Spectator menu doesn't need initializing
		m_pSpectatorPanel->setVisible(false);
	}

	// Make sure all menus are hidden
	dbg("Call HideVGUIMenu");
	HideVGUIMenu();

	dbg("Call HideCommandMenu");
	HideCommandMenu();

	// Clear out some data
	m_iGotAllMOTD = true;
	m_iRandomPC = false;
	m_flScoreBoardLastUpdated = 0;
	m_flSpectatorPanelLastUpdated = 0;

	// reset player info
	g_iPlayerClass = 0;
	g_iTeamNumber = 0;

	dbg("Init Teamnames");
	 strncpy(m_sMapName,  "", sizeof(m_sMapName) );
	 strncpy(m_szServerName,  "", sizeof(m_szServerName) );
	for (int i = 0; i < 5; i++)
	{
		m_iValidClasses[i] = 0;
		strncpy(m_sTeamNames[i], "", MAX_TEAMNAME_SIZE);
	}

	dbg("Call App::getInstance()->setCursorOveride");
	App::getInstance()->setCursorOveride(App::getInstance()->getScheme()->getCursor(Scheme::SchemeCursor::scu_none));
	enddbg;
}

//-----------------------------------------------------------------------------
// Purpose: Read the Command Menu structure from the txt file and create the menu.
//-----------------------------------------------------------------------------
void TeamFortressViewport::CreateCommandMenu(void)
{
	/*
	// COMMAND MENU
	// Create the root of the Command Menu
	m_pCommandMenus[0] = new CCommandMenu(NULL, 0, CMENU_TOP, CMENU_SIZE_X, 300);	// This will be resized once we know how many items are in it
	m_pCommandMenus[0]->setParent(this);
	m_pCommandMenus[0]->setVisible(false);
	m_iNumMenus = 1;
	m_iCurrentTeamNumber = m_iUser1 = m_iUser2 = 0;

	// Read Command Menu from the txt file
	char token[1024];
	char *pfile = (char*)gEngfuncs.COM_LoadFile("commandmenu.txt", 5, NULL);
	if (!pfile)
	{
		gEngfuncs.Con_DPrintf( "Unable to open commandmenu.txt\n");
		SetCurrentCommandMenu( NULL );
		return;
	}

try
{
	// First, read in the localisation strings

	// Detpack strings
	gHUD.m_TextMessage.LocaliseTextString( "#DetpackSet_For5Seconds",   m_sDetpackStrings[0], MAX_BUTTON_SIZE );
	gHUD.m_TextMessage.LocaliseTextString( "#DetpackSet_For20Seconds",   m_sDetpackStrings[1], MAX_BUTTON_SIZE );
	gHUD.m_TextMessage.LocaliseTextString( "#DetpackSet_For50Seconds",   m_sDetpackStrings[2], MAX_BUTTON_SIZE );

	// Now start parsing the menu structure
	m_pCurrentCommandMenu = m_pCommandMenus[0];
	char szLastButtonText[32] = "file start";
	pfile = gEngfuncs.COM_ParseFile(pfile, token);
	while ( ( strlen ( token ) > 0 ) && ( m_iNumMenus < MAX_MENUS ) )
	{
		// Keep looping until we hit the end of this menu
		while ( token[0] != '}' && ( strlen( token ) > 0 ) )
		{
			char cText[32] = "";
			char cBoundKey[32] = "";
			char cCustom[32] = "";
			static const int cCommandLength = 128;
			char cCommand[cCommandLength] = "";
			char szMap[MAX_MAPNAME] = "";
			int	 iPlayerClass = 0;
			int  iCustom = false;
			int  iTeamOnly = 0;
			bool bGetExtraToken = true;
			CommandButton *pButton = NULL;
			
			// We should never be here without a Command Menu
			if (!m_pCurrentCommandMenu)
			{
				gEngfuncs.Con_Printf("Error in Commandmenu.txt file after '%s'.\n", szLastButtonText );
				m_iInitialized = false;
				return;
			}

			// token should already be the bound key, or the custom name
			strncpy( cCustom, token, 32 );
			cCustom[31] = '\0';

			// See if it's a custom button
			if (!strcmp(cCustom, "CUSTOM") )
			{
				iCustom = true;

				// Get the next token
				pfile = gEngfuncs.COM_ParseFile(pfile, token);
			}
			// See if it's a map
			else if (!strcmp(cCustom, "MAP") )
			{
				// Get the mapname
				pfile = gEngfuncs.COM_ParseFile(pfile, token);
				strncpy( szMap, token, MAX_MAPNAME );
				szMap[MAX_MAPNAME-1] = '\0';

				// Get the next token
				pfile = gEngfuncs.COM_ParseFile(pfile, token);
			}
			else if ( !strncmp(cCustom, "TEAM", 4) ) // TEAM1, TEAM2, TEAM3, TEAM4
			{
				// make it a team only button
				iTeamOnly = atoi( cCustom + 4 );
				
				// Get the next token
				pfile = gEngfuncs.COM_ParseFile(pfile, token);
			}
			else
			{
				// See if it's a Class
				for (int i = 1; i <= PC_ENGINEER; i++)
				{
					if ( !strcmp(token, sTFClasses[i]) )
					{
						// Save it off
						iPlayerClass = i;

						// Get the button text
						pfile = gEngfuncs.COM_ParseFile(pfile, token);
						break;
					}
				}
			}

			// Get the button bound key
			strncpy( cBoundKey, token, 32 );
			cText[31] = '\0';

			// Get the button text
			pfile = gEngfuncs.COM_ParseFile(pfile, token);
			strncpy( cText, token, 32 );
			cText[31] = '\0';

			// save off the last button text we've come across (for error reporting)
			 strncpy(szLastButtonText,  cText, sizeof(szLastButtonText) );

			// Get the button command
			pfile = gEngfuncs.COM_ParseFile(pfile, token);
			strncpy( cCommand, token, cCommandLength );
			cCommand[cCommandLength - 1] = '\0';

			// Custom button handling
			if ( iCustom )
			{
				pButton = CreateCustomButton( cText, cCommand );

				// Get the next token to see if we're a menu
				pfile = gEngfuncs.COM_ParseFile(pfile, token);

				if ( token[0] == '{' )
				{
					 strncpy(cCommand,  token, sizeof(cCommand) );
				}
				else
				{
					bGetExtraToken = false;
				}
			}
			else if ( szMap[0] != '\0' )
			{
				// create a map button
				pButton = new MapButton(szMap, cText,0, BUTTON_SIZE_Y * m_pCurrentCommandMenu->GetNumButtons(), CMENU_SIZE_X, BUTTON_SIZE_Y);
			}
			else if ( iTeamOnly )
			{
				// button that only shows up if the player is on team iTeamOnly
				pButton = new TeamOnlyCommandButton( iTeamOnly, cText,0, BUTTON_SIZE_Y * m_pCurrentCommandMenu->GetNumButtons(), CMENU_SIZE_X, BUTTON_SIZE_Y );
			}
			else
			{
				// normal button
				pButton = new CommandButton( iPlayerClass, cText,0, BUTTON_SIZE_Y * m_pCurrentCommandMenu->GetNumButtons(), CMENU_SIZE_X, BUTTON_SIZE_Y );
			}

			// add the button into the command menu
			if ( pButton )
			{
				m_pCurrentCommandMenu->AddButton( pButton );
				pButton->setBoundKey( cBoundKey[0] );
				pButton->setParentMenu( m_pCurrentCommandMenu );

				// Override font in CommandMenu
				pButton->setFont( Scheme::sf_primary3 );
			}

			// Find out if it's a submenu or a button we're dealing with
			if ( cCommand[0] == '{' )
			{
				if ( m_iNumMenus >= MAX_MENUS )
				{
					gEngfuncs.Con_Printf( "Too many menus in commandmenu.txt past '%s'\n", szLastButtonText );
				}
				else
				{
					// Create the menu
					m_pCommandMenus[m_iNumMenus] = CreateSubMenu(pButton, m_pCurrentCommandMenu);
					m_pCurrentCommandMenu = m_pCommandMenus[m_iNumMenus];
					m_iNumMenus++;
				}
			}
			else if ( !iCustom )
			{
				// Create the button and attach it to the current menu
				pButton->addActionSignal(new CMenuHandler_StringCommand(cCommand));
				// Create an input signal that'll popup the current menu
				pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
			}

			// Get the next token
			if ( bGetExtraToken )
			{
				pfile = gEngfuncs.COM_ParseFile(pfile, token);
			}
		}

		// Move back up a menu
		m_pCurrentCommandMenu = m_pCurrentCommandMenu->GetParentMenu();

		pfile = gEngfuncs.COM_ParseFile(pfile, token);
	}
}
catch( CException *e )
{
	e;
	//e->Delete();
	e = NULL;
	m_iInitialized = false;
	Print( "\n\n!!!! EXCEPTION !!!!!\n\n" );
	return;
}

	SetCurrentMenu( NULL );
	SetCurrentCommandMenu( NULL );
	gEngfuncs.COM_FreeFile( pfile );

	m_iInitialized = true;*/
}

void TeamFortressViewport::ToggleServerBrowser()
{
	//if (!m_iInitialized)
	//	return;

	if (!m_pServerBrowser)
		return;

	if (m_pServerBrowser->isVisible())
	{
		m_pServerBrowser->setVisible(false);
	}
	else
	{
		m_pServerBrowser->setVisible(true);
	}

	UpdateCursorState();
}

bool fBroswerVisible()
{
	return gViewPort->m_pServerBrowser ? gViewPort->m_pServerBrowser->isVisible() : false;
}

//=======================================================================
void TeamFortressViewport::ShowCommandMenu(int menuIndex)
{
	/*if (!m_iInitialized)
		return;

	//Already have a menu open.
	if ( m_pCurrentMenu )  
		return;

	// is the command menu open?
	if ( m_pCurrentCommandMenu == m_pCommandMenus[menuIndex] )
	{
		HideCommandMenu();
		return;
	}

	// Not visible while in intermission
	if ( gHUD.m_iIntermission )
		return;

	// Recalculate visible menus
	UpdateCommandMenu( menuIndex );
	HideVGUIMenu();

	SetCurrentCommandMenu( m_pCommandMenus[menuIndex] );
	m_flMenuOpenTime = gHUD.m_flTime;
	UpdateCursorState();

	// get command menu parameters
	for ( int i = 2; i < gEngfuncs.Cmd_Argc(); i++ )
	{
		const char *param = gEngfuncs.Cmd_Argv( i - 1 );
		if ( param )
		{
			if ( m_pCurrentCommandMenu->KeyInput(param[0]) )
			{
				// kill the menu open time, since the key input is final
				HideCommandMenu();
			}
		}
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: Handles the key input of "-commandmenu"
// Input  :
//-----------------------------------------------------------------------------
void TeamFortressViewport::InputSignalHideCommandMenu()
{
	if (!m_iInitialized)
		return;

	// if they've just tapped the command menu key, leave it open
	if ((m_flMenuOpenTime + 0.3) > gHUD.m_flTime)
		return;

	HideCommandMenu();
}

//-----------------------------------------------------------------------------
// Purpose: Hides the command menu
//-----------------------------------------------------------------------------
void TeamFortressViewport::HideCommandMenu(void)
{
	if (!m_iInitialized)
		return;

	if (m_pCommandMenus[0])
	{
		m_pCommandMenus[0]->ClearButtonsOfArmedState();
	}

	m_flMenuOpenTime = 0.0f;
	SetCurrentCommandMenu(NULL);
	UpdateCursorState();
}

//-----------------------------------------------------------------------------
// Purpose: Bring up the scoreboard
//-----------------------------------------------------------------------------
void TeamFortressViewport::ShowScoreBoard(void)
{
	if (m_pScoreBoard)
	{
		// No Scoreboard in single-player
		//if ( gEngfuncs.GetMaxClients() > 1 )
		{
			m_pScoreBoard->Open();
			UpdateCursorState();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the scoreboard is up
//-----------------------------------------------------------------------------
bool TeamFortressViewport::IsScoreBoardVisible(void)
{
	if (m_pScoreBoard)
		return m_pScoreBoard->isVisible();

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Hide the scoreboard
//-----------------------------------------------------------------------------
void TeamFortressViewport::HideScoreBoard(void)
{
	// Prevent removal of scoreboard during intermission
	if (gHUD.m_iIntermission)
		return;

	if (m_pScoreBoard)
	{
		m_pScoreBoard->setVisible(false);

		GetClientVoiceMgr()->StopSquelchMode();

		UpdateCursorState();
	}
}

// Set the submenu of the Command Menu
void TeamFortressViewport::SetCurrentCommandMenu(CCommandMenu *pNewMenu)
{
	for (int i = 0; i < m_iNumMenus; i++)
		m_pCommandMenus[i]->setVisible(false);

	m_pCurrentCommandMenu = pNewMenu;

	if (m_pCurrentCommandMenu)
		m_pCurrentCommandMenu->MakeVisible(NULL);
}

void TeamFortressViewport::UpdateCommandMenu(int menuIndex)
{
	m_pCommandMenus[menuIndex]->RecalculateVisibles(0, false);
	m_pCommandMenus[menuIndex]->RecalculatePositions(0);
}

void TeamFortressViewport::UpdateSpectatorMenu()
{
	char sz[64];

	if (!m_pSpectatorMenu)
		return;

	if (m_iUser1)
	{
		m_pSpectatorMenu->setVisible(true);

		if (m_iUser2 > 0)
		{
			// Locked onto a target, show the player's name
			 _snprintf(sz, sizeof(sz),  "#Spec_Mode%d : %s",  m_iUser1,  g_PlayerInfoList[m_iUser2].name );
			m_pSpectatorLabel->setText(CHudTextMessage::BufferedLocaliseTextString(sz));
		}
		else
		{
			 _snprintf(sz, sizeof(sz),  "#Spec_Mode%d",  m_iUser1 );
			m_pSpectatorLabel->setText(CHudTextMessage::BufferedLocaliseTextString(sz));
		}
	}
	else
	{
		m_pSpectatorMenu->setVisible(false);
	}
}
void TeamFortressViewport::UpdateSpectatorPanel()
{
	m_iUser1 = g_iUser1;
	m_iUser2 = g_iUser2;
	m_iUser3 = g_iUser3;

	if (!m_pSpectatorPanel)
		return;

	//if ( g_iUser1 && gHUD.m_pCvarDraw->value && !gHUD.m_iIntermission)	// don't draw in dev_overview mode
	if (g_iUser1)
	{
		char bottomText[128];
		char helpString2[128];
		char tempString[128];
		char *name;
		int player = 0;

		// check if spectator combinations are still valid
		gHUD.m_Spectator.CheckSettings();

		if (!m_pSpectatorPanel->isVisible())
		{
			m_pSpectatorPanel->setVisible(true); // show spectator panel, but
			m_pSpectatorPanel->ShowMenu(false);	 // dsiable all menus/buttons

			_snprintf(tempString, sizeof(tempString) - 1, "%c%s", HUD_PRINTCENTER, CHudTextMessage::BufferedLocaliseTextString("#Spec_Duck"));
			tempString[sizeof(tempString) - 1] = '\0';

			gHUD.m_TextMessage.MsgFunc_TextMsg(NULL, strlen(tempString) + 1, tempString);
		}

		 _snprintf(bottomText, sizeof(bottomText),  "#Spec_Mode%d",  g_iUser1 );
		 _snprintf(helpString2, sizeof(helpString2),  "#Spec_Mode%d",  g_iUser1 );

		if (gEngfuncs.IsSpectateOnly())
			strncat(helpString2, " - HLTV", 7);

		// check if we're locked onto a target, show the player's name
		if ((g_iUser2 > 0) && (g_iUser2 <= gEngfuncs.GetMaxClients()) && (g_iUser1 != OBS_ROAMING))
		{
			player = g_iUser2;
		}

		// special case in free map and inset off, don't show names
		if ((g_iUser1 == OBS_MAP_FREE) && !gHUD.m_Spectator.m_pip->value)
			name = NULL;
		else
			name = g_PlayerInfoList[player].name;

		// create player & health string
		if (player && name)
		{
			 strncpy(bottomText,  name, sizeof(bottomText) );
		}

		// in first person mode colorize player names
		/*if ( (g_iUser1 == OBS_IN_EYE) && player )
		{
			float * color = GetClientColor( player );
			int r = color[0]*255;
			int g = color[1]*255;
			int b = color[2]*255;
			
			// set team color, a bit transparent
			m_pSpectatorPanel->m_BottomMainLabel->setFgColor(r,g,b,0);
		}
		else
		{	// restore GUI color*/
		m_pSpectatorPanel->m_BottomMainLabel->setFgColor(143, 143, 54, 0);
		//}

		// add sting auto if we are in auto directed mode
		if (gHUD.m_Spectator.m_autoDirector->value)
		{
			char tempString[128];
			 _snprintf(tempString, sizeof(tempString),  "#Spec_Auto %s",  helpString2 );
			 strncpy(helpString2,  tempString, sizeof(helpString2) );
		}

		m_pSpectatorPanel->m_BottomMainLabel->setText(CHudTextMessage::BufferedLocaliseTextString(bottomText));

		// update extra info field
		char szText[64];

		if (gEngfuncs.IsSpectateOnly())
		{
			// in HLTV mode show number of spectators
			_snprintf(szText, 63, "%s: %d", CHudTextMessage::BufferedLocaliseTextString("#Spectators"), gHUD.m_Spectator.m_iSpectatorNumber);
		}
		else
		{
			// otherwise show map name
			char szMapName[64];
			COM_FileBase(gEngfuncs.pfnGetLevelName(), szMapName);

			_snprintf(szText, 63, "%s: %s", CHudTextMessage::BufferedLocaliseTextString("#Spec_Map"), szMapName);
		}

		szText[63] = 0;

		m_pSpectatorPanel->m_ExtraInfo->setText(szText);

		/*
		int timer = (int)( gHUD.m_roundTimer.m_flTimeEnd - gHUD.m_flTime );

		if ( timer < 0 )
			 timer	= 0;

		_snprintf ( szText, 63, "%d:%02d\n", (timer / 60), (timer % 60) );
		
		szText[63] = 0;
				
		m_pSpectatorPanel->m_CurrentTime->setText( szText ); */

		// update spectator panel
		gViewPort->m_pSpectatorPanel->Update();
	}
	else
	{
		if (m_pSpectatorPanel->isVisible())
		{
			m_pSpectatorPanel->setVisible(false);
			m_pSpectatorPanel->ShowMenu(false); // dsiable all menus/buttons
		}
	}

	m_flSpectatorPanelLastUpdated = gHUD.m_flTime + 1.0; // update every seconds
}

//======================================================================
void TeamFortressViewport::CreateScoreBoard(void)
{
	startdbg;

	int xdent = SBOARD_INDENT_X, ydent = SBOARD_INDENT_Y;
	if (ScreenWidth == 512)
	{
		xdent = SBOARD_INDENT_X_512;
		ydent = SBOARD_INDENT_Y_512;
	}
	else if (ScreenWidth == 400)
	{
		xdent = SBOARD_INDENT_X_400;
		ydent = SBOARD_INDENT_Y_400;
	}

	m_pScoreBoard = new ScorePanel(xdent, ydent, ScreenWidth - (xdent * 2), ScreenHeight - (ydent * 2));
	m_pScoreBoard->setParent(this);
	m_pScoreBoard->setVisible(false);

	logfile << Logger::LOG_INFO << "[Scoreboard: Complete]\n";

	enddbg;
}

void TeamFortressViewport::CreateServerBrowser(void)
{
	m_pServerBrowser = new ServerBrowser(0, 0, ScreenWidth, ScreenHeight);
	m_pServerBrowser->setParent(this);
	m_pServerBrowser->setVisible(false);
}

//======================================================================
// Set the VGUI Menu
void TeamFortressViewport::SetCurrentMenu(CMenuPanel *pMenu)
{
	m_pCurrentMenu = pMenu;
	if (m_pCurrentMenu)
	{
		// Don't open menus in demo playback
		if (gEngfuncs.pDemoAPI->IsPlayingback())
			return;

		m_pCurrentMenu->Open();
	}
}

//================================================================
// Text Window
CMenuPanel *TeamFortressViewport::CreateTextWindow(int iTextToShow)
{
	//char sz[256];
	char *cText = "N/A";
	char *pfile = NULL;
	static const int MAX_TITLE_LENGTH = 32;
	char cTitle[MAX_TITLE_LENGTH];

	if (iTextToShow == SHOW_MOTD)
	{
		if (!m_szServerName || !m_szServerName[0])
			 strncpy(cTitle,  "Half-Life", sizeof(cTitle) );
		else
			strncpy(cTitle, m_szServerName, MAX_TITLE_LENGTH);
		cTitle[MAX_TITLE_LENGTH - 1] = 0;
		cText = m_szMOTD;
	}
	/*else if ( iTextToShow == SHOW_MAPBRIEFING )
	{
		// Get the current mapname, and open it's map briefing text
		if (m_sMapName && m_sMapName[0])
		{
			 strncpy(sz,  "maps/", sizeof(sz) );
			strcat( sz, m_sMapName );
			strcat( sz, ".txt" );
		}
		else
		{
			const char *level = gEngfuncs.pfnGetLevelName();
			if (!level)
				return NULL;

			 strncpy(sz,  level, sizeof(sz) );
			char *ch = strchr( sz, '.' );
			*ch = '\0';
			strcat( sz, ".txt" );

			// pull out the map name
			 strncpy(m_sMapName,  level, sizeof(m_sMapName) );
			ch = strchr( m_sMapName, '.' );
			if ( ch )
			{
				*ch = 0;
			}

			ch = strchr( m_sMapName, '/' );
			if ( ch )
			{
				// move the string back over the '/'
				memmove( m_sMapName, ch+1, strlen(ch)+1 );
			}
		}

		pfile = (char*)gEngfuncs.COM_LoadFile( sz, 5, NULL );

		if (!pfile)
			return NULL;

		cText = pfile;

		strncpy( cTitle, m_sMapName, MAX_TITLE_LENGTH );
		cTitle[MAX_TITLE_LENGTH-1] = 0;
	}
	else if ( iTextToShow == SHOW_CLASSDESC )
	{
		switch ( g_iPlayerClass )
		{
		case PC_SCOUT:		cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_scout" ); 
							CHudTextMessage::LocaliseTextString( "#Title_scout", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_SNIPER:		cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_sniper" );
							CHudTextMessage::LocaliseTextString( "#Title_sniper", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_SOLDIER:	cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_soldier" );
							CHudTextMessage::LocaliseTextString( "#Title_soldier", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_DEMOMAN:	cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_demoman" );
							CHudTextMessage::LocaliseTextString( "#Title_demoman", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_MEDIC:		cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_medic" );
							CHudTextMessage::LocaliseTextString( "#Title_medic", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_HVYWEAP:	cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_hwguy" );
							CHudTextMessage::LocaliseTextString( "#Title_hwguy", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_PYRO:		cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_pyro" );
							CHudTextMessage::LocaliseTextString( "#Title_pyro", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_SPY:		cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_spy" );
							CHudTextMessage::LocaliseTextString( "#Title_spy", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_ENGINEER:	cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_engineer" );
							CHudTextMessage::LocaliseTextString( "#Title_engineer", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_CIVILIAN:	cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_civilian" );
							CHudTextMessage::LocaliseTextString( "#Title_civilian", cTitle, MAX_TITLE_LENGTH ); break;
		default:
			return NULL;
		}

		if ( g_iPlayerClass == PC_CIVILIAN )
		{
			 strncpy(sz,  "classes/long_civilian.txt", sizeof(sz) );
		}
		else
		{
			 _snprintf(sz, sizeof(sz),  "classes/long_%s.txt",  sTFClassSelection[ g_iPlayerClass ] );
		}
		char *pfile = (char*)gEngfuncs.COM_LoadFile( sz, 5, NULL );
		if (pfile)
		{
			cText = pfile;
		}
	}*/

	// if we're in the game (ie. have selected a class), flag the menu to be only grayed in the dialog box, instead of full screen
	CMenuPanel *pMOTDPanel = CMessageWindowPanel_Create(cText, cTitle, g_iPlayerClass == PC_UNDEFINED, false, 0, 0, ScreenWidth, ScreenHeight);
	pMOTDPanel->setParent(this);

	if (pfile)
		gEngfuncs.COM_FreeFile(pfile);

	return pMOTDPanel;
}

//================================================================
// VGUI Menus
void TeamFortressViewport::ShowVGUIMenu(int iMenu)
{
	startdbg;
	dbg("TeamFortressViewport::ShowVGUIMenu - Begin");
	CMenuPanel *pNewMenu = NULL;

	// Don't open menus in demo playback
	if (gEngfuncs.pDemoAPI->IsPlayingback())
		return;

	// Don't create one if it's already in the list
	/*dbg( "Check if menu is already shown" );
	if (m_pCurrentMenu)
	{
		CMenuPanel *pMenu = m_pCurrentMenu;
		while (pMenu != NULL)
		{
			if (pMenu->GetMenuID() == iMenu)
				return;
			pMenu = pMenu->GetNextMenu();
		}
	}*/

	switch (iMenu)
	{
	case 0:
		dbg("Hide Menu");
		HideVGUIMenu();
		break;

	case MENU_INTRO:
		dbg("MENU_INTRO");
		pNewMenu = CreateTextWindow(SHOW_MOTD);
		break;

	case MENU_STORE:
	case MENU_STOREBUY:
	case MENU_STORESELL:
	case MENU_STORAGE:
		//Hide all visible menus first
		dbg("MENU_STORE/MENU_STOREBUY/MENU_STORESELL/MENU_STORAGE");
		if (m_pCurrentMenu)
			HideVGUIMenu();
		pNewMenu = ShowStoreMenu(iMenu); //Master Sword
		if (pNewMenu)
			pNewMenu->Reset();
		break;

	case MENU_CONTAINER:
		//Can only show container if there's no current window open
		dbg("MENU_CONTAINER");
		if (!m_pCurrentMenu && !(gHUD.m_iHideHUDDisplay & HIDEHUD_ALL))
		{
			pNewMenu = m_pContainerMenu;
			pNewMenu->Reset();
		}
		break;

	case MENU_NEWCHARACTER:
		//Hide all visible menus first
		dbg("MENU_NEWCHARACTER (Hide old)");
		HideVGUIMenu();

		dbg("MENU_NEWCHARACTER (Create new)");
		pNewMenu = CreateNewCharacterPanel();
		pNewMenu->setParent(this);
		dbg("MENU_NEWCHARACTER (Reset)");
		pNewMenu->Reset();
		m_Menus.add(pNewMenu);
		break;

	case MENU_STATS:
		//Can only show stats if there's no current window open
		dbg("MENU_STATS");
		if (!m_pCurrentMenu && !(gHUD.m_iHideHUDDisplay & HIDEHUD_ALL))
		{
			pNewMenu = m_pStatMenu;
			pNewMenu->Reset();
		}
		break;

	case MENU_SPAWN:
		//Hide everything and show the blank screen
		dbg("MENU_SPAWN");
		HideVGUIMenu();
		pNewMenu = m_pSpawnScreen;
		pNewMenu->Reset();
		break;

	case MENU_LOCAL: // MiB MAR2015_01 [LOCAL_PANEL] - Show the local panel
		dbg("MENU_LOCAL");
		HideVGUIMenu();
		pNewMenu = m_pLocalizedMenu;
		break;

	case MENU_MAIN:
		dbg("MENU_MAIN");
		if (!m_pCurrentMenu && !(gHUD.m_iHideHUDDisplay & HIDEHUD_ALL))
		{
			pNewMenu = m_pMainMenu;
			pNewMenu->Reset();
		}
		break;

	default:
		dbg("Unknown Menu");
		break;
	}

	if (!pNewMenu)
		return;

	// Close the Command Menu if it's open
	dbg("Call HideCommandMenu");
	HideCommandMenu();

	dbg("Call SetMenuID/SetActive");
	pNewMenu->SetMenuID(iMenu);
	pNewMenu->SetActive(true);

	// See if another menu is visible, and if so, cache this one for display once the other one's finished
	if (m_pCurrentMenu)
	{
		m_pCurrentMenu->SetNextMenu(pNewMenu);
	}
	else
	{
		dbg("Call Open");
		m_pCurrentMenu = pNewMenu;
		m_pCurrentMenu->Open();

		dbg("Call UpdateCursorState");
		UpdateCursorState();
	}

	enddbg;
}

// Removes all VGUI Menu's onscreen
void TeamFortressViewport::HideVGUIMenu()
{
	VGUI::HideMenu(m_pCurrentMenu);
	//while (m_pCurrentMenu)
	//{
	//	HideTopMenu();
	//}
}

// Remove the top VGUI menu, and bring up the next one
void TeamFortressViewport::HideTopMenu()
{
	if (m_pCurrentMenu)
	{
		CMenuPanel *pNextMenu = m_pCurrentMenu->GetNextMenu();

		// Close the top one
		m_pCurrentMenu->Close();

		// Bring up the next one
		gViewPort->SetCurrentMenu(pNextMenu);
	}

	UpdateCursorState();
}

// Return TRUE if the HUD's allowed to print text messages
bool TeamFortressViewport::AllowedToPrintText(void)
{
	// Prevent text messages when fullscreen menus are up
	//Master Sword: Print messages always except when spawning or creating
	//a new character
	/*if ( m_pCurrentMenu )
	{
		int iId = m_pCurrentMenu->GetMenuID();
		if ( iId == MENU_SPAWN || iId == MENU_NEWCHARACTER )
			return FALSE;
	}*/

	return TRUE;
}

//======================================================================================
// TEAM MENU
//======================================================================================
// Bring up the Team selection Menu
/*CMenuPanel* TeamFortressViewport::ShowTeamMenu()
{
	// Don't open menus in demo playback
	if ( gEngfuncs.pDemoAPI->IsPlayingback() )
		return NULL;

	return m_pTeamMenu;
}

void TeamFortressViewport::CreateTeamMenu()
{
	// Create the panel
	m_pTeamMenu = new CTeamMenuPanel(100, false, 0, 0, ScreenWidth, ScreenHeight);
	m_pTeamMenu->setParent( this );
	m_pTeamMenu->setVisible( false );
}*/

//======================================================================================
// STORE MENU
//======================================================================================
// Bring up the Store buy/sell selection Menu
CMenuPanel *TeamFortressViewport::ShowStoreMenu(int iStoreType)
{
	// Don't open menus in demo playback
	if (gEngfuncs.pDemoAPI->IsPlayingback())
		return NULL;

	//Special circumstance: inventory items for packs not being held (STORE_INV)
	if (CStorePanel::iStoreBuyFlags & STORE_INV)
		iStoreType = MENU_STOREBUY;

	switch (iStoreType)
	{
	case MENU_STORE:
		return m_pStoreMenu;

	case MENU_STOREBUY:
		return m_pStoreBuyMenu;

	case MENU_STORESELL:
		return m_pStoreSellMenu;

	case MENU_STORAGE:
		return m_pStoreStorageMenu;

	default:
		return NULL;
	}
}

void CreateStoreMenus()
{
	if (gViewPort)
		gViewPort->CreateStoreMenu();
}
void TeamFortressViewport::CreateStoreMenu()
{
	if (m_pStoreMenu)
		return;
	// Create the panel
	m_pStoreMenu = new CStoreMenuPanel(100, false, 0, 0, ScreenWidth, ScreenHeight);
	m_pStoreMenu->setParent(this);
	m_pStoreMenu->setVisible(false);

	// Create the buy panel
	m_pStoreBuyMenu = new CStoreBuyPanel(100, false, 0, 0, ScreenWidth, ScreenHeight);
	m_pStoreBuyMenu->setParent(this);

	// Create the sell panel
	m_pStoreSellMenu = new CStoreSellPanel(this);

	// Create the storage panel
	m_pStoreStorageMenu = new CStoragePanel(this);

	// Create the container panel
	m_pContainerMenu = new CContainerPanel(100, false, 0, 0, ScreenWidth, ScreenHeight);
	m_pContainerMenu->setParent(this);
	m_pContainerMenu->setVisible(false);
}
void TeamFortressViewport::CreateVGUIMenus()
{
	startdbg;

	m_Menus.add(m_pHUDPanel = CreateHUDPanel(this)); // Create the HUD panel

	m_Menus.add(new CStatPanel(this)); // Create the stats panel

	m_Menus.add(m_pSpawnScreen = new CSpawnPanel(this)); // Create the spawn panel

	m_Menus.add(CreateHUD_MenuMain(this)); //Create Main menu

	m_Menus.add(CreateHUD_MenuInteract(this)); //Create Interact menu

	m_Menus.add(m_pLocalizedMenu = new CLocalizedPanel(this)); // MiB MAR2015_01 [LOCAL_PANEL] - Add local panel to list and set pointer

	for (int i = 0; i < m_Menus.size(); i++)
		m_Menus[i]->setVisible(false);

	m_pHUDPanel->setVisible(true);

	logfile << Logger::LOG_INFO << "[Create VGUI Menus: Complete]\n";
	enddbg;
}

//======================================================================================
// SPECTATOR MENU
//======================================================================================
// Spectator "Menu" explaining the Spectator buttons
/*void TeamFortressViewport::CreateSpectatorMenu()
{
	// Create the Panel
	m_pSpectatorMenu = new CTransparentPanel(100, 0, ScreenHeight - YRES(60), ScreenWidth, YRES(60));
	m_pSpectatorMenu->setParent(this);
	m_pSpectatorMenu->setVisible(false);

	// Get the scheme used for the Titles
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();

	// schemes
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
	SchemeHandle_t hHelpText = pSchemes->getSchemeHandle( "Primary Button Text" );

	// color schemes
	int r, g, b, a;

	// Create the title
	m_pSpectatorLabel = new Label( "Spectator", 0, 0, ScreenWidth, YRES(25) );
	m_pSpectatorLabel->setParent( m_pSpectatorMenu );
	m_pSpectatorLabel->setFont( pSchemes->getFont(hTitleScheme) );
	pSchemes->getFgColor( hTitleScheme, r, g, b, a );
	m_pSpectatorLabel->setFgColor( r, g, b, a );
	pSchemes->getBgColor( hTitleScheme, r, g, b, a );
	m_pSpectatorLabel->setBgColor( r, g, b, 255 );
	m_pSpectatorLabel->setContentAlignment( vgui::Label::a_north );

	// Create the Help
	Label *pLabel = new Label( CHudTextMessage::BufferedLocaliseTextString( "#Spec_Help" ), 0, YRES(25), ScreenWidth, YRES(15) );
	pLabel->setParent( m_pSpectatorMenu );
	pLabel->setFont( pSchemes->getFont(hHelpText) );
	pSchemes->getFgColor( hHelpText, r, g, b, a );
	pLabel->setFgColor( r, g, b, a );
	pSchemes->getBgColor( hHelpText, r, g, b, a );
	pLabel->setBgColor( r, g, b, 255 );
	pLabel->setContentAlignment( vgui::Label::a_north );

	pLabel = new Label( CHudTextMessage::BufferedLocaliseTextString( "#Spec_Help2" ), 0, YRES(40), ScreenWidth, YRES(20) );
	pLabel->setParent( m_pSpectatorMenu );
	pLabel->setFont( pSchemes->getFont(hHelpText) );
	pSchemes->getFgColor( hHelpText, r, g, b, a );
	pLabel->setFgColor( r, g, b, a );
	pSchemes->getBgColor( hHelpText, r, g, b, a );
	pLabel->setBgColor( r, g, b, 255 );
	pLabel->setContentAlignment( vgui::Label::a_center );
}*/

//======================================================================================
// UPDATE HUD SECTIONS
//======================================================================================
// We've got an update on player info
// Recalculate any menus that use it.
void TeamFortressViewport::UpdateOnPlayerInfo()
{
	if (m_pScoreBoard)
		m_pScoreBoard->Update();
}

void TeamFortressViewport::UpdateCursorState()
{
	// Need cursor if any VGUI window is up
	if (m_pCurrentMenu)
	{
		m_pCurrentMenu->UpdateCursorState();
		return;
	}

	if (m_pSpectatorPanel->m_menuVisible || m_pServerBrowser->isVisible() || GetClientVoiceMgr()->IsInSquelchMode())
	{
		//Exclude menu panels that don't use the mouse (stats display)
		if ((!m_pStatMenu || !m_pStatMenu->IsActive()) &&
			(!m_pSpawnScreen || !m_pSpawnScreen->IsActive()))
		{
			g_iVisibleMouse = true;
			App::getInstance()->setCursorOveride(App::getInstance()->getScheme()->getCursor(Scheme::SchemeCursor::scu_arrow));
		}
		return;
	}
	else if (m_pCurrentCommandMenu)
	{
		// commandmenu doesn't have cursor if hud_capturemouse is turned off
		if (gHUD.m_pCvarStealMouse->value != 0.0f)
		{
			g_iVisibleMouse = true;
			App::getInstance()->setCursorOveride(App::getInstance()->getScheme()->getCursor(Scheme::SchemeCursor::scu_arrow));
			return;
		}
	}

	// Don't reset mouse in demo playback
	if (!gEngfuncs.pDemoAPI->IsPlayingback())
	{
		IN_ResetMouse();
	}

	//IN_ResetMouse();
	g_iVisibleMouse = false;
	App::getInstance()->setCursorOveride(App::getInstance()->getScheme()->getCursor(Scheme::SchemeCursor::scu_none));
}

void TeamFortressViewport::UpdateHighlights()
{
	if (m_pCurrentCommandMenu)
		m_pCurrentCommandMenu->MakeVisible(NULL);
}

void TeamFortressViewport::GetAllPlayersInfo(void)
{
	for (int i = 1; i < MAX_PLAYERS; i++)
	{
		GetPlayerInfo(i, &g_PlayerInfoList[i]);

		if (g_PlayerInfoList[i].thisplayer)
			m_pScoreBoard->m_iPlayerNum = i; // !!!HACK: this should be initialized elsewhere... maybe gotten from the engine
	}
}

void TeamFortressViewport::paintBackground()
{
	if (m_pScoreBoard)
	{
		int x, y;
		getApp()->getCursorPos(x, y);
		m_pScoreBoard->cursorMoved(x, y, m_pScoreBoard);
	}

	// See if the command menu is visible and needs recalculating due to some external change
	if (g_iTeamNumber != m_iCurrentTeamNumber)
	{
		UpdateCommandMenu(m_StandardMenu);

		m_iCurrentTeamNumber = g_iTeamNumber;
	}

	if (g_iPlayerClass != m_iCurrentPlayerClass)
	{
		UpdateCommandMenu(m_StandardMenu);

		m_iCurrentPlayerClass = g_iPlayerClass;
	}

	// See if the Spectator Menu needs to be update
	if ((g_iUser1 != m_iUser1 || g_iUser2 != m_iUser2) ||
		(m_flSpectatorPanelLastUpdated < gHUD.m_flTime))
	{
		UpdateSpectatorPanel();
	}

	// Update the Scoreboard, if it's visible
	if (m_pScoreBoard->isVisible() && (m_flScoreBoardLastUpdated < gHUD.m_flTime))
	{
		m_pScoreBoard->Update();
		m_flScoreBoardLastUpdated = gHUD.m_flTime + 0.5;
	}

	int extents[4];
	getAbsExtents(extents[0], extents[1], extents[2], extents[3]);
	VGui_ViewportPaintBackground(extents);
}

//================================================================
// Input Handler for Drag N Drop panels
void CDragNDropHandler::cursorMoved(int x, int y, Panel *panel)
{
	if (m_bDragging)
	{
		App::getInstance()->getCursorPos(x, y);
		m_pPanel->setPos(m_iaDragOrgPos[0] + (x - m_iaDragStart[0]), m_iaDragOrgPos[1] + (y - m_iaDragStart[1]));

		if (m_pPanel->getParent() != null)
		{
			m_pPanel->getParent()->repaint();
		}
	}
}

void CDragNDropHandler::mousePressed(MouseCode code, Panel *panel)
{
	int x, y;
	App::getInstance()->getCursorPos(x, y);
	m_bDragging = true;
	m_iaDragStart[0] = x;
	m_iaDragStart[1] = y;
	m_pPanel->getPos(m_iaDragOrgPos[0], m_iaDragOrgPos[1]);
	App::getInstance()->setMouseCapture(panel);

	m_pPanel->setDragged(m_bDragging);
	m_pPanel->requestFocus();
}

void CDragNDropHandler::mouseReleased(MouseCode code, Panel *panel)
{
	m_bDragging = false;
	m_pPanel->setDragged(m_bDragging);
	App::getInstance()->setMouseCapture(null);
}

//================================================================
// Number Key Input
bool TeamFortressViewport::SlotInput(int iSlot)
{
	// If there's a menu up, give it the input
	if (m_pCurrentMenu)
		return m_pCurrentMenu->SlotInput(iSlot);

	return FALSE;
}

// Direct Key Input
bool TrapKey = false;
int TeamFortressViewport::KeyInput(int down, int keynum, const char *pszCurrentBinding)
{
	// Open Text Window?
	if (gEngfuncs.Con_IsVisible() == false)
	{
		if (m_pCurrentMenu && m_pCurrentMenu->KeyInput(down, keynum, pszCurrentBinding))
			return 0;

		//Override input for Master Sword's custom text box
		if (HUD_KeyInput(down, keynum, pszCurrentBinding))
			return 0;

		if (!down)
			return 1;

		//Selecting a menu item (0-9)
		if (gHUD.m_Menu->m_fMenuDisplayed && keynum >= '0' && keynum <= '9')
		{
			int Num = keynum - '0';
			if (!Num)
				Num = 10;
			gHUD.m_Menu->SelectMenuItem(Num);
			return 0; //Override the binding
		}

		int iMenuID = m_pCurrentMenu ? m_pCurrentMenu->GetMenuID() : 0;

		// Get number keys as Input for Team/Class menus
		if (m_pCurrentMenu && m_pCurrentMenu->m_Flags & MENUFLAG_TRAPNUMINPUT)
		{
			for (int i = '0'; i <= '9'; i++)
			{
				if (down && (keynum == i))
				{
					SlotInput((i - '0') - 1);
					return 0;
				}
			}
		}

		//Master Sword
		if (m_pCurrentMenu && m_pCurrentMenu->m_Flags & MENUFLAG_CLOSEONESC)
		{
			// Escape gets you out of menus
			if (keynum == K_ESCAPE)
			{
				HideTopMenu();
				return 0;
			}
		}

		if (m_pServerBrowser && m_pServerBrowser->isVisible() &&
			keynum == K_ESCAPE) // get out of the server browser
		{
			m_pServerBrowser->setVisible(false);
			UpdateCursorState();
			return 0;
		}

		// Grab enter keys to close TextWindows
		/*if ( m_pCurrentMenu && down && (keynum == K_ENTER || keynum == K_KP_ENTER || keynum == K_SPACE || keynum == K_ESCAPE) )
		{
			if ( iMenuID == MENU_MAPBRIEFING || iMenuID == MENU_INTRO || iMenuID == MENU_CLASSHELP )
			{
				HideTopMenu();
				return 0;
			}
		}*/

		// Grab jump key on Team Menu as autoassign
		/*if ( pszCurrentBinding && down && !strcmp(pszCurrentBinding, "+jump") )
		{
			if (iMenuID == MENU_TEAM)
			{
				m_pTeamMenu->SlotInput(5);
				return 0;
			}
		}*/
	}

	// if we're in a command menu, try hit one of it's buttons
	if (down && m_pCurrentCommandMenu)
	{
		// Escape hides the command menu
		if (keynum == K_ESCAPE)
		{
			HideCommandMenu();
			return 0;
		}

		// only trap the number keys
		if (keynum >= '0' && keynum <= '9')
		{
			if (m_pCurrentCommandMenu->KeyInput(keynum))
			{
				// a final command has been issued, so close the command menu
				HideCommandMenu();
			}

			return 0;
		}
	}

	return 1;
}

//================================================================
// Message Handlers
int TeamFortressViewport::MsgFunc_ValClass(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	for (int i = 0; i < 5; i++)
		m_iValidClasses[i] = READ_SHORT();

	// Force the menu to update
	UpdateCommandMenu(m_StandardMenu);

	return 1;
}

int TeamFortressViewport::MsgFunc_TeamNames(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iNumberOfTeams = READ_BYTE();

	for (int i = 0; i < m_iNumberOfTeams; i++)
	{
		int teamNum = i + 1;

		gHUD.m_TextMessage.LocaliseTextString(READ_STRING(), m_sTeamNames[teamNum], MAX_TEAMNAME_SIZE);

		// Set the team name buttons
		if (m_pTeamButtons[i])
			m_pTeamButtons[i]->setText(m_sTeamNames[teamNum]);

		// Set the disguise buttons
		if (m_pDisguiseButtons[i])
			m_pDisguiseButtons[i]->setText(m_sTeamNames[teamNum]);
	}

	// Update the Team Menu
	//	if (m_pTeamMenu)
	//		m_pTeamMenu->Update();

	return 1;
}

int TeamFortressViewport::MsgFunc_Feign(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iIsFeigning = READ_BYTE();

	// Force the menu to update
	UpdateCommandMenu(m_StandardMenu);

	return 1;
}

int TeamFortressViewport::MsgFunc_Detpack(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iIsSettingDetpack = READ_BYTE();

	// Force the menu to update
	UpdateCommandMenu(m_StandardMenu);

	return 1;
}

int TeamFortressViewport::MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iMenu = READ_BYTE();

	// Map briefing includes the name of the map (because it's sent down before the client knows what map it is)
	/*	if (iMenu == MENU_MAPBRIEFING)
	{
		strncpy( m_sMapName, READ_STRING(), sizeof(m_sMapName) );
		m_sMapName[ sizeof(m_sMapName) - 1 ] = '\0';
	}*/

	//Master Sword
	if (iMenu == MENU_STORE)
		MsgFunc_Store();

	// Bring up the menu
	ShowVGUIMenu(iMenu);

	return 1;
}

int TeamFortressViewport::MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (m_iGotAllMOTD)
		m_szMOTD[0] = 0;

	BEGIN_READ(pbuf, iSize);

	m_iGotAllMOTD = READ_BYTE();
	strncat(m_szMOTD, READ_STRING(), sizeof(m_szMOTD) - strlen(m_szMOTD));
	m_szMOTD[sizeof(m_szMOTD) - 1] = '\0';

	if (m_iGotAllMOTD)
	{
		ShowVGUIMenu(MENU_INTRO);
	}

	return 1;
}

int TeamFortressViewport::MsgFunc_BuildSt(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iBuildState = READ_BYTE();

	// Force the menu to update
	UpdateCommandMenu(m_StandardMenu);

	return 1;
}

int TeamFortressViewport::MsgFunc_RandomPC(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iRandomPC = READ_BYTE();

	return 1;
}

int TeamFortressViewport::MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	strncpy(m_szServerName, READ_STRING(), MAX_SERVERNAME_LENGTH);

	return 1;
}

int TeamFortressViewport::MsgFunc_ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	MSG_ScoreInfo(pszName, iSize, pbuf);

	return 1;
}

// Message handler for TeamScore message
// accepts three values:
//		string: team name
//		short: teams kills
//		short: teams deaths
// if this message is never received, then scores will simply be the combined totals of the players.
int TeamFortressViewport::MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	char *TeamName = READ_STRING();
	int i = 0;

	// find the team matching the name
	for (i = 1; i <= m_pScoreBoard->m_iNumTeams; i++)
	{
		if (!stricmp(TeamName, g_TeamInfo[i].name))
			break;
	}

	if (i > m_pScoreBoard->m_iNumTeams)
		return 1;

	// use this new score data instead of combined player scoresw
	g_TeamInfo[i].scores_overriden = TRUE;
	g_TeamInfo[i].frags = READ_SHORT();
	g_TeamInfo[i].deaths = READ_SHORT();

	return 1;
}

// Message handler for TeamInfo message
// accepts two values:
//		byte: client number
//		string: client team name
int TeamFortressViewport::MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	if (!m_pScoreBoard)
		return 1;

	BEGIN_READ(pbuf, iSize);
	short cl = READ_BYTE();

	if (cl > 0 && cl <= MAX_PLAYERS)
	{
		// set the players team
		strncpy(g_PlayerExtraInfo[cl].teamname, READ_STRING(), MAX_TEAM_NAME);
		g_PlayerExtraInfo[cl].TeamID = READ_LONG();
	}

	// rebuild the list of teams
	m_pScoreBoard->RebuildTeams();

	return 1;
}

void TeamFortressViewport::DeathMsg(int killer, int victim)
{
	m_pScoreBoard->DeathMsg(killer, victim);
}

int TeamFortressViewport::MsgFunc_Spectator(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	short cl = READ_BYTE();
	if (cl > 0 && cl <= MAX_PLAYERS)
	{
		g_IsSpectator[cl] = READ_BYTE();
	}

	return 1;
}

int TeamFortressViewport::MsgFunc_AllowSpec(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iAllowSpectators = READ_BYTE();

	// Force the menu to update
	UpdateCommandMenu(m_StandardMenu);

	return 1;
}
void ShowVGUIMenu(int iMenu)
{
	if (gViewPort)
		gViewPort->ShowVGUIMenu(iMenu);
}

//Scroll a HUD item up/down, such as the stat window or an onscreen console
void __CmdFunc_HUDScroll()
{
	if (!gViewPort || gEngfuncs.Cmd_Argc() < 2)
		return;

	hudscroll_e ScrollCmd = HUDSCROLL_DOWN;
	msstring Cmd = gEngfuncs.Cmd_Argv(1);
	if (Cmd == "down")
		ScrollCmd = HUDSCROLL_DOWN;
	else if (Cmd == "up")
		ScrollCmd = HUDSCROLL_UP;
	else if (Cmd == "select")
		ScrollCmd = HUDSCROLL_SELECT;
	else
		return; //Invalid command

	if (gViewPort->m_pCurrentMenu && gViewPort->m_pCurrentMenu->m_Flags & MENUFLAG_TRAPSTEPINPUT)
	{
		// PGup and PGdn affect some menus
		gViewPort->m_pCurrentMenu->StepInput(ScrollCmd);
	}
	else if (gViewPort->m_pContainerMenu && gViewPort->m_pContainerMenu->isVisible())
	{
		// MIB FEB2015_21 [INV_SCROLL] - Pass to the container menu
		gViewPort->m_pContainerMenu->StepInput(ScrollCmd == HUDSCROLL_UP);
	}
	else if (!gViewPort->m_pCurrentMenu)
	{
		//If no menu open, pass down to the HUD
		HUD_StepInput(ScrollCmd);
	}
}
