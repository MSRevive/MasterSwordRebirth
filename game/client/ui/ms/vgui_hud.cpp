//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Character creation menu
//
// $Workfile:     $
// $Date: 2005/01/17 13:16:49 $
//
//-----------------------------------------------------------------------------
// $Log: vgui_HUD.cpp,v $
// Revision 1.9  2005/01/17 13:16:49  dogg
// Brand new inventory VGUI, revised item system, Magic, Mp3 support and item storage
//
// Revision 1.8  2004/11/07 01:06:28  dogg
// New Event Console
//
// Revision 1.7  2004/10/19 23:14:54  dogg
// BIG update
// Memory leak fixes
//
// Revision 1.6  2004/10/16 11:47:01  dogg
// no message
//
// Revision 1.5  2004/10/13 20:21:51  dogg
// Big update
// Netcode re-arranged
//
// Revision 1.4  2004/09/19 13:39:22  dogg
// Splah damage update
//
// Revision 1.3  2004/09/18 10:41:53  dogg
// Magic update
//
// Revision 1.2  2004/09/11 22:21:03  dogg
// fixed the <break> problems in cl_dll/MasterSword here!
//
// Revision 1.1  2004/09/07 17:06:01  reddog
// First commit! ;-)
//
//
// $NoKeywords: $
//=============================================================================

#include "msdllheaders.h"
#include "player/player.h"
#include "weapons/genericitem.h"
#include "stats/statdefs.h"

#include "VGUI_Font.h"
#include "VGUI_TextImage.h"

#include "../hud.h"
#include "../cl_util.h"
#include "cvardef.h"
#include "../parsemsg.h"

#include "vgui_int.h"
#include "vgui_teamfortressviewport.h"

//Master Sword
#include "logger.h"
#include "clglobal.h"
#include "script.h"
#include "../vgui_scorepanel.h"
#include "vgui_hud.h"
#include "vgui_mscontrols.h"
#include "vgui_menudefsshared.h"
//#include "vgui_MainMenu.h"

#include "vgui_infowin.h"
#include "vgui_startsaytext.h"
#include "vgui_eventconsole.h"
#include "vgui_id.h"
#include "vgui_health.h"
#include "vgui_vote.h"
#include "vgui_quickslot.h"

#include "vgui_status.h" //Drigien MAY2008
//------------

enum
{
	CON_EVENT,
	CON_SAYTEXT,
};

#define m_EventConsole m_Consoles[CON_EVENT]
#define m_SayTextConsole m_Consoles[CON_SAYTEXT]

class CHUDPanel : public VGUI_MainPanel
{
public:
	//Voting ----------------------------
	VGUI_VoteInfo* m_VoteInfo;

	//Health & Mana ---------------------
	VGUI_Health* m_Health;

	//Status Icons ----------------------
	VGUI_Status* m_Status; //Drigien MAY2008

	//Text Windows ----------------------
	std::vector<class CInfoWindow*> InfoWindows;
	std::vector<class CInfoWindow*> HelpWindows;
	void AddInfoWin(msstring_ref Title, msstring_ref Text);
	void AddHelpWin(msstring_ref Title, msstring_ref Text);
	void UpdateInfoWindows(std::vector<CInfoWindow*>& Windows);
	void RemoveInfoWindow(std::vector<CInfoWindow*>& Windows, int idx);

	//Event Console ---------------------
	std::vector<VGUI_EventConsole*> m_Consoles;
	void PrintEvent(Color color, msstring_ref Text);
	void PrintSayText(Color color, msstring_ref Text);
	void StepInput(hudscroll_e ScrollCmd);
	VGUI_EventConsole* m_ActiveConsole;

	//Start Say Text Panel --------------
	VGUI_SendTextPanel* m_StartSayText;
	bool KeyInput(int down, int keynum, const char* pszCurrentBinding);

	//ID Panel --------------------------
	VGUI_ID* m_ID;
	//-----------------------------------

	//Debug Text
	MSLabel* m_DebugText;
	//-----------------------------------

	//QuickSlot Text
	VGUI_QuickSlot* m_QuickSlot;
	//-----------------------------------

	CHUDPanel(Panel* pParent);

	virtual bool SlotInput(int iSlot);
	virtual void Open(void);
	virtual void Close(void);
	virtual void Think(void);
	//virtual void SetActiveInfo( int iInput );
	virtual void Initialize(void);

	std::vector<IHUD_Interface*> m_HUDElements;
};

VGUI_MainPanel* CreateHUDPanel(Panel* pParent) { return new CHUDPanel(pParent); }

// Create new Info window
void CHUDPanel::AddInfoWin(msstring_ref Title, msstring_ref Text)
{
	CInfoWindow& NewInfoWin = *new CInfoWindow(Title, Text, INFOWIN_DISPLAY_X, INFOWIN_DISPLAY_Y, this);

	NewInfoWin.m_TimeDisplayed = gpGlobals->time;
	NewInfoWin.m_Duration = INFOWIN_DURATION;
	NewInfoWin.setVisible(false);
	InfoWindows.push_back(&NewInfoWin);
}

// Create new Help window
#define INFOWIN_HELP_DISPLAY_X XRES(640) - XRES(260)
#define INFOWIN_HELP_DISPLAY_Y YRES(300)

void CHUDPanel::AddHelpWin(msstring_ref Title, msstring_ref Text)
{
	if (!EngineFunc::CVAR_GetFloat("ms_help"))
		return;

	//Replaces '|' with '\n'
	int size = strlen(Text);
	for (int i = 0; i < size; i++)
	{
		if (Text[i] == '|')
			((char*)Text)[i] = '\n';
	}

	//CInfoWindow &NewInfoWin = *new CInfoWindow( Title, Text, INFOWIN_HELP_DISPLAY_X, INFOWIN_HELP_DISPLAY_Y, this );
	CInfoWindow& NewInfoWin = *new CInfoWindow(Title, Text, INFOWIN_HELP_DISPLAY_X, YRES(10), this); //MAR2008a - moving helptip window not to overlap eventhud

	NewInfoWin.Resize();
	//NewInfoWin.setPos( XRES(640) - NewInfoWin.getWide()- XRES(60), INFOWIN_HELP_DISPLAY_Y );
	NewInfoWin.setPos(XRES(640) - NewInfoWin.getWide() - XRES(60), YRES(10)); //MAR2008a - moving helptip window not to overlap eventhud
	NewInfoWin.Title->setFgColor(0, 200, 20, 0);
	NewInfoWin.m_TimeDisplayed = gpGlobals->time;
	NewInfoWin.m_Duration = INFOWIN_DURATION + (strlen(Text) / 60.0f);
	NewInfoWin.setVisible(false);
	HelpWindows.push_back(&NewInfoWin);
}

// Write to the Event Console
void CHUDPanel::PrintEvent(Color color, msstring_ref Text)
{
	m_EventConsole->Print(color, Text);
}
// Write to the Event Console
void CHUDPanel::PrintSayText(Color color, msstring_ref Text)
{
	m_SayTextConsole->Print(color, Text);
}

// VGUI HUD Creation
// -----------------
CHUDPanel::CHUDPanel(Panel* pParent) : VGUI_MainPanel(0, 0, 0, ScreenWidth, ScreenHeight)
{
	startdbg;
	dbg("Begin");
	setParent(pParent);

	//Health
	dbg("Create Health");
	m_HUDElements.push_back(m_Health = new VGUI_Health(this));

	//Status Icons
	dbg("Create StatusIcons");
	m_HUDElements.push_back(m_Status = new VGUI_Status(this)); //Drigien MAY2008

	//ID Panel
	dbg("Create ID");
	m_ID = new VGUI_ID(this, ID_X, ID_Y);

	//Vote Info Panel
	dbg("Create Vote");
	m_HUDElements.push_back(m_VoteInfo = new VGUI_VoteInfo(this));

	//Event Console
	dbg("Create Event Console");
#define EVENTCON_SIZE_X XRES(230)
#define EVENTCON_X XRES(640) - EVENTCON_SIZE_X - XRES(20)
#define EVENTCON_Y (YRES(480) - YRES(10)) //Starts here, and moves upward as it grows
#define EVENTCON_SIZE_Y 0				  //Stars with no height.  Gains height when lines are added
	VGUI_EventConsole::prefs_t Prefs;
	Prefs.VisLines = "ms_evthud_size";
	Prefs.MaxLines = "ms_evthud_history";
	Prefs.DecayTime = "ms_evthud_decaytime";
	Prefs.BGTrans = "ms_evthud_bgtrans";
	Prefs.Width = NULL;

	m_Consoles.push_back(m_ActiveConsole = new VGUI_EventConsole(this, EVENTCON_X, EVENTCON_Y, EVENTCON_SIZE_X, EVENTCON_SIZE_Y, Prefs));

	//SayText Console
	dbg("Create SayText Console");
#define SAYTEXTCON_X XRES(10)
	//thothie was 220, moving to 180
#define SAYTEXTCON_Y YRES(180) //Starts here, and moves upward as it grows
#define SAYTEXTCON_SIZE_X XRES(300)
	Prefs.VisLines = "ms_txthud_size";
	Prefs.MaxLines = "ms_txthud_history";
	Prefs.DecayTime = "ms_txthud_decaytime";
	Prefs.BGTrans = "ms_txthud_bgtrans";
	Prefs.Width = "ms_txthud_width";

	m_Consoles.push_back(new VGUI_EventConsole(this, SAYTEXTCON_X, SAYTEXTCON_Y, SAYTEXTCON_SIZE_X, EVENTCON_SIZE_Y, Prefs, true, g_FontID));

	//Start Say text panel
	dbg("Create SayText Typing Panel");
	m_StartSayText = new VGUI_SendTextPanel(this, XRES(100), YRES(300), XRES(640) - XRES(100), YRES(16));

	//Debug Text
	dbg("Create Debug label");
	m_DebugText = new MSLabel(this, "", 0, YRES(16));

	//Quick Slot Text
	dbg("Create Quickslot Text");
	m_HUDElements.push_back(m_QuickSlot = new VGUI_QuickSlot(this));

	enddbg;
}

// Update
void CHUDPanel::Think()
{
	startdbg;
	//Update ID
	dbg("m_ID->Update( );");
	bool showHud = ShowHUD();
	m_ID->Update();

	//Update HUD Elements
	for (int i = 0; i < m_HUDElements.size(); i++)
	{
		msstring d = "HUD Elements loop: ";
		d += i;
		dbg(d);
		m_HUDElements[i]->Update();
		m_HUDElements[i]->SetVisible(showHud);
	}

	dbg("Update InfoWindows");
	UpdateInfoWindows(InfoWindows); //Update Info windows
	dbg("Update HelpWindows");
	UpdateInfoWindows(HelpWindows); //Update Help windows

	//Update Text Consoles
	for (int i = 0; i < m_Consoles.size(); i++)
	{
		msstring d = "Text Consoles: ";
		d += i;
		dbg(d);
		m_Consoles[i]->Update();
		m_Consoles[i]->setVisible(showHud);
	}

	//Update Start Say Text
	dbg("Start saytext");
	m_StartSayText->Update();
	m_StartSayText->setVisible(showHud);
	enddbg;
}

void CHUDPanel::UpdateInfoWindows(std::vector<CInfoWindow*>& Windows)
{
	//Count backwards because the objects might delete themselves
	for (int i = Windows.size() - 1; i >= 0; i--)
	{
		CInfoWindow& InfoWin = *Windows[i];
		//Update the Infowindow (for effects, like fade)
		InfoWin.Update(Windows, i);

		//Infowindow time has expired or the level has changed, delete it
		if (gpGlobals->time > InfoWin.m_TimeDisplayed + InfoWin.m_Duration)
			RemoveInfoWindow(Windows, i);
	}
}
void CHUDPanel::RemoveInfoWindow(std::vector<CInfoWindow*>& Windows, int idx)
{
	CInfoWindow* pInfoWin = Windows[idx];
	Windows.erase(std::next(Windows.begin(), idx));
	removeChild(pInfoWin);
	pInfoWin->Remove();
}
void CHUDPanel::Close(void)
{
}
//======================================
// Key inputs for the Class Menu
bool CHUDPanel::SlotInput(int iSlot)
{
	return false;
}

//======================================
// Update the menu before opening it
void CHUDPanel::Open(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: Called each time a new level is started.
//-----------------------------------------------------------------------------
void CHUDPanel::Initialize(void)
{
	MSGlobals::CurrentVote.fActive = false;

	//Remove all info windows
	int windows = InfoWindows.size(); //Store the amount, because it changes
	for (int i = 0; i < windows; i++)
		RemoveInfoWindow(InfoWindows, 0); //Keep deleting the first item
	windows = HelpWindows.size();
	for (int i = 0; i < windows; i++)
		RemoveInfoWindow(HelpWindows, 0);
	m_ID->NewLevel();

	Update(); //Update, to hide everything
}

//-----------------------------------------------------------------------------
// Purpose: PGUP/PGDN pressed
//-----------------------------------------------------------------------------
void CHUDPanel::StepInput(hudscroll_e ScrollCmd)
{
	if (ScrollCmd == HUDSCROLL_SELECT)
	{
		if (m_ActiveConsole == m_Consoles[0])
			m_ActiveConsole = m_Consoles[1];
		else if (m_ActiveConsole == m_Consoles[1])
			m_ActiveConsole = m_Consoles[0];
		m_ActiveConsole->StepInput(true);
	}
	else
		m_ActiveConsole->StepInput(ScrollCmd == HUDSCROLL_DOWN);
}
bool CHUDPanel::KeyInput(int down, int keynum, const char* pszCurrentBinding)
{
	if (m_StartSayText->isVisible())
	{
		m_StartSayText->KeyInput(down, keynum, pszCurrentBinding);
		return true;
	}
	return false;
}

void HUD_ShowInfoWin(msstring_ref Title, msstring_ref Text)
{
	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return;

	((CHUDPanel*)gViewPort->m_pHUDPanel)->AddInfoWin(Title, Text);
}
void HUD_ShowHelpWin(msstring_ref Title, msstring_ref Text)
{
	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return;

	((CHUDPanel*)gViewPort->m_pHUDPanel)->AddHelpWin(Title, Text);
}
void HUD_StepInput(hudscroll_e ScrollCmd)
{
	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return;
	((CHUDPanel*)gViewPort->m_pHUDPanel)->StepInput(ScrollCmd);
}
bool HUD_KeyInput(int down, int keynum, const char* pszCurrentBinding)
{
	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return false;
	return ((CHUDPanel*)gViewPort->m_pHUDPanel)->KeyInput(down, keynum, pszCurrentBinding);
}
void HUD_PrintEvent(Color color, msstring_ref Text)
{
	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return;
	((CHUDPanel*)gViewPort->m_pHUDPanel)->PrintEvent(color, Text);
}
void HUD_SayTextEvent(Color color, msstring_ref Text)
{
	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return;
	((CHUDPanel*)gViewPort->m_pHUDPanel)->PrintSayText(color, Text);
}

void HUD_StartSayText(int Type)
{
	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return;
	((CHUDPanel*)gViewPort->m_pHUDPanel)->m_StartSayText->Open(Type);
}
void HUD_CancelSayText()
{
	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return;
	((CHUDPanel*)gViewPort->m_pHUDPanel)->m_StartSayText->setVisible(false);
}

void HUD_DrawID(entinfo_t* pEntinfo)
{
	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return;
	((CHUDPanel*)gViewPort->m_pHUDPanel)->m_ID->Update(pEntinfo);
}

msstringlist HelpParts;
int __MsgFunc_HUDInfoMsg(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	msstring sText;
	byte Type = READ_BYTE();
	if (!Type)
	{
		//Display info window
		msstring sTitle = READ_STRING();
		sText = Localized(READ_STRING());

		HUD_ShowInfoWin(sTitle, sText);
	}
	else if (Type == 1)
	{
		//Print to Event console
		ulong lColor = READ_LONG();
		Color color(((uchar*)&lColor)[0], ((uchar*)&lColor)[1], ((uchar*)&lColor)[2], ((uchar*)&lColor)[3]);
		sText = READ_STRING();

		HUD_PrintEvent(color, sText);
	}
	else if (Type == 2)
	{
		//Help window part
		int idx = READ_BYTE();
		if (!idx)
			HelpParts.clear();
		HelpParts.push_back(READ_STRING());
	}
	else if (Type == 3)
	{
		//Print to help window
		msstring sTitle = READ_STRING();

		MSGlobals::Buffer[0] = 0;
		for (int i = 0; i < HelpParts.size(); i++)
			strncat(MSGlobals::Buffer, HelpParts[i], HelpParts[i].len());

		HUD_ShowHelpWin(sTitle, MSGlobals::Buffer);
	}
	else if (Type == 4)
	{
		//Print to saytext console
		saytext_e TextType = (saytext_e)READ_BYTE();
		sText = READ_STRING();
		Color color(255, 255, 255, 0);
		if (TextType == SAYTEXT_GLOBAL)
			color = Color(255, 255, 255, 0);
		else if (TextType == SAYTEXT_LOCAL)
			color = Color(255, 178, 0, 0);
		else if (TextType == SAYTEXT_PARTY)
			color = Color(60, 200, 20, 0);
		else if (TextType == SAYTEXT_NPC)
			color = Color(255, 178, 0, 0);

		HUD_SayTextEvent(color, sText);
	}
	return 0;
}
void __CmdFunc_StartSayText()
{
	if (gEngfuncs.Cmd_Argc() < 2)
		return;

	int Type = atoi(gEngfuncs.Cmd_Argv(1));
	if (Type >= 0 && Type <= 2)
		HUD_StartSayText(Type);
}
void __CmdFunc_QuickSlot()
{
	if (gEngfuncs.Cmd_Argc() < 2)
		return;

	msstring Type = gEngfuncs.Cmd_Argv(1);

	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return;
	((CHUDPanel*)gViewPort->m_pHUDPanel)->m_QuickSlot->Select(Type);
}

void QuickSlotPressed(bool fDown)
{
	if (gEngfuncs.Cmd_Argc() < 2)
		return;

	int Slot = atoi(gEngfuncs.Cmd_Argv(1)) - 1;
	if (Slot < 0 || Slot >= MAX_QUICKSLOTS)
		return;

	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return;
	((CHUDPanel*)gViewPort->m_pHUDPanel)->m_QuickSlot->SlotPressed(fDown, Slot);
}

void __CmdFunc_QuickSlotStart()
{
	QuickSlotPressed(true);
}
void __CmdFunc_QuickSlotEnd()
{
	QuickSlotPressed(false);
}

bool QuickSlotConfirm()
{
	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return false;
	return ((CHUDPanel*)gViewPort->m_pHUDPanel)->m_QuickSlot->ConfirmItem();
}

void dbgtxt(msstring_ref Text)
{
	if (!gViewPort || !gViewPort->m_pHUDPanel)
		return;
	((CHUDPanel*)gViewPort->m_pHUDPanel)->m_DebugText->setText(Text);
}
