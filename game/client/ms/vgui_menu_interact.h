// Menu Dimensions
#define BTN_SPACER_X XRES(15)
#define BTN_SPACER_Y YRES(10)
#define BTN_SIZE_X (MAINWIN_SIZE_X - BTN_SPACER_X * 2.0f)
#define BTN_SIZE_Y YRES(12)
#define BTN_X MAINWIN_SIZE_X / 2.0f - BTN_SIZE_X / 2.0f
#define BTN_START_Y YRES(50)

#define MAINMENU_FADETIME 0.5f //Thothie DEC2012_24 - reduce menu fade time

#define MAINLABEL_TOP_Y YRES(0)

#include "hudid.h"

#define INTERACT_MAX_BUTTONS 10
#define INTERACT_MENU_NAME "interact"

enum interactbtn_e
{
	INTERACT_TRADE,
	INTERACT_FORGIVE,
	INTERACT_CANCEL
};

extern void IN_UseUp();
extern float g_fMenuLastClosed;

void ShowWeaponDesc(CGenericItem *pItem);

// Interact Menu
class VGUI_MenuInteract : public VGUI_MenuBase
{
public:
	int m_LastButton, m_EntIdx;
	mslist<menuoption_t> m_Options;

	VGUI_MenuInteract(Panel *pParent) : VGUI_MenuBase(pParent)
	{
		startdbg;

		m_Name = INTERACT_MENU_NAME;

		Init();

		int w, h;
		m_pMainPanel->getPos(w, h);
		m_pMainPanel->setPos(ScreenWidth - m_pMainPanel->getWide() - XRES(80), h); //Thothie JAN2008a was  XRES(20)

		m_ButtonY = BTN_START_Y;

		for (int i = 0; i < INTERACT_MAX_BUTTONS; i++) //Create all the buttons now.  Fill them in later
			AddButton("", 0, i);

		// MiB NOV2014_25, center the title and separator NpcInteractMenus.rft
		m_AllowKeys = true; // MiB 25NOV_2014 - Disable key-input

		enddbg;
	}

	void Open()
	{
		IN_UseUp();

		m_Title->setFont(g_FontTitle);
		m_Title->setText(Localized("#MENU_INTERACT_TITLE"));
		m_LastButton = 0;
		m_EntIdx = -1;
		m_Options.clearitems();

		for (int i = 0; i < m_Buttons.size(); i++) //Reset all buttons except cance
		{
			m_Buttons[i]->setVisible(false);
			m_Buttons[i]->setEnabled(true);
		}

		SetButton(0, Localized("#CANCEL"), MOT_CALLBACK);

		VGUI_MenuBase::Open();
	}
	bool CanOpen() { return true; }

	void QueryNPC()
	{
		entinfo_t *pEntInfo = gHUD.m_HUDId->GetEntInFrontOfMe(72); //Look 72 units ahead
		if (pEntInfo)
		{
			SetInfo(pEntInfo->Name, pEntInfo->entindex);
			ServerCmd(msstring("getmenuoptions ") + m_EntIdx + "\n");
		}
		else
		{
			ServerCmd("getmenuoptions\n");
		}
	}

	void SetInfo(msstring NPCName, int EntIdx)
	{
		m_EntIdx = EntIdx;

		//Determine if the title is too big.  If so, use the small font for the title
		int textw, texth;
		g_FontTitle->getTextSize(NPCName, textw, texth);
		//Thothie JAN2008a can't figure how title text centers self
		//- so always using small text as it looks less odd uncentered
		/*if( textw >= (m_pMainPanel->getWide()-XRES(5)) )
		{
		m_Title->setFont( g_FontSml );
		m_Title->setText( NPCName );
		}*/

		m_Title->setFont(g_FontSml); //JAN2008a - see above
		m_Title->setText(NPCName);

		// MiB NOV2014_25, center the title and separator NpcInteractMenus.rft [begin]
		int x, y;
		m_Title->getPos(x, y);
		m_Title->setPos(GetCenteredX(m_pMainPanel->getWide(), m_Title->getWide()), y);

		m_TitleSep->getPos(x, y);
		m_TitleSep->setPos(GetCenteredX(m_pMainPanel->getWide(), m_TitleSep->getWide()), y);
		m_TitleSep->setBorder(m_Spacer);
		// MiB NOV2014_25, center the title and separator NpcInteractMenus.rft [end]
	}

	// MiB NOV2014_25, because lazy: NpcInteractMenus.rtf
	int GetCenteredX(int containerWidth, int itemWidth) { return (containerWidth / 2.0) - (itemWidth / 2.0); }

	void AddOption(menuoption_t &MenuOption)
	{
		if (m_LastButton >= INTERACT_MAX_BUTTONS - 1)
			return;

		m_Options.add(MenuOption);

		//MiB Dec2007a
		int w, h;
		m_pMainPanel->getSize(w, h);
		//Thothie JAN2007a - was: m_pMainPanel->setSize(w,YRES(72) + YRES(m_Options.size()*20 ) );
		//- change to attempt to get more width out of menus
		m_pMainPanel->setSize(XRES(200), YRES(72) + YRES((m_Options.size() + 1) * 20)); //Dynamically change the menu size to fit all number of options.
		// 50 (header size)
		// 22 (button size - cancel isn't included in m_Options)
		// 72 (Total displacement)
		//[/MiB]

		SetButton(m_LastButton++, Localized(MenuOption.Title), MenuOption.Type);
		SetButton(m_LastButton, Localized("#CANCEL"), MOT_CALLBACK);
	}
	void SetButton(int idx, msstring_ref Name, menuoptiontype_e Type)
	{
		MSButton &Button = *m_Buttons[idx];
		Button.setText(Name);
		Button.setVisible(true);
		int x, y;
		Button.getPos(x, y);
		Button.setPos((m_pMainPanel->getWide() / 2.0) - (Button.getWide() / 2.0), y);

		//Thothie AUG2013_12 - new green type for completed tally
		if (Type == MOT_GREEN)
		{
			COLOR cTempColor = COLOR(0, 255, 0, 255);
			//Button.SetUnArmedColor(cTempColor);
			Button.SetDisabledColor(cTempColor);
		}
		else
		{
			Button.SetDisabledColor(DisabledColor);
		}

		if (Type == MOT_DISABLED || Type == MOT_GREEN)
			Button.setEnabled(false);
	}

	void Select(int BtnIdx, msvariant &Data)
	{
		IN_UseUp();
		g_fMenuLastClosed = gEngfuncs.GetClientTime();

		bool SendCmd = true;
		if (BtnIdx >= 0 && BtnIdx < (signed)m_Options.size())
		{
			menuoption_t &MenuOption = m_Options[BtnIdx];
			if (MenuOption.Type == MOT_DESC)
			{
				ShowWeaponDesc(player.ActiveItem());
				ServerCmd(msstring("menuoption ") + m_EntIdx + " " + -1 + "\n");
				SendCmd = false;
			}
			else if (MenuOption.Type == MOT_FORGIVE)
			{
				ServerCmd("forgive");
				ServerCmd(msstring("menuoption ") + m_EntIdx + " " + -1 + "\n");
				SendCmd = false;
			}
		}

		if (SendCmd && BtnIdx != m_LastButton)
		{
			ServerCmd(msstring("menuoption ") + m_EntIdx + " " + (int)Data + "\n");
		}

		if (SendCmd && BtnIdx == m_LastButton)
		{
			ServerCmd(msstring("menuoption ") + m_EntIdx + " " + -1 + "\n");
		}

		VGUI::HideMenu(this);
	}
};

VGUI_MainPanel *CreateHUD_MenuInteract(Panel *pParent) { return new VGUI_MenuInteract(pParent); }

void VGUI_ShowMenuInteract()
{
	VGUI::ShowMenu(INTERACT_MENU_NAME);

	VGUI_MainPanel *pPanel = VGUI::FindPanel(INTERACT_MENU_NAME);
	if (!pPanel)
		return;

	VGUI_MenuInteract *pInteractPanel = (VGUI_MenuInteract *)pPanel;

	int EntIdx = READ_LONG();
	msstring NPCName = READ_STRING();

	pInteractPanel->SetInfo(NPCName, EntIdx);
}

void VGUI_AddMenuOption()
{
	menuoption_t MenuOption;

	MenuOption.Access = (menuoptionaccess_e)READ_BYTE();
	MenuOption.Title = READ_STRING();
	MenuOption.Type = (menuoptiontype_e)READ_BYTE();
	MenuOption.Data = READ_STRING();

	VGUI_MainPanel *pPanel = VGUI::FindPanel(INTERACT_MENU_NAME);
	if (!pPanel)
		return;

	VGUI_MenuInteract *pInteractPanel = (VGUI_MenuInteract *)pPanel;
	pInteractPanel->AddOption(MenuOption);
}
