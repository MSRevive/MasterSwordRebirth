//
//  This should only be included by vgui_HUD.cpp
//
//	Info window: Prominent text window that fades in and fades back out
//---------------------------------------------------------------------

class CInfoWindow : public CTransparentPanel
{
public:
#define INFOWIN_INTIAL_SIZE_X 120 //Default size, it's dynamic based on the text
#define INFOWIN_INTIAL_SIZE_Y 100 //Default size, it's dynamic based on the text
#define INFOWIN_SPACER_BORDER 3
#define INFOWIN_SPACER_BELOWTITLE 3
#define INFOWIN_BORDER_SIZE 2
#define INFOWIN_BKTRANS 128
//Effects
#define FADEIN_TIME 1.0f
#define FADEOUT_TIME 1.0f
#define INFOWIN_DURATION 8.0f

//Pos on screen
#define INFOWIN_DISPLAY_X XRES(20)
#define INFOWIN_DISPLAY_Y YRES(50)
#define INFOWIN_DISPLAY_SPACER_Y YRES(4)

	CInfoWindow::CInfoWindow(msstring_ref NewTitle, msstring_ref NewText, int x, int y, Panel *pParent) : CTransparentPanel(0, 0, 0, INFOWIN_INTIAL_SIZE_X, INFOWIN_INTIAL_SIZE_Y)
	{
		setParent(pParent);
		setPos(x, y);
		m_StartY = y;

		CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
		SchemeHandle_t hClassWindowText = pSchemes->getSchemeHandle("Briefing Text");

		Title = new TransLabel("", INFOWIN_SPACER_BORDER, INFOWIN_SPACER_BORDER, this);
		Font *pFont = pSchemes->getFont(hClassWindowText);
		Font *pNewFont = new Font("Courier", pFont->getTall() + 2, 0, 0, 2000, 0, 0, 0, false);
		Title->setFont(pNewFont);
		Title->setFgColor(225, 0, 0, 0);
		Title->setContentAlignment(vgui::Label::a_west);

		Text = new TextPanel("", INFOWIN_SPACER_BORDER, INFOWIN_SPACER_BORDER + Title->getTall() + INFOWIN_SPACER_BELOWTITLE, INFOWIN_INTIAL_SIZE_X - (INFOWIN_SPACER_BORDER * 2), 0);
		Text->setParent(this);
		Text->setFont(pSchemes->getFont(hClassWindowText));
		Text->setBgColor(0, 0, 0, 255);
		Text->setFgColor(192, 192, 192, 0);

		SetTitle(Localized(NewTitle));
		SetText(Localized(NewText));

		m_iTransparency = INFOWIN_BKTRANS;
		//BorderColor = Color(0, 128, 0,0);
		//setBorder( m_Border=new LineBorder( 2, BorderColor ) );
		//setBorder( NULL );
	}
	void SetTitle(msstring_ref NewTitle)
	{
		Title->setText(NewTitle);
		Resize();
	}
	void SetText(msstring_ref NewText)
	{
		Text->setText(NewText);
		int TextX, TextY;
		Text->getTextImage()->getSize(TextX, TextY);
		Text->setSize(TextX, TextY);
		Resize();
	}
	void Resize()
	{
		int TitleX, TitleY;
		Title->getSize(TitleX, TitleY);
		int TextX, TextY;
		Text->getSize(TextX, TextY);

		int x = INFOWIN_SPACER_BORDER + max(TitleX, TextX) + INFOWIN_SPACER_BORDER + INFOWIN_BORDER_SIZE * 2 + 2; //+2 makes up for initial spacing in label text
		int y = INFOWIN_SPACER_BORDER + TitleY + INFOWIN_SPACER_BELOWTITLE + TextY + INFOWIN_SPACER_BORDER + INFOWIN_BORDER_SIZE * 2;

		setSize(x, y);
	}

	void Update(mslist<CInfoWindow *> &Windows, int idx)
	{
		float elapsedtime = gpGlobals->time - m_TimeDisplayed;

		//fade in
		float fadeamt = elapsedtime <= FADEIN_TIME ? (min(elapsedtime, FADEIN_TIME) / FADEIN_TIME) : 1.0f;

		//fade out
		if (elapsedtime > m_Duration - FADEOUT_TIME)
			fadeamt = 1.0f - min(elapsedtime - (m_Duration - FADEOUT_TIME), FADEOUT_TIME) / FADEOUT_TIME;

		int alpha = 255 - (255 * fadeamt);

		Color color;
		Title->getFgColor(color);
		Title->setFgColor(color[0], color[1], color[2], alpha);

		Text->getFgColor(color);
		Text->setFgColor(color[0], color[1], color[2], alpha);

		m_iTransparency = 255 - ((255 - INFOWIN_BKTRANS) * fadeamt);

		int yPos = m_StartY;
		for (int i = 0; i < idx; i++)
			yPos += Windows[i]->getTall() + INFOWIN_DISPLAY_SPACER_Y;

		if (idx > 0)
		{
			//If I'm lower than the first message, check if the first message is disapearing.
			//If so, start moving up before he's done fading
			CInfoWindow &FirstWin = *Windows[0];
			float elapsedtime = gpGlobals->time - FirstWin.m_TimeDisplayed;
			if (elapsedtime > m_Duration - FADEOUT_TIME)
			{
				float moveamt = min(elapsedtime - (m_Duration - FADEOUT_TIME), FADEOUT_TIME) / FADEOUT_TIME;
				yPos -= (FirstWin.getTall() + INFOWIN_DISPLAY_SPACER_Y) * moveamt;
			}
		}

		int x, y;
		getPos(x, y);
		setPos(x, yPos);
		setVisible(true);
	}
	void Remove()
	{
		removeChild(Title);
		removeChild(Text);

		delete Title;
		delete Text;

		delete this;
	}

	TransLabel *Title;
	TextPanel *Text;
	int m_StartY;
	//LineBorder *m_Border;
	//Color BorderColor;

	float m_TimeDisplayed;
	float m_Duration;
};