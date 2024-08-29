//
//  This should only be included by vgui_HUD.cpp
//

static COLOR Color_VoteTitle(255, 0, 0, 20);

class VGUI_VoteInfo : public Panel, public IHUD_Interface
{
public:
	CTransparentPanel *m_pVotePanel;
	Label *m_pVoteTitle,
		*m_pVoteDesc;
	TextPanel *m_YesVotes;

	VGUI_VoteInfo(Panel *pParent) : Panel(0, 0, ScreenWidth, ScreenHeight)
	{
		setParent(pParent);
		setBgColor(0, 0, 0, 255);

#define VOTE_SPACER_X XRES(5)
#define VOTE_SPACER_Y XRES(2)
#define VOTE_SIZE_X XRES(165)
#define VOTE_SIZE_Y YRES(480)
#define VOTE_START_X (XRES(640) - XRES(7) - VOTE_SIZE_X)
#define VOTE_START_Y YRES(7)
#define VOTE_TITLE_SIZE_Y YRES(20)
#define VOTE_DESC_Y VOTE_SPACER_Y + VOTE_TITLE_SIZE_Y + YRES(9)
#define VOTE_DESC_SIZE_Y YRES(12)
#define VOTE_INFAVOR_Y VOTE_DESC_Y + VOTE_DESC_SIZE_Y + YRES(9)

		m_pVotePanel = new CTransparentPanel(190, VOTE_START_X, VOTE_START_Y, VOTE_SIZE_X, VOTE_SIZE_Y);
		m_pVotePanel->setParent(pParent);
		m_pVotePanel->setVisible(false);

		m_pVoteTitle = new MSLabel(m_pVotePanel, "", 0, VOTE_SPACER_Y, VOTE_SIZE_X, VOTE_TITLE_SIZE_Y);
		m_pVoteTitle->setFont(g_FontTitle);
		m_pVoteTitle->setContentAlignment(vgui::Label::a_center);
		m_pVoteTitle->SetFGColorRGB(Color_VoteTitle);
		m_pVoteTitle->setText(Localized("#VOTE_HUD_TITLE"));

		m_pVoteDesc = new MSLabel(m_pVotePanel, "", VOTE_SPACER_X, VOTE_DESC_Y, VOTE_SIZE_X, VOTE_DESC_SIZE_Y);
		m_pVoteDesc->setContentAlignment(vgui::Label::a_west);
		m_pVoteDesc->setFgColor(200, 200, 255, 0);

		m_YesVotes = new TextPanel("", VOTE_SPACER_X, VOTE_INFAVOR_Y, VOTE_SIZE_X, YRES(480));
		m_YesVotes->setParent(m_pVotePanel);
		m_YesVotes->setBgColor(0, 0, 0, 255);
		m_YesVotes->setFgColor(255, 255, 255, 40);
	}

	void Update()
	{
		if (MSGlobals::CurrentVote.fActive)
		{
			m_pVotePanel->setVisible(true);
			m_pVoteTitle->setText(MSGlobals::CurrentVote.Title);
			m_pVoteDesc->setText(MSGlobals::CurrentVote.Desc);

			char cVotes[128];
			 strncpy(cVotes,  "Those in favor:\n\n", sizeof(cVotes) );
			for (int i = 1; i <= 32; i++)
			{
				if (g_PlayerInfoList[i].name && PlayerVotedYes(i))
				{
					strncat(cVotes, "   ", 3);
					strncat(cVotes, g_PlayerInfoList[i].name, MAX_PLAYER_NAME_LENGTH);
					strcat(cVotes, "\n");
				}
			}
			m_YesVotes->setText(cVotes);
		}
		else
			m_pVotePanel->setVisible(false);
	}
};
