#include "../hud.h"
#include "../cl_util.h"
#include "../vgui_teamfortressviewport.h"
#include "../inc_weapondefs.h"
#include "vgui_mscontrols.h"
#include "clglobal.h"
#include "vgui_loadtga.h"

/*
	Checkbox
*/
void Action_ClickCheck::actionPerformed(Panel* panel)
{
	m_pCheckBtn->doClick( );
}

struct bitmapresource_t {
	BitmapTGA *m_TGA;
	HLSPRITE m_Sprite;
	msstring sImageName;
	bool m_TGAorSprite;
};
mslist<bitmapresource_t> g_Bitmaps;

BitmapTGA *MSBitmap::GetTGA(const char *pszImageName)
{
	bitmapresource_t Bitmap;

	//Return existing TGA, or load from file
	if( !pszImageName )
		return NULL;

	for (unsigned int i = 0; i < g_Bitmaps.size(); i++) 
	{
		if( g_Bitmaps[i].m_TGAorSprite && !strcmp(g_Bitmaps[i].sImageName, pszImageName) )
			return g_Bitmaps[i].m_TGA; //Return existing TGA
	}

	//Create new TGA
	clrmem( Bitmap );
	Bitmap.sImageName = pszImageName;

	Bitmap.m_TGA = LoadTGAForRes( pszImageName );
	if ( !Bitmap.m_TGA ) Bitmap.m_TGA = vgui_LoadTGA( UTIL_VarArgs( "gfx/vgui/%s.tga", pszImageName ) );
	Bitmap.m_TGAorSprite = true;

	g_Bitmaps.push_back( Bitmap );

	return Bitmap.m_TGA;
}

HLSPRITE MSBitmap::GetSprite(const char *pszImageName)
{
	//Return existing sprite, or load from file
	if( !pszImageName )
		return NULL;

	 for (unsigned int i = 0; i < g_Bitmaps.size(); i++) 
		if( !g_Bitmaps[i].m_TGAorSprite && !strcmp(g_Bitmaps[i].sImageName, pszImageName) )
			return g_Bitmaps[i].m_Sprite;						//Return existing Sprite

	//Create new Sprite
	bitmapresource_t Bitmap;
	clrmem( Bitmap );
	Bitmap.sImageName = pszImageName;
	Bitmap.m_TGAorSprite = false;
	Bitmap.m_Sprite = LoadSprite( pszImageName ); 
	if( Bitmap.m_Sprite )
		g_Bitmaps.push_back( Bitmap );

	return Bitmap.m_Sprite;
}

HLSPRITE MSBitmap::LoadSprite(const char *pszImageName)
{
	//Load new sprite from file
	HLSPRITE Sprite = SPR_Load(msstring("sprites/") + pszImageName + ".spr");
	if( !Sprite ) Sprite = gHUD.GetSprite( gHUD.GetSpriteIndex(pszImageName) );
	return Sprite;
}

void MSBitmap::ReloadSprites()
{
	//Reload sprites after a video or level change
	 for (unsigned int i = 0; i < g_Bitmaps.size(); i++) 
		if( !g_Bitmaps[i].m_TGAorSprite )
			g_Bitmaps[i].m_Sprite = LoadSprite( g_Bitmaps[i].sImageName );
}

/*
	CImageDelayed - Can load an image as a sprite or TGA.  Can preload the image filename, then load the image data on demand, if desired
*/
CImageDelayed::CImageDelayed( const char *pszImageName, bool TGAorSprite, bool Delayed, int x, int y, int wide, int tall ) : CImageLabel( NULL, x, y, wide, tall )
{
	setFgColor( 255, 255, 255, 255 );
	LoadImg( pszImageName, TGAorSprite, Delayed );
}

void CImageDelayed::LoadImg( )
{
	if( m_ImageLoaded )
		return;	//already loaded image

	if( m_TGAorSprite )
	{
		setImage( m_pTGA=MSBitmap::GetTGA( m_ImageName ) ); //load image
	}
	else
	{
		m_SpriteHandle = MSBitmap::GetSprite( m_ImageName );
		if( m_SpriteHandle )
		{
			m_Size.left = 0;
			m_Size.right = getImageWide();
			m_Size.top = 0;
			m_Size.bottom = getImageTall();
			setSize( m_Size.right, m_Size.bottom );	//Resize to image size
		}
	}

	m_ImageLoaded = true;
}

void CImageDelayed::LoadImg( const char *pszImageName, bool TGAorSprite, bool Delayed )
{
	//The image name is stored, but we may delay loading of the image data until LoadImg() is called.
	//Some image buttons are never be shown... this saves us from ever loading those
	ClearImg( );

	m_ImageName = pszImageName ? pszImageName : "";
	m_TGAorSprite = TGAorSprite;
	
	if( !Delayed )
		LoadImg( );
}

void CImageDelayed::ClearImg( )
{
	m_ImageLoaded = false;
	m_pTGA = NULL;
	m_SpriteHandle = 0;
	setImage( NULL );
}

void CImageDelayed::paintBackground()
{
	if ( m_TGAorSprite || !m_SpriteHandle )
		return;

	int coords[2] = { 0 };
	localToScreen( coords[0], coords[1] );

	int Ofs[2] = { 0, 0 };
	int ClipBeg[2] = { m_Size.left, m_Size.top };
	int ClipEnd[2] = { m_Size.right, m_Size.bottom };
	int ClipEndOrg[2] = { ClipEnd[0], ClipEnd[1] };
	//Have to do manual clipping for sprites...
	Panel *Parent = this;
	while( Parent = Parent->getParent() )
	{
		int parentcoords[2] = { 0 }, parentsize[2] = { 0 };
		Parent->localToScreen( parentcoords[0], parentcoords[1] );
		Parent->getSize( parentsize[0], parentsize[1] );

		 for (int c = 0; c < 2; c++) 
		{
			int startofs = coords[c] - parentcoords[c];
			if( startofs < 0 && ClipBeg[c] < -startofs )
					ClipBeg[c] = Ofs[c] = -startofs;
			int endofs = (coords[c] + (!c ? m_Size.right : m_Size.bottom)) - (parentcoords[c] + parentsize[c]);
			if( endofs > 0 && ClipEnd[c] > ClipEndOrg[c]-endofs )
				ClipEnd[c] = ClipEndOrg[c] - endofs;
		}

	}


	Rect DisplaySize = { ClipBeg[0], ClipEnd[0], ClipBeg[1], ClipEnd[1] };

	Color fgColor;
	getFgColor( fgColor );
	SPR_Set( m_SpriteHandle, fgColor[0], fgColor[1], fgColor[2] );
	if( fgColor[3] >= 255 )
		SPR_DrawHoles( m_Frame, Ofs[0], Ofs[1], &DisplaySize );
	else
		SPR_DrawAdditive( m_Frame, Ofs[0], Ofs[1], &DisplaySize );	
}

int CImageDelayed::getImageWide( void )
{
	int iXSize = { 0 }, iYSize;
	if( m_pTGA ) m_pTGA->getSize( iXSize, iYSize );
	else if( m_SpriteHandle ) iXSize = SPR_Width( m_SpriteHandle, 0 );
	return iXSize;
}

int CImageDelayed::getImageTall( void )
{
	int iXSize, iYSize = { 0 };
	if( m_pTGA ) m_pTGA->getSize( iXSize, iYSize );
	else if( m_SpriteHandle ) iYSize = SPR_Height( m_SpriteHandle, 0 );
	return iYSize;
}


//Fading Text
void VGUI_FadeText::MSInit( float FadeDuration, const char *pszText )
{
	m_Text = pszText;
	m_StartTime = gpGlobals->time;
	m_Alpha = 255;
	m_FadeDuration = FadeDuration;
	Color OldColor = getMSFGColor( );
	setFgColor( OldColor[0], OldColor[1], OldColor[2], m_Alpha );
}

void VGUI_FadeText::Update( )
{
	float TimeRatio = (gpGlobals->time - m_StartTime) / m_FadeDuration;
	TimeRatio = V_max(V_min(TimeRatio,1.0f),0);
	if( m_FadeOut ) TimeRatio = 1.0f - TimeRatio;
	m_Alpha = 255 * TimeRatio;
	m_Alpha = 255 - V_min(m_Alpha,255);
	Color OldColor = getMSFGColor( );
	setFgColor( OldColor[0], OldColor[1], OldColor[2], m_Alpha );
}

void VGUI_FadeText::StartFade( bool FadeOut ) { m_StartTime = gpGlobals->time; m_FadeOut = FadeOut; }

// ---------------------------------------
// A list of items which shows their icons. 
// User can click an icon to select the icon
// for some function
// ---------------------------------------

//Master Sword - Item system
//--------------------------

static COLOR	EnabledColor = COLOR( 255, 255, 255, 0 ), 
				DisabledColor = COLOR( 128, 128, 128, 80 ), 
				Color_DisabledText = COLOR( 128, 128, 128, 0 ), 
				ItemColor_Selected = COLOR( 0, 255, 0, 255 ), 
				ItemColor_Highlighted = COLOR( 255, 255, 255, 255 ), 
				ItemColor_Normal = COLOR( 180, 180, 180, 255 ), 
				ItemColor_NormalImage = COLOR( 140, 140, 140, 255 ), 
				ItemColor_Disabled = COLOR( 60, 60, 60, 255 ), 
				ItemColor_DisabledText = COLOR( 60, 60, 60, 0 ), 
				TransparentColor = COLOR( 0, 0, 0, 255 );

containeritem_t::containeritem_t( genericitem_t &Item )
{
	CGenericItem *pItem = CGenericItemMgr::GetGlobalGenericItemByName( Item.Name, true );
	init( pItem );
	
	ID = Item.ID;
	Properties = Item.Properties;
	Quantity = Item.Quantity;
	Quality = Item.Quality;
	Weight = 0.0;
}

containeritem_t::containeritem_t( CGenericItem *pItem )
{
	init( pItem );
}
void containeritem_t::init( CGenericItem *pItem )
{
	Name = pItem->ItemName;
	DebugName = pItem->ItemName.c_str();
	SpriteName = pItem->TradeSpriteName ? msstring("items/640_") + pItem->TradeSpriteName : msstring("");
	SpriteFrame = pItem->SpriteFrame;  //Shuriken FEB2008
	FullName = SPEECH::ItemName( pItem, true );
	Disabled = false;
	ID = pItem->m_iId;
	Properties = pItem->MSProperties( );
	Quantity = FBitSet( Properties, ITEM_GROUPABLE ) ? pItem->iQuantity : 1;
	Quality = pItem->Quality;
	Value = pItem->m_Value;
	Desc = pItem->DisplayDesc;
	bForceFull = false;
	pOrig = pItem;
	Weight = pItem->Weight();
}

msstring containeritem_t::getFullName( )
{
	if ( !pOrig || bForceFull ) return FullName;
	return SPEECH::ItemName( pOrig, true );
}

void containeritem_t::setFullName( msstring name )
{
	bForceFull = true;
	FullName = name;
}

class CHandler_ItemButton : public InputSignal
{
private:
	VGUI_ItemButton *m_pItemButton;
	VGUI_DoubleClickDetector *mpDoubleClickDetector;

public:
	CHandler_ItemButton(VGUI_ItemButton *pItemButton, VGUI_DoubleClickDetector* pDoubleClickDetector)
	{
		m_pItemButton = pItemButton;
		mpDoubleClickDetector = pDoubleClickDetector;
	}
		
	void cursorEntered(Panel *panel) { m_pItemButton->Highlight(true); }
	void cursorExited(Panel* panel) { m_pItemButton->Highlight(false); };
	void mousePressed(MouseCode code,Panel* panel) 
	{ 
		if (code == MOUSE_LEFT)
		{
			if ( mpDoubleClickDetector->Click(this, code) && m_pItemButton->m_Selected)
			{
				mouseDoublePressed(code, panel);
			}
			else
			{
				m_pItemButton->Clicked();
			}
		}
		else if (code == MOUSE_RIGHT)
		{
			m_pItemButton->RightClicked();
		}
	};
	void cursorMoved(int x,int y,Panel* panel) {};
	void mouseReleased(MouseCode code,Panel* panel) {};
	void mouseDoublePressed( MouseCode code, Panel* panel ) { m_pItemButton->Doubleclicked( ); };
	void mouseWheeled(int delta,Panel* panel) {};
	void keyPressed(KeyCode code,Panel* panel) {};
	void keyTyped(KeyCode code,Panel* panel) {};
	void keyReleased(KeyCode code,Panel* panel) {};
	void keyFocusTicked(Panel* panel) {};
};

#define ITEMBTN_REG_MIN_SIZE_X		164
#define ITEMBTN_REG_MIN_SIZE_Y		128
#define ITEMBTN_SML_MIN_SIZE_X		40
#define	ITEMBTN_SML_MIN_SIZE_Y		32
#define ITEMBTN_LABEL_START_X		XRES(0)
#define ITEMBTN_LABEL_START_Y		(ITEMBTN_REG_MIN_SIZE_Y + YRES(4))
#define ITEMBTN_LABEL_SIZE_Y		YRES(12)

//Item button

VGUI_ItemButton::VGUI_ItemButton( int x, int y, VGUI_ItemCallbackPanel *pCallbackPanel, CTFScrollPanel *pParent ) : CTransparentPanel( 255, x, y, 1, 1 )
{
	setParent( pParent->getClient() );
	m_CallbackPanel = pCallbackPanel;
	m_PanelMaxWidth = pParent->getWide() - pParent->getVerticalScrollBar()->getWide();

	// Get the scheme used for the Titles
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();

	// schemes
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
	SchemeHandle_t hClassWindowText = pSchemes->getSchemeHandle( "Briefing Text" );

	m_Image.setParent( this );

	//Set the label
	 for (unsigned int i = 0; i < ITEMBTN_LABELS_MAX; i++) 
		m_Labels[i] = new MSLabel( this, "Item", 0, 0, 1, 1, MSLabel::a_center );

	m_Description = new TextPanel( "", 0, 0, 1, 1 );
	m_Description->setParent( this ); 
	m_Description->setBgColor( 0, 0, 0, 255 );
	m_Description->setFont( g_FontSml );

	// MIB Note: Make sure this is always last so the layering makes this on top
	//Make the hitbox button cover the whole thing
	m_Button = new CTransparentPanel( 0, 0, 0, getWide(), getTall() );
	m_Button->setParent( this );

	m_Button->addInputSignal( m_Signal = new CHandler_ItemButton( this, pCallbackPanel ) );
}

/*VGUI_ItemButton::~VGUI_ItemButton( )
{
	removeChild( Image );
	m_Button->removeInputSignal( m_Signal );
	removeChild( m_Button );

	 for (unsigned int i = 0; i < ITEMBTN_LABELS; i++) 
	{
		removeChild( Labels[i] );
		delete Labels[i];
	}
}*/

void VGUI_ItemButton::SetItem( containeritem_t &Item )
{
	m_Data = Item;
	Update( );
}

void VGUI_ItemButton::Update( )
{
	int iInvType = atoi(gEngfuncs.pfnGetCvarString( "ms_invtype" ));

	if( m_Data.SpriteName )
	{
		msstring spriteName = m_Data.SpriteName;
		if ( iInvType == INVTYPE_SMALL )
			spriteName += "_small";

		m_Image.LoadImg( spriteName, false, false );
		m_Image.SetFrame( m_Data.SpriteFrame ); //Shuriken FEB2008 
	}

	int sizex = iInvType == INVTYPE_SMALL ? ITEMBTN_SML_MIN_SIZE_X : ITEMBTN_REG_MIN_SIZE_X;
	int sizey = iInvType == INVTYPE_SMALL ? ITEMBTN_SML_MIN_SIZE_Y : ITEMBTN_REG_MIN_SIZE_Y;
	int maxWidth = 0;
	int maxHeight = 0;
	
	if ( m_Image.m_ImageLoaded )
	{
		int imgw = m_Image.getWide( ),
		    imgh = m_Image.getTall( );
		
		if( imgw > sizex ) sizex = imgw;
		if( imgh > sizey ) sizey = imgh;

		// Center the image
		int x = (sizex/2.0f)-(imgw/2.0f),
			y = (sizey/2.0f)-(imgh/2.0f);

		m_Image.setPos( x, y );
		int iImageRightEdge = x + imgw;

		int lblx, lbly;
		int w, h;

		 for (unsigned int i = 0; i < ITEMBTN_LABELS_MAX; i++) 
		{
			switch( i )
			{
				case 0: m_Labels[i]->setText( m_Data.getFullName()	); break; // Item Name
				//case 1: m_Labels[i]->setText( m_Data.Desc		); break; // Item Description
			}

			//m_Labels[i]->setVisible( i < numLabels );
			//if ( i > numLabels ) continue;

			switch( iInvType )
			{
				case INVTYPE_ORIGINAL:
					lblx = ITEMBTN_LABEL_START_X;
					lbly = ITEMBTN_LABEL_START_Y + (ITEMBTN_LABEL_SIZE_Y * i);
					w = sizex;
					h = ITEMBTN_LABEL_SIZE_Y;
					maxHeight = lbly + h;
					m_Labels[i]->setContentAlignment( vgui::Label::Alignment::a_center );
					break;
				case INVTYPE_DESC:
				case INVTYPE_SMALL:
					lblx = iImageRightEdge + XRES(4);
					if ( iInvType == INVTYPE_SMALL ) lbly = (sizey/2) - (ITEMBTN_LABELS_MAX*ITEMBTN_LABEL_SIZE_Y)/2 + (ITEMBTN_LABEL_SIZE_Y * i);
					else lbly = (sizey/2) - ((2+ITEMBTN_LABELS_MAX)*ITEMBTN_LABEL_SIZE_Y)/2 + (ITEMBTN_LABEL_SIZE_Y * i);
					w = m_PanelMaxWidth - lblx;
					h = ITEMBTN_LABEL_SIZE_Y;
					maxHeight = sizey;
					m_Labels[i]->setContentAlignment( vgui::Label::Alignment::a_west );
					break;
			}
			if ( w + lblx > maxWidth ) maxWidth = w + lblx;

			m_Labels[i]->setPos( lblx, lbly );
			m_Labels[i]->setSize( w, h );
		}

		m_Description->setText( m_Data.Desc );
		m_Description->setPos( lblx, lbly + ITEMBTN_LABEL_SIZE_Y );
		m_Description->setSize( w, ITEMBTN_LABEL_SIZE_Y * 2 );
		m_Description->setVisible( iInvType == INVTYPE_DESC );
	}

	//Set my size
	setSize( maxWidth, maxHeight );

	//Make the hitbox button cover the whole thing
	m_Button->setSize( getWide(), getTall() );

	//Give the owner window a chance to disable this item
	if( m_CallbackPanel ) m_CallbackPanel->ItemCreated( &m_Data );

	COLOR LabelColor = EnabledColor;

	setVisible( true );
	m_Button->setEnabled( !m_Data.Disabled );
}

void VGUI_ItemButton::Reset( )
{
	setVisible( false );
	m_Selected = false;
	m_Highlighted = false;
}

void VGUI_ItemButton::Clicked( )
{
	if( !m_CallbackPanel || !m_CallbackPanel->ItemClicked( this ) )
		Select( !m_Selected );
}

void VGUI_ItemButton::RightClicked()
{
    if ( m_CallbackPanel ) 
		m_CallbackPanel->ItemRightClicked(this);
}

void VGUI_ItemButton::Doubleclicked( )
{
	if( m_CallbackPanel ) 
		m_CallbackPanel->ItemDoubleclicked( m_Data.ID );
}

void VGUI_ItemButton::Select( bool fSelect )
{
	m_Selected = fSelect;
	if( m_CallbackPanel ) 
		m_CallbackPanel->ItemSelectChanged( m_Data.ID, m_Selected );
}

void VGUI_ItemButton::Highlight( bool fSelect )
{
	m_Highlighted = fSelect;
	if( m_CallbackPanel ) 
		m_CallbackPanel->ItemHighlighted( this );
	//if( !m_Highlighted )
}

void VGUI_ItemButton::paint( )
{
	COLOR ImageColor, LabelColor;

	if( m_Data.Disabled )
	{
		ImageColor = ItemColor_Disabled;
		LabelColor = ItemColor_DisabledText;
	}
	else
	{
		if( m_Selected )
		{
			ImageColor = LabelColor = ItemColor_Selected;
		}
		else
		{
			if( m_Highlighted )
			{
				ImageColor = LabelColor = ItemColor_Highlighted;
			}
			else
			{
				ImageColor = ItemColor_NormalImage;
				LabelColor = ItemColor_Normal;
			}
		}
		//If enabled, flip the label's alpha since I set the label and the image to 
		//the same color but the label requires inverted alpa
		LabelColor.a = 255 - LabelColor.a;
	}

	m_Image.SetFGColorRGB( ImageColor );
	for (unsigned int i = 0; i < ITEMBTN_LABELS_MAX; i++) 
		m_Labels[i]->SetFGColorRGB( LabelColor );
	m_Description->SetFGColorRGB( LabelColor );
}

class CHandler_ScrollPanel : public InputSignal
{
	CTransparentPanel	*m_pListenerPanel;
	CTFScrollPanel		*m_pScrollPanel;
	bool				m_RequireInside, m_IsInside;

	void SetInside( bool b ) { m_IsInside = b; mouseWheeled( b ? 1 : -1, m_pListenerPanel);}
	void Construct( CTransparentPanel *panel, CTFScrollPanel *scroll, bool requireInside )
	{
		m_pListenerPanel	= panel;
		m_pScrollPanel		= scroll;
		m_RequireInside		= requireInside;
	}
public:
	CHandler_ScrollPanel( CTransparentPanel *panel, CTFScrollPanel *scroll ) { Construct( panel, scroll, true ); }
	CHandler_ScrollPanel( CTransparentPanel *panel, CTFScrollPanel *scroll, bool requireInside ){ Construct( panel, scroll, requireInside ); }

	void mouseWheeled(int delta,Panel* panel) 
	{
		int h, v;
		m_pScrollPanel->getScrollValue(h,v);
		m_pScrollPanel->setScrollValue(h,v+(10*delta));
	}

	void mousePressed( MouseCode code,Panel* panel )  { }
	void cursorEntered( Panel *panel ) { SetInside( true ); }
	void cursorMoved(int x,int y,Panel* panel) { }
	void mouseReleased(MouseCode code,Panel* panel) { }
	void mouseDoublePressed( MouseCode code, Panel* panel ) { }
	void cursorExited( Panel* panel ) { SetInside( false ); }
	void keyPressed(KeyCode code,Panel* panel) { }
	void keyTyped(KeyCode code,Panel* panel) { }
	void keyReleased(KeyCode code,Panel* panel) { }
	void keyFocusTicked(Panel* panel) { }
};

// Container Creation
VGUI_Container::VGUI_Container( int x, int y, int w, int h, VGUI_ItemCallbackPanel *pCallbackPanel, Panel *pParent ) : 
	CTransparentPanel( INVENTORY_TRANSPARENCY, x, y, w, h )
{
	setParent( pParent );
	m_CallbackPanel = pCallbackPanel;
	m_InitializedItemButtons = 0;
	m_ItemButtonTotal = 0;

	m_pScrollPanel = new CTFScrollPanel( 0, 0, getWide(), getTall() );
	m_pScrollPanel->setParent( this );
	m_pScrollPanel->setScrollBarAutoVisible( false, true );
	m_pScrollPanel->setScrollBarVisible( false, false );

	m_pInvTypePanel = new VGUI_InvTypePanel( pParent, this );

	m_NoItems = new MSLabel( m_pScrollPanel->getClient(), "No items", XRES(10), YRES(10) );
	m_NoItems->SetFGColorRGB( Color_DisabledText );

	m_pScrollPanel->validate( );
}

VGUI_Container::~VGUI_Container( )
{
	/*PurgeButtons( );

	m_pScrollPanel->getClient()->removeChild( m_NoItems );
	delete m_NoItems;
	
	removeChild( m_pScrollPanel );
	delete m_pScrollPanel;*/
}

VGUI_ItemButton *VGUI_Container::AddItem(containeritem_t &Item)
{
	if (m_ItemButtonTotal >= m_InitializedItemButtons)
	{
		VGUI_ItemButton *pNewButton = new VGUI_ItemButton(0, 0, m_CallbackPanel, m_pScrollPanel);
		m_ItemButtons.push_back(pNewButton);
		m_AlphabetizedItemButtons.push_back(pNewButton);
		m_InitializedItemButtons++;
	}

	VGUI_ItemButton &NewItemButton = *m_ItemButtons[m_ItemButtonTotal];
	NewItemButton.SetItem(Item);

	unsigned int iAlphaLoc = m_ItemButtonTotal;

	if (m_ItemButtonTotal > 0)
	{
		msstring sNewItemName = Item.getFullName();

		// Binary search to find the insertion point
		int lo = 0, hi = m_ItemButtonTotal - 1;
		while (lo <= hi)
		{
			int mid = lo + (hi - lo) / 2;
			if (_stricmp(m_AlphabetizedItemButtons[mid]->m_Data.getFullName().c_str(), sNewItemName) <= 0)
				lo = mid + 1;
			else
				hi = mid - 1;
		}
		iAlphaLoc = lo;

		for (unsigned int i = m_ItemButtonTotal; i > iAlphaLoc; --i)
			m_AlphabetizedItemButtons[i] = m_AlphabetizedItemButtons[i - 1];
	}

	m_AlphabetizedItemButtons[iAlphaLoc] = &NewItemButton;

	m_NoItems->setVisible(false);
	m_ItemButtonTotal++;

	const int invType = atoi(gEngfuncs.pfnGetCvarString("ms_invtype"));
	if (IsAlphabetical())
	{
		for (unsigned int i = iAlphaLoc; i < m_ItemButtonTotal; ++i)
			UpdatePosition(i, invType);
	}
	else
	{
		UpdatePosition(m_ItemButtonTotal - 1, invType);
	}

	m_pScrollPanel->validate();
	return &NewItemButton;
}

void VGUI_Container::Update( )
{
	bool bIsAlpha = IsAlphabetical();
	const int invType = atoi(gEngfuncs.pfnGetCvarString("ms_invtype"));
	for (unsigned int i = 0; i < m_ItemButtonTotal; i++) 
	{
		// MiB FEB2019_24 [ALPHABETICAL_INVENTORY]
		if ( bIsAlpha )
			m_AlphabetizedItemButtons[i]->Update();
		else
			m_ItemButtons[i]->Update();

		UpdatePosition(i, invType);
	}
	m_pScrollPanel->validate();
}

void VGUI_Container::UpdatePosition(int idx, int invType)
{
	auto &ActiveList = IsAlphabetical() ? m_AlphabetizedItemButtons : m_ItemButtons;

	int x = 0, y = 10;

	if (idx > 0)
	{
		VGUI_ItemButton *LastItemButton = ActiveList[idx - 1];
		LastItemButton->getPos(x, y);

		switch (invType)
		{
			case INVTYPE_ORIGINAL:
				x += LastItemButton->getWide();
				if (x + LastItemButton->getWide() >= (m_pScrollPanel->getWide() - m_pScrollPanel->getVerticalScrollBar()->getWide()))
				{
					x = 0;
					y += LastItemButton->getTall();
				}
				break;

			case INVTYPE_SMALL:
			case INVTYPE_DESC:
				x = 0;
				y += LastItemButton->getTall();
				break;
		}
	}

	ActiveList[idx]->setPos(x, y);
}

void VGUI_Container::PurgeButtons( )
{
	//Reset the old buttons
	 for (unsigned int i = 0; i < m_ItemButtonTotal; i++) 
		m_ItemButtons[i]->Reset( );

	m_ItemButtonTotal = 0;

	m_NoItems->setVisible( true );
	m_pScrollPanel->setScrollValue( 0, 0 );
}


//VGUI_TextPanel

void VGUI_TextPanel::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	if( !m_Active )
		return;

	if( !down )
	{
		return;
	}

	if( keynum == '\r' )
	{
		return; //Do nothing - this should be handled by my parent
	}
	else if( keynum == 127 )
	{
		if( m_Message.len() )
			m_Message.str()[m_Message.len()-1] = 0;
		m_VisibleMsg = m_Message;
		UpdateText( );
		return;
	}
	else if( keynum == 134 )
	{
		return;
	}

// TODO!!!
#ifdef _WIN32
	byte State[256];
	BOOL Success = GetKeyboardState( State );
	if( !Success ) return;

	unsigned short Char;
	int Result = ToAscii( VkKeyScan(keynum), keynum, State, &Char, 0 );
	if( Result != 1 ) return;

	if ( Char == 37 || Char == 92 ) return; //AUG2013_18 - sanitize inputs

	if( (signed)m_Message.len() >= m_MaxLetters ) return;

	//char Temp[2] = { m_Caps ? toupper(Char) : Char, '\0' };
	char Temp[2] = { Char & 0xFF, '\0' };
	AddLetter( Temp );
#endif
}

// MiB FEB2019_24 [ALPHABETICAL_INVENTORY]
VGUI_CheckBox::VGUI_CheckBox(const char *pszText, int x, int y, int w, int h, bool bStartChecked)
	: Panel( x, y, w, h )
{
	int vTmp;
	mpHandler = nullptr;
	SetChecked(bStartChecked);
	SetArmed(false);
	mpTextLabel = new Label(pszText);
	mCheckSquareSize = V_min(mCheckSquareSize, h / 2);
	mpTextLabel->getTextSize(vTmp, mCheckSquareSize);
	mpTextLabel->setPos(CHECKBOX_LABELPOS_X, 0);
	//mpTextLabel->setSize(w - CHECKBOX_LABELPOS_X, h);
	mpTextLabel->setSize(w, h);
	mpTextLabel->setBgColor(0, 0, 0, 255);
	mpTextLabel->setParent(this);
	mpTextLabel->setContentAlignment(Label::Alignment::a_center);

	// MiB Note: Make sure this is always last so the layering makes this on top
	// Make the hitbox button cover the whole thing
	mpOverlayButton = new CTransparentPanel(0, 0, 0, getWide(), getTall());
	mpOverlayButton->setParent(this);
	mpOverlayButton->addInputSignal(new CheckHandler(this));
}

VGUI_CheckBox::~VGUI_CheckBox()
{
	if ( mpHandler ) 
		delete mpHandler;
}

void VGUI_CheckBox::paint()
{
	Scheme::SchemeColor vSchemeColor = IsArmed() ? Scheme::sc_secondary2 : Scheme::sc_primary1;

	setFgColor(vSchemeColor);
	mpTextLabel->setFgColor(vSchemeColor);

	drawSetColor(vSchemeColor);
	drawOutlinedRect(CHECKBOX_CHECKSPACER, CHECKBOX_CHECKPOS_Y, CHECKBOX_CHECKSPACER + mCheckSquareSize, CHECKBOX_CHECKPOS_Y + mCheckSquareSize);
	if (IsChecked())
	{
		drawFilledRect( CHECKBOX_CHECKSPACER + CHECKBOX_INNERSPACER, CHECKBOX_CHECKPOS_Y + CHECKBOX_INNERSPACER, CHECKBOX_CHECKSPACER + mCheckSquareSize - CHECKBOX_INNERSPACER, CHECKBOX_CHECKPOS_Y + mCheckSquareSize - CHECKBOX_INNERSPACER);
	}

	Panel::paint();
}

void VGUI_CheckBox::paintBackground()
{
	if (IsArmed())
	{
		drawSetColor(Scheme::sc_primary2);
		drawFilledRect(0,0,_size[0],_size[1]);
	}

	drawSetColor(Scheme::sc_secondary1);
	drawOutlinedRect(0,0,_size[0],_size[1]);
}
