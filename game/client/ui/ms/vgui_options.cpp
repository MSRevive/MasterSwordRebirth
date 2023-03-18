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
// $Log: vgui_Vote.cpp,v $
// Revision 1.6  2005/01/17 13:16:49  dogg
// Brand new inventory VGUI, revised item system, Magic, Mp3 support and item storage
//
// Revision 1.5  2004/11/07 01:06:28  dogg
// New Event Console
//
// Revision 1.4  2004/10/19 23:14:54  dogg
// BIG update
// Memory leak fixes
//
// Revision 1.3  2004/10/13 20:21:51  dogg
// Big update
// Netcode re-arranged
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

#include "../hud.h"
#include "../cl_util.h"
#include "../parsemsg.h"
#include "../vgui_teamfortressviewport.h"

//Master Sword
#include "../vgui_scorepanel.h"
#include "vgui_options.h"
#include "vgui_menudefsshared.h"
#include "clglobal.h"
#include "vgui_mscontrols.h"
#include "logger.h"
//------------

COLOR Color_Transparent(0, 0, 0, 255), Color_Text_White(255, 255, 255, 0); //Global
static COLOR Color_FadedWhite(255, 255, 255, 64),
	Color_Orange(255, 178, 0, 0), Color_BtnHighlight(255, 0, 0, 0), Color_BG_NameHightlight(100, 100, 100, 150),
	Color_Text_Desc(255, 255, 255, 128);

class ListButton : public MSButton
{
protected:
	class VGUI_Options *m_pPanel;
	int m_idx;

public:
	msstring m_PartyName;

	ListButton(VGUI_Options *pPanel, int idx, int x, int y, int wide, int tall);
	void Clicked();
};

class VoteButton : public MSButton
{
public:
	VoteButton(Panel *pParent, msstring_ref Text, int x, int y, int wide, int tall) : MSButton(pParent, Text, x, y, wide, tall, Color_BtnHighlight, Color_FadedWhite) {}
};

class CAction_SelectList : public ActionSignal
{
protected:
	ListButton *m_pListButton;

public:
	CAction_SelectList(ListButton *pListButton)
	{
		m_pListButton = pListButton;
	}

	virtual void actionPerformed(Panel *panel)
	{
		m_pListButton->Clicked();
	}
};

class CAction_OptionBegin : public ActionSignal
{
public:
	VGUI_Options *m_Panel;
	enum actoption_e
	{
		opt_vote,
		opt_joinparty,
		opt_createparty
	} m_OptionType;

	CAction_OptionBegin(VGUI_Options *pPanel, actoption_e OptionType)
	{
		m_Panel = pPanel;
		m_OptionType = OptionType;
	}
	void actionPerformed(Panel *panel);
};

class CAction_CastVote : public ActionSignal
{
public:
	int m_Type;

	CAction_CastVote(int Type) { m_Type = Type; }
	virtual void actionPerformed(Panel *panel)
	{
		if (m_Type == 0)
			ClientCmd("vote 1\n");
		else if (m_Type == 1)
			ClientCmd("leaveparty\n");
		ShowVGUIMenu(0);
	}
};

// Menu Dimensions
#define MAINWIN_SIZE_X XRES(320)
#define MAINWIN_SIZE_Y YRES(240)
#define MAINWIN_X (XRES(320) - MAINWIN_SIZE_X / 2)
#define MAINWIN_Y (YRES(240) - MAINWIN_SIZE_Y / 2)
#define MAINWIN_SPACER_X XRES(15)
#define MAINWIN_SPACER_Y YRES(8)

#define KICKBTN_SIZE_X XRES(45)
#define KICKBTN_SIZE_Y YRES(20)
#define KICKBTN_X (MAINWIN_SIZE_X - MAINWIN_SPACER_X - KICKBTN_SIZE_X)
#define KICKBTN_Y (MAINWIN_SIZE_Y - MAINWIN_SPACER_Y - KICKBTN_SIZE_Y)

#define CANCELBTN_X (MAINWIN_SPACER_X)
#define CANCELBTN_SIZE_X XRES(45)

#define SCROLL_SIZE_X (MAINWIN_SIZE_X - MAINWIN_SPACER_X * 2.0f)
#define SCROLL_SIZE_Y YRES(148)
#define SCROLL_X (MAINWIN_SIZE_X / 2.0f - SCROLL_SIZE_X / 2.0f)
#define SCROLL_Y MAINWIN_SIZE_Y - MAINWIN_SPACER_Y - KICKBTN_SIZE_Y - YRES(2) - SCROLL_SIZE_Y

#define PLAYERBTN_SPACER_X XRES(5)
#define PLAYERBTN_SPACER_Y YRES(1)
#define PLAYERBTN_SIZE_X (SCROLL_SIZE_X - PLAYERBTN_SPACER_X * 2.0f)
#define PLAYERBTN_SIZE_Y YRES(12)
#define PLAYERBTN_X (SCROLL_SIZE_X / 2.0f - PLAYERBTN_SIZE_X / 2.0f)
#define PLAYERBTN_START_Y YRES(0)

#define MAINLABEL_TOP_Y YRES(0)

/*
	Template Panel for options screens
*/
class VGUI_Options : public CTransparentPanel
{
public:
	ListButton *m_pListButton[MAX_VOTE_PLAYERS];
	CTFScrollPanel *m_pScrollPanel;
	MSButton *m_pBegin, *m_pCancel;
	int m_Target;
	MSLabel *m_Title, *m_Desc;
	CAction_OptionBegin *m_Action;
	MSLabel *m_NoResults;
	msstring m_VoteType;

	VGUI_Options(Panel *pParent) : CTransparentPanel(128, 0, 0, MAINWIN_SIZE_X, MAINWIN_SIZE_Y)
	{
		setParent(pParent);
		setVisible(false);

		CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
		SchemeHandle_t hClassWindowText = pSchemes->getSchemeHandle("Briefing Text");

		m_Title = new MSLabel(this, Localized("#VOTE_KICK"), MAINWIN_SPACER_X, MAINWIN_SPACER_Y, XRES(120), YRES(20));
		m_Title->setContentAlignment(vgui::Label::a_west);
		m_Title->SetFGColorRGB(Color_Text_White);

		m_Desc = new MSLabel(this, Localized("#VOTE_KICK_DESC"), MAINWIN_SPACER_X + XRES(5), MAINWIN_SPACER_Y + YRES(22));
		m_Desc->setContentAlignment(vgui::Label::a_west);
		m_Desc->SetFGColorRGB(Color_Text_Desc);
		//m_Desc->setFont( pSchemes->getFont(hClassWindowText) );

		CTransparentPanel *pSpacer = new CTransparentPanel(0, MAINWIN_SPACER_X, MAINWIN_SPACER_Y + YRES(42), MAINWIN_SIZE_X - MAINWIN_SPACER_X * 2.0f, YRES(3));
		pSpacer->setBorder(new LineBorder(2, Color(0, 128, 0, 128)));
		pSpacer->setParent(this);

		// Create the Scroll panel
		m_pScrollPanel = new CTFScrollPanel(SCROLL_X, SCROLL_Y, SCROLL_SIZE_X, SCROLL_SIZE_Y);
		m_pScrollPanel->setParent(this);
		m_pScrollPanel->setScrollBarVisible(false, false);
		m_pScrollPanel->setScrollBarAutoVisible(true, true);
		//m_pScrollPanel->setBorder( new LineBorder( Color(255,255,255,0) ) );

		int yPos = PLAYERBTN_START_Y;
		for (int i = 0; i < MAX_VOTE_PLAYERS; i++)
		{
			ListButton *pButton = m_pListButton[i] = new ListButton(this, i, PLAYERBTN_X, yPos, PLAYERBTN_SIZE_X, PLAYERBTN_SIZE_Y);
			pButton->setFont(pSchemes->getFont(hClassWindowText));
			pButton->SetArmedColor(Color_FadedWhite);
			pButton->SetArmedColor(Color_Orange);
			yPos += PLAYERBTN_SIZE_Y + PLAYERBTN_SPACER_Y;
		}

		m_NoResults = new MSLabel(m_pScrollPanel->getClient(), Localized("#VOTE_NONE"), 0, 0);
		m_NoResults->SetFGColorRGB(Color_Orange);

		m_pBegin = new VoteButton(this, Localized("#OK"), KICKBTN_X, KICKBTN_Y, KICKBTN_SIZE_X, KICKBTN_SIZE_Y);
		m_pBegin->addActionSignal(m_Action = new CAction_OptionBegin(this, CAction_OptionBegin::opt_vote));

		m_pCancel = new VoteButton(this, Localized("#CANCEL"), CANCELBTN_X, KICKBTN_Y, CANCELBTN_SIZE_X, KICKBTN_SIZE_Y);
		m_pCancel->addActionSignal(new CMenuHandler_TextWindow(HIDE_TEXTWINDOW));
	}

	virtual void Open()
	{
		m_Target = -1;
		setVisible(true);
		for (int i = 0; i < MAX_VOTE_PLAYERS; i++)
		{
			m_pListButton[i]->setArmed(false);
			m_pListButton[i]->SetBGColorRGB(Color_Transparent);
		}
		m_pBegin->setArmed(false);
		m_pCancel->setArmed(false);
		Update();
		m_pScrollPanel->setScrollValue(0, 0);
		m_pScrollPanel->validate();
	}

	virtual void Update() { UpdateBeginBtn(); }
	virtual void UpdateBeginBtn()
	{
		if (m_Target < 0)
		{
			m_pBegin->setEnabled(false);
			m_pBegin->setBgColor(0, 0, 0, 255);
			m_pBegin->setFgColor(255, 255, 255, 200);
		}
	}

	virtual void ButtonClicked(int idx)
	{
		m_Target = idx;
		for (int i = 0; i < MAX_VOTE_PLAYERS; i++)
			m_pListButton[i]->SetBgColor(Color_Transparent);
		m_pListButton[m_Target]->SetBgColor(Color_BG_NameHightlight);
		m_pBegin->setEnabled(true);
	}
};

/*
	Panel to initiate a vote
*/
class CInitVotePanel : public VGUI_Options
{
public:
	CInitVotePanel(Panel *pParent) : VGUI_Options(pParent) {}

	virtual void Open(msstring_ref VoteType)
	{
		m_VoteType = VoteType;

		msstring VoteTypeCaps = m_VoteType;
		strupr(VoteTypeCaps);

		msstring TitleText = msstring("#VOTE_") + VoteTypeCaps + "_TITLE",
				 DescText = msstring("#VOTE_") + VoteTypeCaps + "_DESC",
				 ConfirmText = msstring("#VOTE_") + VoteTypeCaps + "_CONFIRM";

		m_Title->setText(Localized(TitleText));
		m_Desc->setText(Localized(DescText));
		m_pBegin->setText(Localized(ConfirmText));
		VGUI_Options::Open();
	}

	virtual void Update()
	{
		int UsedBtns = 0;
		m_NoResults->setVisible(false);

		if (m_VoteType == "kick")
		{
			for (int i = 0; i < MAX_VOTE_PLAYERS; i++)
				if (g_PlayerInfoList[i].name &&
					i != MSCLGlobals::GetLocalPlayerIndex())
				{
					m_pListButton[UsedBtns]->setParent(m_pScrollPanel->getClient());
					m_pListButton[UsedBtns]->setText(g_PlayerInfoList[i].name);
					UsedBtns++;
				}

			m_NoResults->setVisible(!UsedBtns);
		}
		else if (m_VoteType == "advtime")
		{
			for (int i = 0; i < 3; i++)
			{
				m_pListButton[i]->setParent(m_pScrollPanel->getClient());
				m_pListButton[i]->setText(Localized(msstring("#VOTE_ADVTIME_OPT") + i));
				UsedBtns++;
			}
		}

		for (; UsedBtns < MAX_VOTE_PLAYERS; UsedBtns++) //Intentional ommission - This unsets the parent of all the
			m_pListButton[UsedBtns]->setParent(NULL);	//buttons beyond the valid ones

		UpdateBeginBtn();
	}
	virtual void UpdateBeginBtn()
	{
		if (m_Target < 0)
		{
			m_pBegin->setEnabled(false);
			m_pBegin->setBgColor(0, 0, 0, 255);
			m_pBegin->setFgColor(255, 255, 255, 200);
		}
	}

	virtual void ButtonClicked(int idx)
	{
		m_Target = idx;
		for (int i = 0; i < MAX_VOTE_PLAYERS; i++)
			m_pListButton[i]->SetBgColor(Color_Transparent);
		m_pListButton[m_Target]->SetBgColor(Color_BG_NameHightlight);
		m_pBegin->setEnabled(true);
	}
};

ListButton::ListButton(VGUI_Options *pPanel, int idx, int x, int y, int wide, int tall) : MSButton(NULL, "", x, y, wide, tall)
{
	m_pPanel = pPanel;
	m_idx = idx;
	setContentAlignment(vgui::Label::a_west);
	addActionSignal(new CAction_SelectList(this));
}
void ListButton::Clicked()
{
	m_pPanel->ButtonClicked(m_idx);
}

/*
	Panel to cast a vote
*/
class CCastVotePanel : public CTransparentPanel
{
public:
	VoteButton *m_pNoBtn, *m_pYesBtn;
	MSLabel *m_Title, *m_Desc;
	CAction_CastVote *m_Action;

	CCastVotePanel(Panel *myParent) : CTransparentPanel(128, 0, 0, MAINWIN_SIZE_X, MAINWIN_SIZE_Y)
	{
		setParent(myParent);
		setVisible(false);

		m_Title = new MSLabel(this, Localized("#VOTE_CAST"), MAINWIN_SPACER_X, MAINWIN_SPACER_Y, XRES(120), YRES(20));
		m_Title->setContentAlignment(vgui::Label::a_west);
		m_Title->setFgColor(255, 255, 255, 0);

		m_Desc = new MSLabel(this, "", MAINWIN_SPACER_X + XRES(5), MAINWIN_SPACER_Y + YRES(24), XRES(230), YRES(23));
		m_Desc->setContentAlignment(vgui::Label::a_west);
		m_Desc->setFgColor(255, 255, 255, 128);

		CTransparentPanel *pSpacer = new CTransparentPanel(0, MAINWIN_SPACER_X, MAINWIN_SPACER_Y + YRES(48), MAINWIN_SIZE_X - MAINWIN_SPACER_X * 2.0f, YRES(3));
		pSpacer->setBorder(new LineBorder(2, Color(0, 128, 0, 128)));
		pSpacer->setParent(this);

#define NOBTN_SIZE_X XRES(30)

		m_pNoBtn = new VoteButton(this, Localized("#NO"), CANCELBTN_X, KICKBTN_Y, NOBTN_SIZE_X, KICKBTN_SIZE_Y);
		m_pNoBtn->addActionSignal(new CMenuHandler_TextWindow(HIDE_TEXTWINDOW));

#define YESBTN_SIZE_X XRES(30)

		m_pYesBtn = new VoteButton(this, Localized("#YES"), KICKBTN_X, KICKBTN_Y, YESBTN_SIZE_X, KICKBTN_SIZE_Y);
		m_pYesBtn->addActionSignal(m_Action = new CAction_CastVote(0));
	}

	void Open()
	{
		setVisible(true);
		msstring sVoteDesc = MSGlobals::CurrentVote.Desc + Localized("#VOTE_CAST_DESC");

		m_Desc->setText(sVoteDesc.c_str());
		m_pNoBtn->setArmed(false);
		m_pYesBtn->setArmed(false);
	}
};

//Party panel
class CPartyPanel : public VGUI_Options
{
public:
	VoteButton *m_CreateParty;

	CPartyPanel(Panel *myParent) : VGUI_Options(myParent)
	{
		m_Title->setText(Localized("#PARTY_JOIN"));
		m_Desc->setText(Localized("#PARTY_JOIN_DESC"));

#define PTY_JOINBTN_SIZE_X XRES(82)
#define PTY_JOINBTN_X (MAINWIN_SIZE_X - MAINWIN_SPACER_X - PTY_JOINBTN_SIZE_X)

		m_pBegin->setPos(PTY_JOINBTN_X, KICKBTN_Y);
		m_pBegin->setSize(PTY_JOINBTN_SIZE_X, KICKBTN_SIZE_Y);
		m_pBegin->setText(Localized("#PARTY_JOIN_BTN"));
		m_Action->m_OptionType = CAction_OptionBegin::opt_joinparty;

#define PTY_CREATEBTN_SIZE_X XRES(110)

		m_CreateParty = new VoteButton(this, Localized("#PARTY_CREATE"), CANCELBTN_X + CANCELBTN_SIZE_X + XRES(25), KICKBTN_Y, PTY_CREATEBTN_SIZE_X, KICKBTN_SIZE_Y);
		m_CreateParty->addActionSignal(new CAction_OptionBegin(this, CAction_OptionBegin::opt_createparty));

		m_NoResults->setText(Localized("#PARTY_NONE"));

		m_pScrollPanel->validate();
	}

	void Open()
	{
		m_CreateParty->setArmed(false);
		VGUI_Options::Open();
	}

	void Update()
	{
		int r = 0;
		for (int i = 0; i < MAX_VOTE_PLAYERS; i++)
			if (g_PlayerExtraInfo[i].teamname[0] /*&&
				i != GetLocalPlayerIndex()*/
			)
			{
				m_pListButton[r]->setParent(m_pScrollPanel->getClient());
				m_pListButton[r]->setText(g_PlayerExtraInfo[i].teamname);
				m_pListButton[r]->m_PartyName = g_PlayerExtraInfo[i].teamname;
				r++;
			}

		m_NoResults->setVisible(!r);

		for (; r < MAX_VOTE_PLAYERS; r++)	   //Intentional ommission - This unsets the parent of all the
			m_pListButton[r]->setParent(NULL); //buttons beyond the valid ones

		UpdateBeginBtn();
	}
};
class CPartyLeavePanel : public CCastVotePanel
{
public:
	CPartyLeavePanel(Panel *myParent) : CCastVotePanel(myParent)
	{
		m_Title->setText(Localized("#PARTY_LEAVE"));
		m_Action->m_Type = 1;
	}

	void Open()
	{
		setVisible(true);
		msstring Line1 = Localized("#PARTY_EXISTS1"); //Separate this, because Localized uses the same static data
		m_Desc->setText(Line1 + " " + player.TeamID() + Localized("#PARTY_EXISTS2"));
	}
};

// Options panel Creation
CPanel_Options::CPanel_Options(Panel *myParent) : CTransparentPanel(200, MAINWIN_X, MAINWIN_Y, MAINWIN_SIZE_X, MAINWIN_SIZE_Y)
{
	MSGlobals::CurrentVote.fActive = false;

	setParent(myParent);

	LineBorder *pBorder = new LineBorder(3, Color(0, 128, 0, 0));
	m_pInitVote = new CInitVotePanel(this);
	m_pInitVote->setBorder(pBorder);

	m_pCastVote = new CCastVotePanel(this);
	m_pCastVote->setBorder(pBorder);

	m_pPartyPanel = new CPartyPanel(this);
	m_pPartyPanel->setBorder(pBorder);

	m_pPartyLeavePanel = new CPartyLeavePanel(this);
	m_pPartyLeavePanel->setBorder(pBorder);

	m_OpenScreen = OPTIONSC_VOTEKICK;
}

// Reset - Called when parant window (main menu) is opened
void CPanel_Options::Reset()
{
	m_pInitVote->setVisible(false);
	m_pCastVote->setVisible(false);
	m_pPartyPanel->setVisible(false);
	m_pPartyLeavePanel->setVisible(false);
}

//======================================
// Key inputs for the Class Menu
bool CPanel_Options::SlotInput(int iSlot)
{
	return false;
}

//Open the specified options menu

void CPanel_Options::Open(option_e OptionScreen)
{
	startdbg;

	dbg("Call Reset");
	Reset();

	setVisible(true);

	dbg("Open option panel");
	m_OpenScreen = OptionScreen;
	switch (m_OpenScreen)
	{
	case OPTIONSC_VOTEKICK:
	case OPTIONSC_VOTETIME:
		if (!MSGlobals::CurrentVote.fActive)
			m_pInitVote->Open((m_OpenScreen == OPTIONSC_VOTEKICK) ? "kick" : "advtime");
		else
			m_pCastVote->Open();
		break;
	case OPTIONSC_PARTY:
		if (!player.TeamID()[0])
			m_pPartyPanel->Open();
		else
			m_pPartyLeavePanel->Open();
		break;
	}

	enddbg;
}

//-----------------------------------------------------------------------------
// Purpose: Called each time a new level is started.
//-----------------------------------------------------------------------------
void CPanel_Options::Initialize(void)
{
	setVisible(false);
	m_pInitVote->m_pScrollPanel->setScrollValue(0, 0);
}

void CAction_OptionBegin::actionPerformed(Panel *panel)
{
	char cSendStr[128];
	switch (m_OptionType)
	{
	case opt_vote:
		 _snprintf(cSendStr, sizeof(cSendStr),  "startvote %s %i\n",  m_Panel->m_VoteType.c_str(),  m_Panel->m_Target );
		break;
	case opt_joinparty:
		 _snprintf(cSendStr, sizeof(cSendStr),  "joinparty \"%s\"\n",  m_Panel->m_pListButton[m_Panel->m_Target]->m_PartyName.c_str() );
		break;
	case opt_createparty:
		 strncpy(cSendStr,  "joinparty ???\n", sizeof(cSendStr) );
		break;
	}

	ClientCmd(cSendStr);
	ShowVGUIMenu(0);
}

int __MsgFunc_Vote(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	byte VoteActive = READ_BYTE();

	switch (VoteActive)
	{
	case 0: //End the vote
	{
		MSGlobals::CurrentVote.fActive = false;
		break;
	}
	case 1: //Start the vote
	{
		MSGlobals::CurrentVote.fActive = true;
		msstring VoteInfo = READ_STRING();
		msstringlist Params;
		TokenizeString(VoteInfo, Params);

		if (Params.size() >= 1)
			MSGlobals::CurrentVote.Type = Params[0];
		if (Params.size() >= 2)
			MSGlobals::CurrentVote.SourcePlayer = atoi(Params[1]);

		//msstring VoteTypeCaps = m_VoteType;
		//strupr( VoteTypeCaps );
		MSGlobals::CurrentVote.Title = Localized(msstring("#VOTE_") + MSGlobals::CurrentVote.Type + "_INPROGRESS_TITLE");
		char cTargetPlayerName[64], cSourcePlayerName[64];
		if (g_PlayerInfoList[MSGlobals::CurrentVote.SourcePlayer].name)
			 strncpy(cSourcePlayerName,  g_PlayerInfoList[MSGlobals::CurrentVote.SourcePlayer].name, sizeof(cSourcePlayerName) );
		else
			 strncpy(cSourcePlayerName,  "<unknown>", sizeof(cSourcePlayerName) );

		if (MSGlobals::CurrentVote.Type == "kick" && Params.size() >= 3)
		{
			MSGlobals::CurrentVote.TargetPlayer = atoi(Params[2]);
			if (g_PlayerInfoList[MSGlobals::CurrentVote.TargetPlayer].name)
				 strncpy(cTargetPlayerName,  g_PlayerInfoList[MSGlobals::CurrentVote.TargetPlayer].name, sizeof(cTargetPlayerName) );
			else
				 strncpy(cTargetPlayerName,  "<unknown>", sizeof(cTargetPlayerName) );

			_snprintf(MSGlobals::CurrentVote.Desc, MSSTRING_SIZE, Localized(msstring("#VOTE_") + MSGlobals::CurrentVote.Type + "_INPROGRESS_DESC"), cSourcePlayerName, cTargetPlayerName);
		}
		else if (MSGlobals::CurrentVote.Type == "advtime" && Params.size() >= 3)
		{
			msstring FormatStr = Localized(msstring("#VOTE_") + MSGlobals::CurrentVote.Type + "_INPROGRESS_DESC" + Params[2]);

			_snprintf(MSGlobals::CurrentVote.Desc, MSSTRING_SIZE, FormatStr, cSourcePlayerName);
		}

		if (MSGlobals::GameScript)
			MSGlobals::GameScript->CallScriptEvent("game_vote_started");

		player.PlaySound(CHAN_AUTO, "misc/vote.wav", 1.0, true);
		break;
	}
	case 2: //Update the vote tally
	{
		MSGlobals::CurrentVote.VoteTally = READ_LONG();
		break;
	}
	}

	return 1;
}
