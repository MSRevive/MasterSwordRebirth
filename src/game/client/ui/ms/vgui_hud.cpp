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
#include "vgui_healthretro.h"
#include "vgui_vote.h"
#include "vgui_quickslot.h"

#include "vgui_status.h" //Drigien MAY2008
//------------

enum
{
	CON_EVENT,
	CON_SAYTEXT,
};

class CHUDPanel : public VGUI_MainPanel
{
public:
	//Start Say Text Panel --------------
	VGUI_SendTextPanel* m_StartSayText;

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
	~CHUDPanel();

	virtual bool SlotInput(int iSlot) { return false; }
	virtual void Open(void) { }
	virtual void Close(void) { }
	virtual void Think(void);
	//virtual void SetActiveInfo( int iInput );
	virtual void Initialize(void);

	void AddInfoWin(msstring_ref Title, msstring_ref Text);
	void AddHelpWin(msstring_ref Title, msstring_ref Text);
	void UpdateInfoWindows(mslist<CInfoWindow*>& Windows);
	void RemoveInfoWindow(mslist<CInfoWindow*>& Windows, int idx);
	void PrintEvent(Color color, msstring_ref Text);
	void PrintSayText(Color color, msstring_ref Text);
	void StepInput(hudscroll_e ScrollCmd);
	bool KeyInput(int down, int keynum, const char* pszCurrentBinding);

private:
	std::vector<IHUD*> m_HUDElements;
	std::vector<VGUI_EventConsole*> m_Consoles;
	mslist<CInfoWindow*> m_InfoWindows;
	mslist<CInfoWindow*> m_HelpWindows;

	VGUI_VoteInfo* m_VoteInfo;
	RetroHUD::VGUI_Health* m_RetroHealth;
	PrimaryHUD::VGUI_Health* m_Health;
	VGUI_Status* m_Status; //Drigien MAY2008
	VGUI_EventConsole* m_ActiveConsole;

	const int INFOWIN_HELP_DISPLAY_X = XRES(640) - XRES(260);
	const int INFOWIN_HELP_DISPLAY_Y = YRES(300);
	const int EVENTCON_SIZE_X = XRES(230);
	const int EVENTCON_X = XRES(640) - EVENTCON_SIZE_X - XRES(20);
	const int EVENTCON_Y = (YRES(480) - YRES(10));
	const int EVENTCON_SIZE_Y = 0;
	const int SAYTEXTCON_X = XRES(10);
	const int SAYTEXTCON_Y = YRES(180);
	const int SAYTEXTCON_SIZE_X = XRES(300);
};

VGUI_MainPanel* CreateHUDPanel(Panel* pParent) { return new CHUDPanel(pParent); }

CHUDPanel::CHUDPanel(Panel* pParent) : VGUI_MainPanel(0, 0, 0, ScreenWidth, ScreenHeight)
{
	setParent(pParent);
	
	m_HUDElements.push_back(m_RetroHealth = new RetroHUD::VGUI_Health(this));
	m_HUDElements.push_back(m_Health = new PrimaryHUD::VGUI_Health(this));

	if (CVAR_GET_FLOAT("cl_retrohud") > 0)
	{
		m_RetroHealth->setVisible(true);
		m_Health->setVisible(false);
	}else{
		m_RetroHealth->setVisible(false);
		m_Health->setVisible(true);
	}

	//Status Icons
	m_Status = new VGUI_Status(this);
	m_HUDElements.push_back(m_Status); //Drigien MAY2008

	//ID Panel
	m_ID = new VGUI_ID(this, ID_X, ID_Y);

	//Vote Info Panel
	m_HUDElements.push_back(m_VoteInfo = new VGUI_VoteInfo(this));

	//Event Console
	VGUI_EventConsole::prefs_t Prefs;
	Prefs.VisLines = "ms_evthud_size";
	Prefs.MaxLines = "ms_evthud_history";
	Prefs.DecayTime = "ms_evthud_decaytime";
	Prefs.BGTrans = "ms_evthud_bgtrans";
	Prefs.Width = NULL;

	m_Consoles.push_back(m_ActiveConsole = new VGUI_EventConsole(this, EVENTCON_X, EVENTCON_Y, EVENTCON_SIZE_X, EVENTCON_SIZE_Y, Prefs));

	Prefs.VisLines = "ms_txthud_size";
	Prefs.MaxLines = "ms_txthud_history";
	Prefs.DecayTime = "ms_txthud_decaytime";
	Prefs.BGTrans = "ms_txthud_bgtrans";
	Prefs.Width = "ms_txthud_width";

	m_Consoles.push_back(new VGUI_EventConsole(this, SAYTEXTCON_X, SAYTEXTCON_Y, SAYTEXTCON_SIZE_X, EVENTCON_SIZE_Y, Prefs, true, g_FontID));

	//Start Say text panel
	m_StartSayText = new VGUI_SendTextPanel(this, XRES(100), YRES(300), XRES(640) - XRES(100), YRES(16));

	//Debug Text
	m_DebugText = new MSLabel(this, "", 0, YRES(16));

	//Quick Slot Text
	m_HUDElements.push_back(m_QuickSlot = new VGUI_QuickSlot(this));
}

CHUDPanel::~CHUDPanel() 
{
	delete m_ID;
	delete m_StartSayText;
	delete m_DebugText;

	for(int i = 0; i < m_HUDElements.size(); i++)
	{
		delete m_HUDElements[i];
	}

	for(int i = 0; i < m_Consoles.size(); i++)
	{
		delete m_Consoles[i];
	}

	for (int i = 0; i < m_InfoWindows.size(); i++)
	{
		RemoveInfoWindow(m_InfoWindows, 0);
	}

	for (int i = 0; i < m_HelpWindows.size(); i++)
	{
		RemoveInfoWindow(m_HelpWindows, 0);
	}
		
	std::vector<IHUD*>().swap(m_HUDElements);
	std::vector<VGUI_EventConsole*>().swap(m_Consoles);

	m_InfoWindows.clear();
	m_HelpWindows.clear();
}

// Create new Info window
void CHUDPanel::AddInfoWin(msstring_ref Title, msstring_ref Text)
{
	CInfoWindow& NewInfoWin = *new CInfoWindow(Title, Text, INFOWIN_DISPLAY_X, INFOWIN_DISPLAY_Y, this);

	NewInfoWin.m_TimeDisplayed = gpGlobals->time;
	NewInfoWin.m_Duration = INFOWIN_DURATION;
	NewInfoWin.setVisible(false);
	m_InfoWindows.push_back(&NewInfoWin);
}

// Create new Help window

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
	m_HelpWindows.push_back(&NewInfoWin);
}

// Write to the Event Console
void CHUDPanel::PrintEvent(Color color, msstring_ref Text)
{
	m_Consoles[CON_EVENT]->Print(color, Text);
}

// Write to the Event Console
void CHUDPanel::PrintSayText(Color color, msstring_ref Text)
{
	m_Consoles[CON_SAYTEXT]->Print(color, Text);
}

// VGUI HUD Creation
// -----------------

// Update
void CHUDPanel::Think()
{
	//Update ID
	m_ID->Update();

	//Update HUD Elements
	for (int i = 0; i < m_HUDElements.size(); i++)
	{
		msstring d = "HUD Elements loop: ";
		d += i;
		m_HUDElements[i]->Update();
	}

	if ((CVAR_GET_FLOAT("cl_retrohud") > 0) && m_Health->isVisible())
	{
		m_RetroHealth->setVisible(true);
		m_Health->setVisible(false);
	}

	if ((CVAR_GET_FLOAT("cl_retrohud") == 0) && m_RetroHealth->isVisible()) {
		m_RetroHealth->setVisible(false);
		m_Health->setVisible(true);
	}

	UpdateInfoWindows(m_InfoWindows); //Update Info windows
	UpdateInfoWindows(m_HelpWindows); //Update Help windows

	//Update Text Consoles
	for (int i = 0; i < m_Consoles.size(); i++)
	{
		msstring d = "Text Consoles: ";
		d += i;
		m_Consoles[i]->Update();
	}

	//Update Start Say Text
	m_StartSayText->Update();
}

void CHUDPanel::UpdateInfoWindows(mslist<CInfoWindow*>& Windows)
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

void CHUDPanel::RemoveInfoWindow(mslist<CInfoWindow*>& Windows, int idx)
{
	CInfoWindow* pInfoWin = Windows[idx];
	Windows.erase(idx);
	removeChild(pInfoWin);
	pInfoWin->Remove();
}

//-----------------------------------------------------------------------------
// Purpose: Called each time a new level is started.
//-----------------------------------------------------------------------------
void CHUDPanel::Initialize(void)
{
	MSGlobals::CurrentVote.fActive = false;

	//Remove all info windows
	for (int i = 0; i < m_InfoWindows.size(); i++)
		RemoveInfoWindow(m_InfoWindows, 0);

	for (int i = 0; i < m_HelpWindows.size(); i++)
		RemoveInfoWindow(m_HelpWindows, 0);

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
		HelpParts.add(READ_STRING());
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

void HUD_ShowInfoWin(msstring_ref Title, msstring_ref Text)
{
	if (!gViewPort || !gViewPort->m_pHUDPanel || !ShowHUD())
		return;

	((CHUDPanel*)gViewPort->m_pHUDPanel)->AddInfoWin(Title, Text);
}

void HUD_ShowHelpWin(msstring_ref Title, msstring_ref Text)
{
	if (!gViewPort || !gViewPort->m_pHUDPanel || !ShowHUD())
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
	if (!gViewPort || !gViewPort->m_pHUDPanel ||!ShowHUD())
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