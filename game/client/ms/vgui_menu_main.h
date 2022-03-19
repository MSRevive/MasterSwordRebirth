
// Menu Dimensions
#define MAINWIN_SIZE_X XRES(120)
#define MAINWIN_SIZE_Y YRES(170)
#define MAINWIN_X XRES(320) - MAINWIN_SIZE_X / 2
#define MAINWIN_Y YRES(240) - MAINWIN_SIZE_Y / 2

#define BTN_SPACER_X XRES(15)
#define BTN_SPACER_Y YRES(10)
#define BTN_SIZE_X (MAINWIN_SIZE_X - BTN_SPACER_X * 2.0f)
#define BTN_SIZE_Y YRES(12)
#define BTN_X MAINWIN_SIZE_X / 2.0f - BTN_SIZE_X / 2.0f
#define BTN_START_Y YRES(50)

#define MAINMENU_FADETIME 0.5f //Thothie DEC2012_24 - reduce menu fade time

#define MAINLABEL_TOP_Y YRES(0)

// Creation
class VGUI_MenuMain : public VGUI_MenuBase
{
public:
	class CPanel_Options *m_OptionsPanel;

	VGUI_MenuMain(Panel *pParent) : VGUI_MenuBase(pParent)
	{
		startdbg;

		m_Name = "main";

		Init();

		struct btninfo_t
		{
			const char *Name;
			int Width;
			int OptionScreen;
		} static g_ButtonNames[] =
			{
				"#PARTY", XRES(28), OPTIONSC_PARTY,
				"#VOTEKICK", XRES(30), OPTIONSC_VOTEKICK,
				"#VOTETIME", XRES(23), OPTIONSC_VOTETIME,
				"#CANCEL", XRES(33), 0};

		m_ButtonY = BTN_START_Y;
		for (int i = 0; i < ARRAYSIZE(g_ButtonNames); i++)
			MSButton *pButton = AddButton(Localized(g_ButtonNames[i].Name), g_ButtonNames[i].Width, msvariant(g_ButtonNames[i].OptionScreen));

		m_OptionsPanel = new CPanel_Options(this);
		m_OptionsPanel->setVisible(false);

		enddbg;
	}

	// Update
	void Update()
	{
		m_Buttons[OPT_VOTEKICK]->setEnabled(false);
		m_Buttons[OPT_VOTETIME]->setEnabled(false);
		for (int i = 0; i < vote_t::VotesTypesAllowed.size(); i++)
		{
			if (vote_t::VotesTypesAllowed[i] == "kick")
				m_Buttons[OPT_VOTEKICK]->setEnabled(true);
			if (vote_t::VotesTypesAllowed[i] == "advtime")
				m_Buttons[OPT_VOTETIME]->setEnabled(true);
		}
		//m_pButton[OPT_VOTE]->SetBGColorRGB( BtnColor );
	}

	void Select(int BtnIdx, msvariant &Data)
	{
		m_pMainPanel->setVisible(false);

		if (BtnIdx != OPT_CANCEL)
			m_OptionsPanel->Open((option_e)(int)Data);
		else
			//Clicked Cancel
			VGUI::HideMenu(this);
	}

	void Reset(void)
	{
		VGUI_MenuBase::Reset();
		m_OptionsPanel->setVisible(false);
	}

	// Update the menu before opening it
	//void Open( void )
	//{
	//	foreach( i, MAX_MAINBUTTONS ) m_Buttons[i]->setArmed( false );

	//	Update( );
	//	CMenuPanel::Open( );
	//	m_OpenTime = gpGlobals->time;
	//}
};

VGUI_MainPanel *CreateHUD_MenuMain(Panel *pParent) { return new VGUI_MenuMain(pParent); }
