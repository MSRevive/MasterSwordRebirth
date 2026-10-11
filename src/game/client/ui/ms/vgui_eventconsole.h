// doesn't matter if it SHOULD be only included by vgui_hud.cpp, 
// should still have header guard.
#pragma once

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
		ConsolePrint(m_Text + "\n");
	}

	msstring m_Text;
	Color m_Color;
	int m_TextWidth;
	bool m_SpansFromPrevLine;
	static int LineHeight;
};
int EventConsoleText::LineHeight = 0;

class VGUI_EventConsole : public Panel
{
public:

	#define EVENTCON_LINE_SIZE_Y EventConsoleText::LineHeight
	#define EVENTCON_MAXLINES 128

	#define EVENTCON_PREF_VISIBLELINES					m_VisLines->value
	#define EVENTCON_PREF_MAXLINES						m_MaxLines->value
	#define EVENTCON_PREF_DECAYTIME						m_DecayTime->value

	struct prefs_t
	{
		const char* VisLines;
		const char* MaxLines;
		const char* DecayTime;
		const char* BGTrans;
		const char* Width;
	};

	int LogicalToPhysical( int logical ) const
	{
		int MaxLines = V_min((int)EVENTCON_PREF_MAXLINES, EVENTCON_MAXLINES);
		return (m_Head + logical) % MaxLines;
	}

	EventConsoleText* GetLine( int logical ) const
	{
		return m_Line[ LogicalToPhysical(logical) ];
	}

	VGUI_EventConsole( Panel *pParent, int x, int y, int w, int h, prefs_t &Prefs, bool DynamicWidth = false, Font *TextFont = NULL ) : Panel( x, y, w, h )
	{
		setParent( pParent );
		setBgColor( 0, 0, 20, 128 );
		m_TotalLines = 0;
		m_ActiveLine = 0;
		m_ShrinkTime = 0;
		m_VisibleLines = 0;
		m_Head = 0;
		m_StartY = y;
		m_VisLines = gEngfuncs.pfnGetCvarPointer( Prefs.VisLines );
		m_MaxLines = gEngfuncs.pfnGetCvarPointer( Prefs.MaxLines );
		m_DecayTime = gEngfuncs.pfnGetCvarPointer( Prefs.DecayTime );
		m_BGTrans = gEngfuncs.pfnGetCvarPointer( Prefs.BGTrans );
		m_Width = Prefs.Width ? gEngfuncs.pfnGetCvarPointer( Prefs.Width ) : NULL;
		m_DynamicWidth = DynamicWidth;

		setSize( GetWidth(), EVENTCON_LINE_SIZE_Y * m_VisibleLines );

		m_ScrollPanel = new CTFScrollPanel( 0, 0, getWide(), EVENTCON_MAXLINES * EVENTCON_LINE_SIZE_Y );
		m_ScrollPanel->setParent( this );
		m_ScrollPanel->setScrollBarAutoVisible(false, true);
		m_ScrollPanel->setScrollBarVisible(false, false);
		m_ScrollPanel->getVerticalScrollBar()->getSlider()->setVisible( false );
		m_ScrollPanel->validate();
		m_ScrollBarWidth = m_ScrollPanel->getVerticalScrollBar()->getWide();
		int ScrollClientWidth = m_ScrollPanel->getWide() - m_ScrollBarWidth;

		for (int i = 0; i < EVENTCON_PREF_MAXLINES; i++) 
		{
			m_Line[i] = new EventConsoleText( 0, 0, ScrollClientWidth, EVENTCON_LINE_SIZE_Y, NULL );
			if( TextFont ) 
				m_Line[i]->setFont( TextFont );
		}

		Resize( );
	}

	void Print( Color color, const char* Text )
	{
		Print( color, Text, false );
	}

	void Print( Color color, const char* Text, bool WrappedFromLastLine )
	{
		if (!Text || !Text[0])
			return;

		int MaxLines = V_min(EVENTCON_PREF_MAXLINES, EVENTCON_MAXLINES);

		if( m_TotalLines >= MaxLines )
		{
			m_Head = (m_Head + 1) % MaxLines;
		}
		else
		{
			m_TotalLines++;
		}

		// The newest line is always at logical index (m_TotalLines - 1)
		int iNewLine = m_TotalLines - 1;

		// If the active line was tracking the bottom, keep it there
		if( m_ActiveLine >= (iNewLine - 1) )
			m_ActiveLine = iNewLine;

		// Reposition all active lines in the scroll panel
		// (VGUI needs the panels laid out top-to-bottom by logical order)
		for( int i = 0; i < m_TotalLines; i++ )
		{
			EventConsoleText *line = GetLine(i);
			line->setParent( m_ScrollPanel->getClient() );
			line->setPos( 0, EVENTCON_LINE_SIZE_Y * i );
			line->setSize( line->getWide(), EVENTCON_LINE_SIZE_Y );
		}

		EventConsoleText &NewLine = *GetLine(iNewLine);

		if( m_VisibleLines < EVENTCON_PREF_VISIBLELINES )
			m_VisibleLines++;

		m_ShrinkTime = 0;
		NewLine.m_SpansFromPrevLine = WrappedFromLastLine;
		
		const char* ThisLineText = Text;
		msstring NextLineText;
		char ctemp[512] = "";

		int w, h;
		int MaxWidth = NewLine.getWide();
		int MaxHeight = EVENTCON_LINE_SIZE_Y;
		NewLine.getTextImage()->getFont()->getTextSize( ThisLineText, w, h );

		if( w > MaxWidth || h > MaxHeight )
		{
			int TextLen = (int)strlen(Text);
			int WrapPos = -1;
			int WrapLength = -1;
			bool SkipChar = false;

			// Binary search for approximate overflow point
			int lo = 0, hi = TextLen;
			while( lo < hi )
			{
				int mid = (lo + hi) / 2;
				strncpy(ctemp, Text, mid + 1);
				ctemp[mid + 1] = 0;
				int testw, testh;
				NewLine.getTextImage()->getFont()->getTextSize( ctemp, testw, testh );
				if( testw > MaxWidth || testh > MaxHeight )
					hi = mid;
				else
					lo = mid + 1;
			}

			int lastSpacePos = -1;
			int widthAtLastSpace = 0;
			int lastGoodWidth = 0;
			int scanEnd = V_min(lo, TextLen - 1);

			for( int c = 0; c <= scanEnd; c++ )
			{
				if( Text[c] == '\n' )
				{
					WrapPos = c;
					SkipChar = true;
					w = lastGoodWidth;
					break;
				}
				if( Text[c] == ' ' )
				{
					lastSpacePos = c;
					strncpy(ctemp, Text, c + 1);
					ctemp[c + 1] = 0;
					int sw, sh;
					NewLine.getTextImage()->getFont()->getTextSize( ctemp, sw, sh );
					widthAtLastSpace = sw;
				}

				if( c == lo )
				{
					if( lastSpacePos >= 0 )
					{
						WrapPos = lastSpacePos;
						SkipChar = true;
						w = widthAtLastSpace;
					}
					else
					{
						WrapPos = c;
						SkipChar = false;
					}
					break;
				}

				strncpy(ctemp, Text, c + 1);
				ctemp[c + 1] = 0;
				int tw, th;
				NewLine.getTextImage()->getFont()->getTextSize( ctemp, tw, th );
				lastGoodWidth = tw;
			}

			if( WrapPos > 0 )
			{
				int WrapEnd   = WrapPos;
				int WrapStart = WrapPos + (SkipChar ? 1 : 0);
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
		if( !m_VisibleLines || !ShowHUD())
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
				int logIdx = m_ActiveLine - i;
				EventConsoleText *line = GetLine(logIdx);
				int linewidth = V_min( line->m_TextWidth, line->getWide() );
 				if( linewidth > w ) w = linewidth;
			}
		}

		setSize( w + m_ScrollBarWidth, EVENTCON_LINE_SIZE_Y * m_VisibleLines );
		m_ScrollPanel->setSize( getWide( ), getTall() );

		Color color;
		getBgColor( color );
		int bgAlpha = 128 + V_max(V_min(m_BGTrans->value, 1), -1) * 127;
		setBgColor( color[0], color[1], color[2], bgAlpha );

		bool canScrollUp = ((m_ActiveLine + 1) - m_VisibleLines) > 0;
		bool canScrollDown = m_ActiveLine < (m_TotalLines - 1);

		Button *TopScrollBtn = m_ScrollPanel->getVerticalScrollBar()->getButton(0);
		Button *BtmScrollBtn = m_ScrollPanel->getVerticalScrollBar()->getButton(1);

		TopScrollBtn->setVisible( canScrollUp && !m_DynamicWidth );
		BtmScrollBtn->setVisible( canScrollDown );

		if( m_VisibleLines <= 1 && canScrollDown )
			TopScrollBtn->setVisible( false );

		m_ScrollPanel->setScrollValue( 0, EVENTCON_LINE_SIZE_Y * ((m_ActiveLine+1) - m_VisibleLines) );
		m_ScrollPanel->validate( );
	}

	void Update( )
	{
		if( !m_ShrinkTime )
		{
			if( m_VisibleLines )
				m_ShrinkTime = gpGlobals->time + EVENTCON_PREF_DECAYTIME;
		}
		else if ((gpGlobals->time > m_ShrinkTime) && (m_VisibleLines > 0))
		{
			m_VisibleLines--;
			Resize( );

			int TopLine = V_max( m_ActiveLine - (m_VisibleLines - 1), 0 );
			EventConsoleText *topLine = GetLine(TopLine);

			if (topLine != nullptr && !topLine->m_SpansFromPrevLine)
				m_ShrinkTime = 0;
		}

		if(!ShowHUD())
			setVisible( false );
		else if( m_VisibleLines ) 
			setVisible( true );
	}

	void StepInput( bool fDown )
	{
		m_VisibleLines = V_min(EVENTCON_PREF_VISIBLELINES, m_TotalLines);
		m_ShrinkTime = 0;

		if( fDown )
			m_ActiveLine = V_min(m_ActiveLine+1, m_TotalLines-1);
		else
			m_ActiveLine = V_max(m_ActiveLine-1, m_VisibleLines-1);

		Resize( );
	}

	float GetWidth( )
	{
		return m_Width ? XRES(m_Width->value) : getWide();
	}

	CTFScrollPanel *m_ScrollPanel;
	EventConsoleText *m_Line[EVENTCON_MAXLINES];
	int m_Head;	// index of the oldest (logical 0) line
	int m_VisibleLines,
		m_TotalLines,
		m_ActiveLine,
	    m_StartY,
		m_ScrollBarWidth;
	float m_ShrinkTime;
	cvar_t *m_VisLines, *m_MaxLines,
		   *m_DecayTime, *m_BGTrans,
		   *m_Width;
	bool m_DynamicWidth;
};