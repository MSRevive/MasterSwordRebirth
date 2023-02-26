#ifndef MSCONTROLS_H
#define MSCONTROLS_H

#include "storeshared.h"

#define MAX_CONTAINER_ITEMS 256

inline RGBA MakeRGBA( uchar r, uchar g, uchar b, uchar a )
{
	RGBA NewColor;
	NewColor.r = r;
	NewColor.g = g;
	NewColor.b = b;
	NewColor.a = a;
	return NewColor;
}

class MSButton : public CommandButton
{
public:
	MSButton::MSButton( Panel *pParent, const char *pszText, int x, int y, int w, int h ) :
		CommandButton( pszText, x, y, w, h, false )
	{
		MSInit( pParent, "", x, y, w, h );
	}
	MSButton::MSButton( Panel *pParent, const char *pszText, int x, int y, int w, int h, COLOR ArmedColor, COLOR UnArmedColor ) :
		CommandButton( pszText, x, y, w, h, false )
	{
		MSInit( pParent, pszText, x, y, w, h );
		SetArmedColor( ArmedColor );
		SetUnArmedColor( UnArmedColor );
	}

	void MSInit( Panel *pParent, const char *pszText, int x, int y, int w, int h )
	{
		setParent( pParent );
		setContentAlignment( Label::a_east );
		int r, g, b, a;
		getFgColor( r, g, b, a );
		m_UnArmedColor.r = r;
		m_UnArmedColor.g = g;
		m_UnArmedColor.b = b;
		m_UnArmedColor.a = a;
		setBgColor( 0, 0, 0, 255 );
		m_DisabledColor = COLOR( 127, 127, 127, 255 );
		setFont( g_FontSml );
		setText( pszText );
		m_AutoFitText = false;
	}

	inline void SetArmedColor( COLOR &Color )
	{
		m_ArmedColor = Color;
	}
	inline void SetUnArmedColor( COLOR &Color )
	{
		m_UnArmedColor = Color;
	}
	inline void SetDisabledColor( COLOR &Color )
	{
		m_DisabledColor = Color;
	}
	inline void SetBgColor( COLOR &Color )
	{
		setBgColor( Color.r, Color.g, Color.b, Color.a );
	}
	void paint( )
	{
		if( isEnabled() )
		{
			if( isArmed() )
			{
				setFgColor( m_ArmedColor.r, m_ArmedColor.g, m_ArmedColor.b, m_ArmedColor.a );
			}
			else
			{
				setFgColor( m_UnArmedColor.r, m_UnArmedColor.g, m_UnArmedColor.b, m_UnArmedColor.a );
			}
		}
		else
		{
			setFgColor( m_DisabledColor.r, m_DisabledColor.g, m_DisabledColor.b, m_DisabledColor.a );
		}

		Button::paint( );
	}
	void paintBackground( )
	{
		//Button::paintBackground( );
		Label::paintBackground( );
	}

	void setText( const char *Text )
	{
		CommandButton::setText( Text );
		if( m_AutoFitText ) FitText( );;
	}
	void FitText( )
	{
		int w, h;
		getTextSize( w, h );
		setSize( w, h );
	}

	COLOR m_ArmedColor, m_UnArmedColor, m_DisabledColor;
	bool m_AutoFitText;
};
class MSLabel : public Label
{
public:
	enum Alignment
	{
		a_northwest=0,
		a_north,
		a_northeast,
		a_west,
		a_center,
		a_east,
		a_southwest,
		a_south,
		a_southeast,
	};

	MSLabel::MSLabel( Panel *pParent, const char *pszText, int x, int y, int w, int h, MSLabel::Alignment Alignment = a_west ) :
		Label( pszText, x, y, w, h )
	{
		MSInit( pParent, pszText, Alignment );
		setSize( w, h );
	}
	MSLabel::MSLabel( Panel *pParent, const char *pszText, int x, int y, MSLabel::Alignment Alignment = a_west ) :
		Label( pszText, x, y )
	{
		MSInit( pParent, pszText, Alignment );
	}

	void MSInit( Panel *pParent, msstring_ref Text, MSLabel::Alignment Alignment )
	{
		setParent( pParent );
		//setFont( gViewPort->GetSchemeManager()->getFont( gViewPort->GetSchemeManager()->getSchemeHandle( "Briefing Text" ) ) );
		setFgColor( 255, 255, 255, 0 );
		setBgColor( 0, 0, 0, 255 );
		setContentAlignment( vgui::Label::Alignment(Alignment/*-1*/) );
		setFont( g_FontSml );
		setText( Text );
	}
};

//Fading text
class VGUI_FadeText : public MSLabel
{
public:
	VGUI_FadeText( Panel *pParent, float FadeDuration, const char *pszText, int x, int y, MSLabel::Alignment Alignment = a_west ) :
		MSLabel( pParent, pszText, x, y, Alignment )
	{
		MSInit( FadeDuration, pszText );
	}
	VGUI_FadeText( Panel *pParent, float FadeDuration, const char *pszText, int x, int y, int w, int h, MSLabel::Alignment Alignment = a_west ) :
		MSLabel( pParent, pszText, x, y, w, h, Alignment )
	{
		MSInit( FadeDuration, pszText );
	}

	void MSInit( float FadeDuration, const char *pszText );
	void StartFade( bool FadeOut = false );
	void Update( );

	int m_Alpha;
	float m_FadeDuration;
	float m_StartTime;
	msstring m_Text;
	bool m_FadeOut;
};

//Status bar
class CStatusBar : public CommandButton
{
public:
	float Percentage;
	bool m_fBorder;
	bool mbDrawNeg;
	COLOR mAntiChargeColor;

	CStatusBar(Panel *pParent, int x, int y, int wide, int tall, bool bDrawNeg = false) : CommandButton( "", x, y, wide, tall, true )
	{
		setParent( pParent );
		Percentage = 0;
		m_fBorder = true;
		mbDrawNeg = bDrawNeg;
		SetAntiChargeColor(COLOR(0,0,0,255));
	}
	void SetAntiChargeColor(COLOR c)
	{
		mAntiChargeColor = c;
	}
	inline void Set( float fAmt, float fMax )
	{
		Percentage = (fAmt / fMax) * 100.0f;
		if( Percentage < 0 ) Percentage = 0;
	}
	inline void Set( float fPercent )
	{
		Percentage = fPercent;
	}
	inline float Get( )
	{
		return Percentage;
	}
	void EnableBorder( bool fEnable )
	{
		m_fBorder = fEnable;
	}
	void paint() { Button::paint(); }

	void paintBackground()
	{
		// Border
		int r, g, b, a;
		int vBarJoin;

		if ( mbDrawNeg )
		{
			vBarJoin = _size[0] * (1 - (Percentage * 0.01));
			getFgColor( r, g, b, a );
			drawSetColor( r, g, b, a );
			drawFilledRect( vBarJoin, 0, _size[0], _size[1]);
			drawSetColor(mAntiChargeColor.r, mAntiChargeColor.g, mAntiChargeColor.b, mAntiChargeColor.a);
			drawFilledRect(0, 0, vBarJoin,	_size[1]);
		}
		else
		{
			vBarJoin = _size[0] * (Percentage * 0.01);
			getFgColor(r, g, b, a);
			drawSetColor(r, g, b, a);
			drawFilledRect( 0, 0, vBarJoin,	_size[1]);
			drawSetColor(mAntiChargeColor.r, mAntiChargeColor.g, mAntiChargeColor.b, mAntiChargeColor.a);
			drawFilledRect(vBarJoin, 0,	_size[0], _size[1]);
		}

		if( m_fBorder )
		{
			getBgColor( r, g, b, a );
			drawSetColor( r, g, b, a );
			drawOutlinedRect(0,0,_size[0],_size[1]);
		}
	}
};

//Check button
class Action_ClickCheck : public ActionSignal
{
protected:
	class CheckButton *m_pCheckBtn;
public:
	Action_ClickCheck( class CheckButton *pCheckBtn )
	{
		m_pCheckBtn = pCheckBtn;
	}

	virtual void actionPerformed(Panel* panel);
};

class CheckButton : public CommandButton
{
	char m_CVARName[256];
public:
	CheckButton( char *CVARName, int x, int y, int wide, int tall ) : CommandButton( "", x, y, wide, tall, true )
	{
		 strncpy(m_CVARName,  CVARName, sizeof(m_CVARName) );
		m_CVARName[255] = 0;
		addActionSignal( new Action_ClickCheck( this ) );
	}

	void doClick()
	{
		float m_State = gEngfuncs.pfnGetCvarFloat( m_CVARName );
		m_State = !m_State;
		gEngfuncs.Cvar_SetValue( m_CVARName, m_State );
	}

	void paintBackground()
	{
		float m_State = gEngfuncs.pfnGetCvarFloat( m_CVARName );
		// Orange Border
		drawSetColor( Scheme::sc_secondary1 );
		if( m_State )
			drawFilledRect(0,0,_size[0],_size[1]);
		else 
			drawOutlinedRect(0,0,_size[0],_size[1]);
	}
};


//Line
class VGUI_Line : public Panel
{
public:
	VGUI_Line( int x, int y, int Length, bool fHorizontalDirection, int ThickNess, COLOR &color, Panel *pParent ) : Panel( x, y, 0, 0 )
	{
		MSInit( x, y, Length, fHorizontalDirection, ThickNess, color, pParent );
	}
	VGUI_Line( int x, int y, int Length, bool fHorizontalDirection, COLOR &color, Panel *pParent ) : Panel( x, y, 0, 0 )
	{
		MSInit( x, y, Length, fHorizontalDirection, fHorizontalDirection ? XRES(2) : YRES(2), color, pParent );
	}
	void MSInit( int x, int y, int Length, bool fHorizontalDirection, int Thickness, COLOR &color, Panel *pParent )
	{
		setParent( pParent );

		if( fHorizontalDirection ) setSize( Length, Thickness );
		else setSize( Thickness, Length );

		setBorder( new LineBorder( Thickness, Color(color.r,color.g,color.b,color.a) ) );
	}
};


//CImageDelayed - An image that doesn't load its data until LoadImg() is called (usually called when displayed)
//Update: Can dislay sprites or TGA images
//Update: It can also create regular images that load upon creation
class CImageDelayed : public CImageLabel
{
public:
	msstring m_ImageName;
	HLSPRITE m_SpriteHandle;
	int m_imgw, m_imgh;
	wrect_t m_Size;
	int m_Frame;		//Current frame
	bool m_TGAorSprite; //True = TGA, False = Sprite
	bool m_ImageLoaded; //For delayed images... is it loaded yet?

	CImageDelayed( ) : CImageLabel( "", 0, 0, 10, 10 ) { m_ImageName[0] = 0; m_ImageLoaded = false; }
	CImageDelayed( const char *pszImageName, bool TGAorSprite, bool Delayed, int x = 0, int y = 0, int wide = 10, int tall = 10 );

	virtual int getImageTall();
	virtual int getImageWide();

	void ClearImg( );
	virtual void LoadImg( );
	virtual void LoadImg( const char *pszImageName, bool TGAorSprite, bool Delayed );
	void SetFrame( int Frame ) { m_Frame = Frame; }
	int GetMaxFrames( ) { return SPR_Frames(m_SpriteHandle); }
	void paintBackground( );
};

//VGUI_Image3D - A 2D image, drawn as a close-up 3D sprite.  This allows automatic resizing for resolution.
class VGUI_Image3D : public CImageDelayed
{
public:
	class CParticle *m_Particle;
	VGUI_Image3D( ) : CImageDelayed( ) { init( ); }
	VGUI_Image3D( const char *pszImageName, bool TGAorSprite, bool Delayed, int x = 0, int y = 0, int wide = 10, int tall = 10 );
	~VGUI_Image3D( );

	void init( );
	void LoadImg( );
	void LoadImg( const char *pszImageName, bool TGAorSprite, bool Delayed ) { CImageDelayed::LoadImg( pszImageName, TGAorSprite, Delayed ); }
	void paintBackground( );
};

//Callback for when items get selected, etc.
class VGUI_ItemCallbackPanel
{
public:
	virtual void ItemSelectChanged( ulong ID, bool fSelected ) { }
	virtual bool ItemClicked( void *pData  ) { return false; }		//Return true to override the default behavior
	virtual void ItemCreated( void *pData ) { }
	virtual void ItemHighlighted( void *pData ) { }
	virtual void ItemDoubleclicked( ulong ID ) { };
	virtual void GearItemSelected( ulong ID ) { }
	virtual bool GearItemClicked( ulong ID ) { return false; }			//Return true to override the default behavior
	virtual bool GearItemDoubleClicked( ulong ID ) { return false; }	//Return true to override the default behavior
};

//An item in a pack or player hands
#define ITEMBTN_LABELS_MAX 1
#define INVENTORY_TRANSPARENCY		90

class VGUI_ItemButton : public CTransparentPanel
{
public:
	containeritem_t			m_Data;
	CTransparentPanel		*m_Button;
	CImageDelayed			m_Image;
	MSLabel					*m_Labels[ITEMBTN_LABELS_MAX];
	TextPanel				*m_Description;
	bool					m_Selected, m_Highlighted;
	VGUI_ItemCallbackPanel	*m_CallbackPanel;	//Calls this on events, like item select, double-click, etc.
	InputSignal				*m_Signal;
	int						m_PanelMaxWidth;

	VGUI_ItemButton( int x, int y, class VGUI_ItemCallbackPanel *pCallbackPanel, CTFScrollPanel *pParent );
	//~VGUI_ItemButton( );
	void SetItem( containeritem_t &Item );
	void Update( );

	void Reset( );
	void Clicked( );
	void Doubleclicked( );
	void Select( bool fSelect );
	void Highlight( bool fSelect );
	void paint( );
};


//Item container - used in Buy, sell, inventory...
class VGUI_Container : public CTransparentPanel
{
public:
	mslist<VGUI_ItemButton *>		m_ItemButtons;
	int								m_ItemButtonTotal;
	int								m_InitializedItemButtons;
	CTFScrollPanel					*m_pScrollPanel;
	VGUI_ItemCallbackPanel			*m_CallbackPanel;
	MSLabel							*m_NoItems;

	class VGUI_InvTypePanel			*m_pInvTypePanel;

	void							Update();
	void							UpdatePosition( int idx );

	void							StepInput( bool bDirUp ); // MIB FEB2015_21 [INV_SCROLL]
	
	#define INVTYPE_ORIGINAL		0
	#define INVTYPE_SMALL			1
	#define INVTYPE_DESC			2

	//pGearItem == NULL means uses held items instead of pack items
	VGUI_Container( int x, int y, int w, int h, class VGUI_ItemCallbackPanel *pCallbackPanel, Panel *pParent );
	~VGUI_Container( );

	VGUI_ItemButton *AddItem( containeritem_t &Item );
	void PurgeButtons( );
};

class VGUI_InvTypePanel : public CTransparentPanel
{
public:
	#define INVTYPE_BUTTONS_TOTAL 3
	mslist<ClassButton *>					InvTypeButtons;

	VGUI_InvTypePanel( Panel *pParent, VGUI_Container *pCallback );
};

// One-line Text box - Can be typed in by user
class VGUI_TextPanel : public CTransparentPanel
{
public:
	MSLabel *m_MessageLabel;
	VGUI_Line *m_Line;
	msstring m_Message, m_VisibleMsg;
	mslist<VGUI_FadeText *> m_FadeLabels;
	float m_TimeLastLetter;
	float m_StartWidth;
	bool m_Active;
	int m_MaxLetters;

	VGUI_TextPanel( Panel *Parent, int x, int y, int w, int h ) : CTransparentPanel( 128, x, y, w, h )
	{
		setSize( w, h );
		m_Message = ""; m_VisibleMsg = "";
		setParent( Parent );
		m_StartWidth = w;

		//m_MessageLabel = new MSLabel( this, "", TEXT_START, 0, getWide() - TEXT_START - XRES(3), h, MSLabel::a_west );
		m_MessageLabel = new MSLabel( this, "", 0, 0, getWide() - XRES(3), h, MSLabel::a_west );
		m_MessageLabel->setBorder( new LineBorder( 1, Color(255,0,0,255) ) );
		COLOR vColor = COLOR(128, 128, 128, 128);
		m_Line = new VGUI_Line(0, h - YRES(2), m_MessageLabel->getWide(), 1, true, vColor, this);
		m_MaxLetters = 120;
	}

	void Open( )
	{
		m_Message = ""; m_VisibleMsg = "";
		setVisible( true );
		setSize( m_StartWidth, getTall() );
		SetActive( true );
		UpdateText( );
	}

	void SetActive( bool Active )
	{
		m_Active = Active;
		m_Line->setVisible( m_Active );
	}

	void AddLetter( msstring_ref Letter )
	{
		int x, y;
		m_MessageLabel->getPos( x, y );
		int w, h;
		m_MessageLabel->getTextSize( w, h );
		x += w + 1;
		for (int i = 0; i < m_FadeLabels.size(); i++) { m_FadeLabels[i]->getTextSize(w, h); x += w; }
		m_FadeLabels.add( new VGUI_FadeText( this, 0, Letter, x, y, XRES(16), m_MessageLabel->getTall() ) );
		m_Message += Letter;

		if( getWide() - x < XRES(10) )
		{
			setSize( getWide() + XRES(10), getTall() );
			m_MessageLabel->setSize( getWide(), m_MessageLabel->getTall() );
			m_Line->setSize( m_MessageLabel->getWide(), m_Line->getTall() );
		}
	}

	//Instantly update to full text
	void UpdateText( )
	{
		m_VisibleMsg = m_Message;
		m_MessageLabel->setText( m_Message );
		int sz =  m_FadeLabels.size();
		 for (int i = 0; i < sz; i++) 
		{
			m_FadeLabels[0]->setParent( NULL );
			delete m_FadeLabels[0];
			m_FadeLabels.erase( 0 );
		}
	}

	//Instantly update to this text
	void SetText( msstring_ref Text )
	{
		m_Message = Text;
		UpdateText( );
	}

	void Update( )
	{
		 for (int i = 0; i < m_FadeLabels.size(); i++) 
		{
			VGUI_FadeText *pFadeText = m_FadeLabels[i];
			pFadeText->Update( );
			if( pFadeText->m_Alpha <= 0 )
			{
				m_VisibleMsg += pFadeText->m_Text;
				pFadeText->setVisible( false );
				pFadeText->setParent( NULL );
				m_MessageLabel->setText( m_VisibleMsg );
				delete pFadeText;
				m_FadeLabels.erase( i-- );
			}
		}
	}

	void KeyInput( int down, int keynum, const char *pszCurrentBinding );
};


#endif
