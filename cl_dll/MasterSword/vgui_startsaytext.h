//
//  This should only be included by vgui_HUD.cpp
//

class VGUI_SendTextPanel : public CTransparentPanel
{
public:
	saytext_e m_Type;
	CImageDelayed m_Image;
	VGUI_TextPanel *m_TextPanel;

#define IMG_SPACER XRES(1)
#define IMG_SIZE 16
#define TEXT_START IMG_SIZE + IMG_SPACER + XRES(2)

	VGUI_SendTextPanel(Panel *Parent, int x, int y, int w, int h) : CTransparentPanel(128, x, y, w, h)
	{
		setParent(Parent);
		setVisible(false);

		m_Image.setParent(this);
		m_Image.setPos(IMG_SPACER, (h - IMG_SIZE) / 2);
		m_Image.setFgColor(255, 255, 255, 255);

		m_TextPanel = new VGUI_TextPanel(this, TEXT_START, 0, XRES(300) - TEXT_START - XRES(3), h);
		m_TextPanel->m_iTransparency = 0;
		m_TextPanel->setVisible(isVisible());
		m_TextPanel->m_MaxLetters = 120;
	}

	void Open(int Type)
	{
		m_Type = (saytext_e)Type;
		msstring_ref ImageName = "hud_shout";
		if (m_Type == SAYTEXT_LOCAL)
			ImageName = "hud_talk";
		else if (m_Type == SAYTEXT_PARTY)
			ImageName = "hud_party";

		m_Image.LoadImg(ImageName, false, false);
		setVisible(true);
		m_TextPanel->Open();
		Resize();
	}

	void Update() { m_TextPanel->Update(); }
	void KeyInput(int down, int keynum, const char *pszCurrentBinding)
	{
		if (keynum == '\r')
		{
			setVisible(false);
			ServerCmd(UTIL_VarArgs("say_text %i %s", m_Type, m_TextPanel->m_Message.c_str()));
			return;
		}
		//AUG2013_18 - can't accept % key
		//stupid to do it here, doing it on the recive end (VGUI_TextPanel::KeyInput in vgui_MSControls.cpp)
		/*
		if( keynum == 53 && down == 1 )
		{
			return; 
		}
		else
		{
			m_TextPanel->KeyInput( down, keynum, pszCurrentBinding );
			Resize( );
		}
		*/
		m_TextPanel->KeyInput(down, keynum, pszCurrentBinding);
		Resize();
	}
	void Resize()
	{
		setSize(TEXT_START + m_TextPanel->getWide() + XRES(3), getTall());
	}
};