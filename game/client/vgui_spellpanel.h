/*#ifndef VGUI_SPELL_PANEL
#define VGUI_SPELL_PANEL

#include "../MSShared/sharedutil.h"
#include "vgui_mscontrols.h"

#define Y_SPACER YRES(10)

#define SPELL_INFO_X			XRES(0)
#define SPELL_INFO_Y			YRES(110)
#define SPELL_INFO_SIZE_X		( 128 + XRES(10) )
#define SPELL_INFO_SIZE_Y		YRES(260)

#define SPELL_INFO_SPACER		XRES(15)

#define SPELL_NAME_X			(SPELL_INFO_X + SPELL_INFO_SIZE_X) + SPELL_INFO_SPACER
#define SPELL_NAME_Y			SPELL_INFO_Y
#define SPELL_NAME_SIZE_X		XRES(440)
#define SPELL_NAME_SIZE_Y		YRES(260)

class VGUI_SpellContainer;
class VGUI_SpellList;
// Spell Information Display
class VGUI_SpellInfo : public CTransparentPanel
{
public:
	MSLabel					*m_Name, *m_School, *m_Mana, *m_PrepTime;
	VGUI_SpellContainer		*m_pParent;
	
	void SetLabels( msstring name, msstring school, float mana, float prep )
	{
		m_Name->setText( name.c_str() );
		m_School->setText( school.c_str() );

		msstring temp = msstring( "Mana Usage: " ) + ( msstring() + mana );
		m_Mana->setText( temp.c_str() );

		temp = msstring( "Preparation Time: " ) + ( msstring() + prep );
		m_PrepTime->setText( temp.c_str() );
	}

	VGUI_SpellInfo (int iTrans) : CTransparentPanel(iTrans,SPELL_INFO_X,SPELL_INFO_Y,SPELL_INFO_SIZE_X,SPELL_INFO_SIZE_Y)
	{;;;}
};

// Spell "Button"
class VGUI_SpellButton : public CTransparentPanel
{
	MSLabel					*m_Name;
	int						idx;

	VGUI_SpellList			*m_pParent;

	void					Reset( );
	void					Select( );
	void					DoubleClick( );

	VGUI_SpellButton (int y) : CTransparentPanel(iTrans,x,y,wide,tall)
	{;;;}
};

// Spell List
class VGUI_SpellList : public CTransparentPanel
{
private:
	mslist<VGUI_SpellButton *>	m_ButtonList;
	CTFScrollPanel				*m_ScrollPanel;

public:
	void Update					( void );
	VGUI_SpellContainer			*m_pParent;
	int							curSelect;

	VGUI_SpellList ( void ) : CTransparentPanel(50,SPELL_NAME_X,SPELL_NAME_Y,SPELL_NAME_SIZE_X,SPELL_NAME_SIZE_Y)
	{;;;}
};

// Container
class VGUI_SpellContainer : public CTransparentPanel
{
private:
	MSButton			*m_Select, *m_Close;
	MSLabel				*m_Title;
	VGUI_SpellList		*m_SpellList;
	VGUI_SpellInfo		*m_SpellInfo;

public:
	virtual void		Open( void );
	virtual void		Close( void );
	virtual void		Update( void );
	virtual void		Initialize( void );

	virtual void		ItemHighlighted( void *pData );

	VGUI_SpellContainer ( ) : CTransparentPanel(80,0,0,ScreenWidth,ScreenHeight)
	{;;;}
};

#endif //VGUI_SPELL_PANEL*/