//
//  This should only be included by vgui_HUD.cpp
//

class EventConsoleText : public TextPanel
{
public:
	EventConsoleText( int x, int y, int w, int h, Panel *pParent ) : TextPanel( "", x, y, w, h ) 
	{ 
		setParent( pParent ); 
		setBgColor( 0, 0, 0, 255 ); 
		setFont( g_FontSml );
		LineHeight = g_FontSml->getTall();
	}
	void setText( Color color, const char *Text )
	{
		m_Text = Text;
		m_Color = color;
		TextPanel::setText( m_Text.c_str() );
		setFgColor( m_Color[0], m_Color[1], m_Color[2], m_Color[3] );
	}
	void CopyLine( EventConsoleText *pNewTextLine )
	{
		setText( pNewTextLine->m_Color, pNewTextLine->m_Text );
		m_TextWidth = pNewTextLine->m_TextWidth;
		m_SpansFromPrevLine = pNewTextLine->m_SpansFromPrevLine;
	}
	void Archive( )
	{
		if( m_Archived )
			return;

		ConsolePrint( m_Text + "\n" );
		m_Archived = true;
	}

	msstring m_Text;
	Color m_Color;
	int m_TextWidth;
	bool m_SpansFromPrevLine,		//The text has a carriage return or is just so long it goes to the next line
		 m_Archived;				//The text has been logged to console
	static int LineHeight;
};
int EventConsoleText::LineHeight = 0;

class VGUI_EventConsole : public Panel
{
public:

	#define EVENTCON_LINE_SIZE_Y EventConsoleText::LineHeight//YRES(10)
	#define EVENTCON_MAXLINES 256

	#define EVENTCON_PREF_VISIBLELINES					m_VisLines->value
	#define EVENTCON_PREF_MAXLINES						m_MaxLines->value
	#define EVENTCON_PREF_DECAYTIME						m_DecayTime->value

	struct prefs_t
	{
		msstring_ref VisLines;
		msstring_ref MaxLines;
		msstring_ref DecayTime;
		msstring_ref BGTrans;
		msstring_ref Width;
	};
	
	VGUI_EventConsole( Panel *pParent, int x, int y, int w, int h, prefs_t &Prefs, bool DynamicWidth = false, Font *TextFont = NULL ) : Panel( x, y, w, h )
	{
		setParent( pParent );
		setBgColor( 0, 0, 20, 128 );
		m_TotalLines = 0;
		m_ActiveLine = 0;
		m_ShrinkTime = 0;
		m_VisibleLines = 0;
		m_StartY = y;
		m_VisLines = gEngfuncs.pfnGetCvarPointer( Prefs.VisLines );
		m_MaxLines = gEngfuncs.pfnGetCvarPointer( Prefs.MaxLines );
		m_DecayTime = gEngfuncs.pfnGetCvarPointer( Prefs.DecayTime );
		m_BGTrans = gEngfuncs.pfnGetCvarPointer( Prefs.BGTrans );
		m_Width = Prefs.Width ? gEngfuncs.pfnGetCvarPointer( Prefs.Width ) : NULL;
		m_DynamicWidth = DynamicWidth;

		setSize( GetWidth(), EVENTCON_LINE_SIZE_Y * m_VisibleLines );

		// Create the Scroll panel
		m_ScrollPanel = new CTFScrollPanel( 0, 0, getWide(), EVENTCON_MAXLINES * EVENTCON_LINE_SIZE_Y );
		m_ScrollPanel->setParent( this );
		m_ScrollPanel->setScrollBarAutoVisible(false, true);
		m_ScrollPanel->setScrollBarVisible(false, false);
		m_ScrollPanel->getVerticalScrollBar()->getSlider()->setVisible( false );
		m_ScrollPanel->validate();
		m_ScrollBarWidth = m_ScrollPanel->getVerticalScrollBar()->getWide();
		int ScrollClientWidth = m_ScrollPanel->getWide() - m_ScrollBarWidth;

		//Create the Text lines
		 for (int i = 0; i < EVENTCON_MAXLINES; i++) 
		{
			m_Line[i] = new EventConsoleText( 0, 0, ScrollClientWidth, EVENTCON_LINE_SIZE_Y, NULL );
			if( TextFont ) 
				m_Line[i]->setFont( TextFont );
		}

		Resize( );

	}
	void Print( Color color, msstring_ref Text )
	{
		Print( color, Text, false );
	}

	void Print( Color color, msstring_ref Text, bool WrappedFromLastLine )
	{
		if( !Text || !strlen(Text) ) 
			return;

		int MaxLines = min( EVENTCON_PREF_MAXLINES, EVENTCON_MAXLINES ); // Max amount of text lines to keep in the history
		int iNewLine = min(m_TotalLines,MaxLines-1);

		//If the active line was the last line, then make this new line the active line
		if( m_ActiveLine >= (iNewLine-1) )
			m_ActiveLine = iNewLine;

		for( int i = 0; i <= iNewLine; i++ )
		{
			if( (m_TotalLines >= MaxLines) && i < iNewLine )		//We've run out of lines.  Move each line up to the previous line
				m_Line[i]->CopyLine( m_Line[i+1] );

			m_Line[i]->setParent( m_ScrollPanel->getClient() );
			m_Line[i]->setPos( 0, EVENTCON_LINE_SIZE_Y * i );
			m_Line[i]->setSize( m_Line[i]->getWide(), EVENTCON_LINE_SIZE_Y );
		}
		
		EventConsoleText &NewLine = *m_Line[iNewLine];

		//Make the window grow... up to the maximum visible lines
		if( m_VisibleLines < EVENTCON_PREF_VISIBLELINES ) m_VisibleLines++;
		else
			m_Line[m_ActiveLine - m_VisibleLines]->Archive( );		//Window is at full size, start archiving text above what's visible
		m_TotalLines = min( m_TotalLines+1, MaxLines );				//Increment the total number of lines... up to the maximum kept in history
		m_ShrinkTime = 0;											//Reset shrinktime
		NewLine.m_SpansFromPrevLine = WrappedFromLastLine;
		
		msstring_ref ThisLineText = Text;
		msstring NextLineText;
		char ctemp[512] = "";

		int w, h;
		int MaxWidth = NewLine.getWide();
		int MaxHeight = EVENTCON_LINE_SIZE_Y;
		NewLine.getTextImage()->getFont()->getTextSize( ThisLineText, w, h ); //Test the text to see if it exceeds the boundaries
		if( w > MaxWidth || h > MaxHeight )
		{
			//Line is too long.  
			//Wrap it at the last space before it gets too long,
			//or if no spaces, wrap it at the last character before it gets too long
			int WrapPos = -1;
			int WrapLength = -1;
			bool SkipChar = false;
			 for (int c = 0; c < strlen(Text); c++) 
			{
				strncpy( ctemp, Text, c+1 );
				ctemp[c+1] = 0;
				int testw, testh;
				NewLine.getTextImage()->getFont()->getTextSize( ctemp, testw, testh );
				if( testw > MaxWidth || testh > MaxHeight )
				{
					if( Text[c] == '\n' ) { WrapPos = c; SkipChar = true; } //Skip over the carriage return, so I don't try to print it to the next line
					else if( WrapPos < 0 ) WrapPos = c;						//Wrapped by line too long, but no spaces were found.  Use the last char
					else w = WrapLength;									//Wrapped by line too long and a space was found.  My width only goes up to the space
					break;
				}

				if( Text[c] == ' ' ) { WrapPos = c; SkipChar = true; WrapLength = w; } //Use this space as the breaking point, but skip over the space
				else w = testw;
			}

			if( WrapPos > 0 )
			{
				int WrapEnd   = WrapPos,						//WrapEnd doesn't count the last character.  That's the char that put us over the limit
					WrapStart = WrapPos + (SkipChar ? 1 : 0);	//Skip the breaking character if Skipchar == true (carriage return or space)
				strncpy( ctemp, Text, WrapEnd ); ctemp[WrapEnd] = 0;
				ThisLineText = ctemp;
				NextLineText = &(((char *)Text)[WrapStart]);
			}
		}

		NewLine.setText( color, ThisLineText );
		NewLine.m_TextWidth = w;
		if( NextLineText.len() )
			Print( color, NextLineText, true );

		Resize( );
	}
	void Resize( )
	{
		if( !m_VisibleLines )
		{
			setVisible( false );
			return;
		}

		setVisible( true );
		int x, y;
		getPos( x, y );
		setPos( x, m_StartY - EVENTCON_LINE_SIZE_Y * m_VisibleLines );

		int w = getWide();
		if( m_DynamicWidth )
		{
			w = 0;
 			 for (int i = 0; i < m_VisibleLines; i++) 
			{
				int idx = m_ActiveLine - i;
				int linewidth = min( m_Line[idx]->m_TextWidth, m_Line[idx]->getWide() );
 				if( linewidth > w ) w = linewidth;
			}
		}

		setSize( w + m_ScrollBarWidth, EVENTCON_LINE_SIZE_Y * m_VisibleLines );

		m_ScrollPanel->setSize( getWide( ), getTall() );

		Color color;
		getBgColor( color );
		setBgColor( color[0], color[1], color[2], 128 + max(min(m_BGTrans->value,1),-1) * 127 );

		Button &TopScrollBtn = *m_ScrollPanel->getVerticalScrollBar()->getButton(0);
		if( (m_ActiveLine+1) - m_VisibleLines > 0 )
			TopScrollBtn.setVisible( !m_DynamicWidth );		//Using m_DynamicWidth to determine if this is 
															//the saytext box or the event console
		else
			TopScrollBtn.setVisible( false );

		Button &BtmScrollBtn = *m_ScrollPanel->getVerticalScrollBar()->getButton(1);
		if( m_ActiveLine < (m_TotalLines-1) )
			BtmScrollBtn.setVisible( true );
		else
			BtmScrollBtn.setVisible( false );

		if( m_VisibleLines <= 1 )	
			//When only one line is shwon, give priority to the down arrow so the user knows this isn't the last line in the console
			if( m_ScrollPanel->getVerticalScrollBar()->getButton(1)->isVisible() )
				m_ScrollPanel->getVerticalScrollBar()->getButton(0)->setVisible( false );

		m_ScrollPanel->setScrollValue( 0, EVENTCON_LINE_SIZE_Y * ((m_ActiveLine+1) - m_VisibleLines) ); //Scroll down to the active line

		m_ScrollPanel->validate( );
	}
	void Update( )
	{
		if( !m_ShrinkTime )
		{
			if( m_VisibleLines )
				m_ShrinkTime = gpGlobals->time + EVENTCON_PREF_DECAYTIME;
		}
		else if( gpGlobals->time > m_ShrinkTime )
		{
			//Shrink down one line
			m_Line[m_ActiveLine - (m_VisibleLines-1)]->Archive( );

			m_VisibleLines--;
			Resize( );

			//If the top visible line is a wrapped line, then shrink again... 
			//And keep shrinking until the top visible line is a normal line

			int TopLine = m_ActiveLine - (m_VisibleLines-1);
			TopLine = max( TopLine, 0 );
			if( !m_Line[TopLine]->m_SpansFromPrevLine )
				m_ShrinkTime = 0;
		}

		if( FBitSet(gHUD.m_iHideHUDDisplay,HIDEHUD_ALL) )
			setVisible( false );
		else if( m_VisibleLines ) setVisible( true );
	}

	void StepInput( bool fDown )
	{
		m_VisibleLines = min(EVENTCON_PREF_VISIBLELINES, m_TotalLines);	//Grow to max visible lines
		m_ShrinkTime = 0;																//Reset shrinktime

		if( fDown )
			m_ActiveLine = min(m_ActiveLine+1,m_TotalLines-1);
		else
			m_ActiveLine = max(m_ActiveLine-1,m_VisibleLines-1);

		Resize( );
	}

	float GetWidth( )
	{
		return m_Width ? XRES(m_Width->value) : getWide();
	}


	CTFScrollPanel *m_ScrollPanel;
	EventConsoleText *m_Line[EVENTCON_MAXLINES];
	int m_VisibleLines,						//Current number of visible lines
		m_TotalLines,						//Current number of lines in the console
		m_ActiveLine,						//Bottom line that must be shown in the viewable area (changes with pgup/pgdn)
	    m_StartY,							//Initial Y position
		m_ScrollBarWidth;					//Width of vertical scrollbar
	float m_ShrinkTime;						//Shrink time
	cvar_t *m_VisLines, *m_MaxLines,			//CVARs controlling size and delays
		   *m_DecayTime, *m_BGTrans,
		   *m_Width;
	bool m_DynamicWidth;
};