//Drigien MAY2008
class VGUI_Status;
class VGUI_StatusIcon;

static VGUI_Status *StatusIcons;
static COLOR DurColor(0, 255, 0, 128);

#define ICON_W 64	 //Width
#define ICON_H 75	 //Height
#define ICONIMG_W 64 //Width
#define ICONIMG_H 64 //Height

//FN Sprite
#define FN_SPRITE "FN2"		  //Sprite
#define FN_W XRES(64 * 0.625) //Width
#define FN_H YRES(64 * 0.625) //Height

enum HUD_IDS
{
	FN_DW = -3,
	REMOVE_IMG,
	REMOVE_STATUS,
	REMOVE_ALL,
	ADD_STATUS,
	ADD_IMG,
	FN_UP,
};

class VGUI_StatusIcon : public Panel
{
public:
	CImageDelayed m_Img; //Icon

	CStatusBar *m_pBar; //Duration Bar
	char m_Name[32];	//ID of Img
	float m_Time;		//Start Time
	float m_Dur;		//Duration

	VGUI_StatusIcon(Panel *pParent, const char *Icon, const char *Name,
					float Dur, int x, int y, bool bSprite = false) : Panel(x, y, ICON_W, ICON_H)
	{
		if (EngineFunc::CVAR_GetFloat("ms_status_icons") != 0)
		{
			setParent(pParent);
			setBgColor(0, 0, 0, 255);

			 strncpy(m_Name,  Name, sizeof(m_Name) );
			m_Time = gpGlobals->time;
			m_Dur = Dur;

			m_Img.setParent(this);
			//Thothie JAN2013_07 - note - I HAVE NO IDEA why this wont load TGA's
			//gets here, pSprite=True, TGA is valid and loads as an image with addimgicon
			//but won't work with this status icon thing :(
			m_Img.LoadImg(Icon, bSprite, false);
			m_Img.setFgColor(255, 255, 255, 255);
			m_Img.setSize(ICONIMG_W, ICONIMG_H);

			m_pBar = new CStatusBar(this, 0, ICONIMG_H, ICON_W, ICON_H - ICONIMG_H);
			m_pBar->m_fBorder = true;
			m_pBar->Set(100.0);
			m_pBar->setVisible(true);
		}
	}

	void Update(void)
	{
		m_Img.setVisible(true);
		m_pBar->setVisible(true);

		float flPercent = 1.0 - (gpGlobals->time - m_Time) / m_Dur;
		m_pBar->Set(flPercent * 100.0f);
		m_pBar->SetFGColorRGB(DurColor);

		/*
		// Use Different Colors for % left
		if( flPercent < 0.15 ) 
			m_pBar->SetFGColorRGB( LowColor ); 
		else if( flPercent <= 0.85f ) 
			m_pBar->SetFGColorRGB( MedColor ); 
		else 
			m_pBar->SetFGColorRGB( HighColor ); 
		*/
	}

	bool IsActive(void)
	{
		return gpGlobals->time < (m_Time + m_Dur);
	}

	void Kill(void)
	{
		m_Time = 0;
		m_Dur = 0;
		m_Img.setFgColor(0, 0, 0, 0);
		m_Img.ClearImg();
		m_pBar->Set(0);
		m_pBar->setVisible(false);
		//NOV2014_11 - maybe remove image from array here?
	}

	~VGUI_StatusIcon()
	{
		Kill();
		delete m_pBar;
		//delete m_Img;
	}
};

class VGUI_ImgIcon : public Panel
{
public:
	VGUI_Image3D m_Img; //Icon
	char m_ImgName[32]; //Name of Img
	char m_Name[32];	//ID of Img
	float m_Time;		//Start Time
	float m_Dur;		//Duration

	VGUI_ImgIcon(Panel *pParent, const char *Img, const char *Name,
				 int x, int y, int w, int h, float Dur = -1.0) : Panel(x, y, w, h)
	{
		setParent(pParent);
		setBgColor(0, 0, 0, 255);

		 strncpy(m_ImgName,  Img, sizeof(m_ImgName) );
		 strncpy(m_Name,  Name, sizeof(m_Name) );
		m_Time = gpGlobals->time;
		m_Dur = Dur;

		m_Img.setParent(this);
		m_Img.LoadImg(Img, true, false);
		m_Img.setFgColor(255, 255, 255, 255);

		//thothie messes JAN2010_29 - sloppy but works
		//moved to side that calls this
		/*
		float fimgsize_w = ScreenWidth * (w*0.01); 
		float fimgsize_h = ScreenHeight * (h*0.01);
		int imgsize_w = (int)fimgsize_w;
		int imgsize_h = (int)fimgsize_h;
		*/

		//Print("got: x %i y %i w %i h %i wf %f hf %f wa %i ha %i", x, y, w, h, fimgsize_w, fimgsize_h, imgsize_w, imgsize_h );

		//m_Img.setSize( imgsize_w, imgsize_h ); //original
		m_Img.setSize(w, h); //original

		//pParent->setPos( x, y ); //Thothie h4x JAN2010_29 (fail)
		//m_Img.setPos( x, y ); //Thothie h4x JAN2010_29 (fail)

		//MiB h4x - no effect
		//m_Img.setSize( XRES( w ) , YRES( h ) ); //MIB h4x JAN2010_29
		//m_Img.setBounds( XRES(x), YRES(y), XRES(w), YRES(h) ); //MIB h4x JAN2010_29
	}
	void Update()
	{
		//JAN2010_18 - Thothie we always showhudimgs
		//if( EngineFunc::CVAR_GetFloat("ms_showhudimgs")==0 )
		//	m_Img.setVisible( false );
		//else
		m_Img.setVisible(true);
	}
	bool IsActive(void)
	{
		return (m_Dur == -1.0) || (gpGlobals->time < (m_Time + m_Dur));
	}
	void Kill(void)
	{
		m_Time = 0;
		m_Dur = 0;
		m_Img.setFgColor(0, 0, 0, 0);
		m_Img.LoadImg("skull", false, false);
		m_Img.ClearImg();
	}

	~VGUI_ImgIcon()
	{
		Kill();
	}
};

class VGUI_Status : public Panel, public IHUD_Interface
{
public:
	mslist<VGUI_StatusIcon *> m_Status;
	mslist<VGUI_ImgIcon *> m_Img;
	CImageDelayed m_FN;
	bool m_bFN;

	VGUI_Status(Panel *pParent) : Panel(0, 0, ScreenWidth, ScreenHeight)
	{
		startdbg;
		StatusIcons = this;

		dbg("Begin");
		setParent(pParent);
		SetBGColorRGB(Color_Transparent);

		//FN Sprite
		m_bFN = true;
		m_FN.setParent(this);
		m_FN.LoadImg(FN_SPRITE, false, false);
		m_FN.setSize(FN_W, FN_H);
		m_FN.setPos(XRES(640) - FN_W, 10);
		m_FN.setFgColor(255, 255, 255, 255);
		m_FN.setVisible(false);

		enddbg;
	}

	void Update(void)
	{
		//Update/Check Status Icons
		size_t i = 0;
		while (i < m_Status.size())
		{
			if (!m_Status[i]->IsActive()) // Kill Status
			{
				m_Status[i]->Kill();
				m_Status.erase(i);
			}

			else
			{
				m_Status[i]->setPos((ICON_W * (i / 5)) + 10, (ICON_H * (i % 5)) + 10);
				m_Status[i]->Update();
				i++;
			}
		}

		//Update/Check Imgs
		i = 0;
		while (i < m_Img.size())
		{
			if (!m_Img[i]->IsActive()) // Kill Img
			{
				m_Img[i]->Kill();
				m_Img.erase(i);
			}
			else
			{
				m_Img[i]->Update();
				i++;
			}
		}

		//FN Check
		if (m_bFN == false)
			m_FN.setFgColor(255, 255, 255, 255);
		else
			m_FN.setFgColor(0, 0, 0, 0);
	}
};

static void SetFN(bool Up)
{
	StatusIcons->m_bFN = Up;
	if (Up)
		StatusIcons->m_FN.setVisible(false);
	else
		StatusIcons->m_FN.setVisible(true);
}
static void AddStatus(const char *Icon, const char *Name, float Dur, bool bSprite = false)
{
	StatusIcons->m_Status.add(new VGUI_StatusIcon(StatusIcons, Icon, Name, Dur, 0, 0, bSprite));
}

static void AddImg(const char *Img, const char *Name,
				   int x, int y, int w, int h, float Dur = -1.0)
{
	bool found = false;
	for (int i = 0; i < StatusIcons->m_Img.size(); i++)
		//Check for ID conflictions
		if (!strcmp(Name, StatusIcons->m_Img[i]->m_Name))
		{
			found = true;
			break;
		}

	if (!found)
	{
		//Thothie JAN2010_29 - convert x/y/h/w coords to % of screen
		float fimg_x = ScreenWidth * (x * 0.01);
		float fimg_y = ScreenHeight * (y * 0.01);
		int img_x = (int)fimg_x;
		int img_y = (int)fimg_y;

		float fimg_w = ScreenWidth * (w * 0.01);
		float fimg_h = ScreenHeight * (h * 0.01);
		int img_w = (int)fimg_w;
		int img_h = (int)fimg_h;

		StatusIcons->m_Img.add(
			new VGUI_ImgIcon(StatusIcons, Img, Name, img_x, img_y, img_w, img_h, Dur));
	}
}

void KillStatus(const char *Name)
{
	for (int i = 0; i < StatusIcons->m_Status.size(); i++)
	{
		if (!strcmp(StatusIcons->m_Status[i]->m_Name, Name))
		{
			StatusIcons->m_Status[i]->Kill();
			StatusIcons->m_Status.erase(i);
			break;
		}
	}
}
void KillImg(const char *Name)
{
	for (int i = 0; i < StatusIcons->m_Img.size(); i++)
	{
		if (!strcmp(StatusIcons->m_Img[i]->m_Name, Name))
		{
			StatusIcons->m_Img[i]->Kill();
			StatusIcons->m_Img.erase(i);
			break;
		}
	}
}
void KillAllStatus()
{
	while (StatusIcons->m_Status.size() > 0)
	{
		StatusIcons->m_Status[0]->Kill();
		StatusIcons->m_Status.erase(0);
	}
}
void KillAllImgs()
{
	while (StatusIcons->m_Img.size() > 0)
	{
		StatusIcons->m_Img[0]->Kill();
		StatusIcons->m_Img.erase(0);
	}
}
void KillAll()
{
	KillAllStatus();
	KillAllImgs();
}

//This is where the hud.xxxx messages from the client comes in
int __MsgFunc_StatusIcons(const char *pszName, int iSize, void *pbuf)
{
	startdbg;
	dbg("Reading..");
	BEGIN_READ(pbuf, iSize);
	int Type = READ_SHORT();

	//Kill All Status Icons
	if (Type == REMOVE_IMG)
	{
		dbg("Reading.. Remove Img");
		const char *ID = READ_STRING();
		if (!strcmp(ID, "all"))
			KillAllImgs();
		else
			KillImg(ID);
	}

	//Kill All Status Icons
	else if (Type == REMOVE_STATUS)
	{
		dbg("Reading.. Remove Status");
		const char *ID = READ_STRING();
		if (!strcmp(ID, "all"))
			KillAllStatus();
		else
			KillStatus(ID);
	}

	//Kill All Icons
	else if (Type == REMOVE_ALL)
	{
		dbg("Reading.. Remove All");
		KillAll();
	}

	//Add Status Icon
	else if (Type == ADD_STATUS)
	{
		dbg("Reading.. Add Status");
		msstring Icon = READ_STRING();
		msstring Name = READ_STRING();
		float Dur = READ_FLOAT();
		bool isTGA = (READ_BYTE() == 1);
		if (strcmp(Name, "all"))
			AddStatus(Icon.c_str(), Name.c_str(), Dur, isTGA);
	}

	//Add Img Icon
	else if (Type == ADD_IMG)
	{
		dbg("Reading.. Add Img");
		msstring Icon = READ_STRING();
		msstring Name = READ_STRING();
		int x = READ_SHORT();
		int y = READ_SHORT();
		int w = READ_SHORT();
		int h = READ_SHORT();
		float Dur = READ_FLOAT();
		if (strcmp(Name, "all"))
			AddImg(Icon.c_str(), Name.c_str(), x, y, w, h, Dur);
	}

	//FN Connection
	else if (Type == FN_UP)
		SetFN(true);

	else if (Type == FN_DW)
		SetFN(false);

	enddbg;
	return 0;
}
