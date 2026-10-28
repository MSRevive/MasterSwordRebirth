#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL_events.h>

#include <tier0/dbg.h>
#include <tier1/strtools.h>
#include <tier1/KeyValues.h>
#include <tier2/tier2.h>

#include <FileSystem.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/ScrollBarSlider.h>

#include "CChatPanel.h"
#include "vgui2/ms_vgui2.h"

// Its fonts and colors win, the font and color macros below only fill in for keys it doesn't have
#define CHAT_SCHEME_FILE			"resource/ms/ChatScheme.res"

// Chat box. Positions and sizes are in 640x480 units and scale with the screen like XRES/YRES.
#define CHAT_BOX_X					10
#define CHAT_BOX_BOTTOM_Y			350	// Bottom edge, the box grows upwards from here like the old chat
#define CHAT_BOX_WIDTH				280
#define CHAT_BOX_HEIGHT				120
#define CHAT_BOX_INSET				4
#define CHAT_INPUT_MARGIN_TOP		0.5f	// In text lines, between the history and the input
#define CHAT_INPUT_MARGIN_BOTTOM	0.75f	// In text lines, below the input

// In pixels
#define CHAT_PROMPT_GAP				2	// Between the prompt and the entry
#define CHAT_PROMPT_INSET			2
#define CHAT_SCROLLBAR_WIDTH		17

#define CHAT_FONT_NAME				"Sitka"
#define CHAT_FONT_TALL				20
#define CHAT_FONT_WEIGHT			800
#define CHAT_FONT_FLAGS				vgui2::ISurface::FONTFLAG_DROPSHADOW

// Colors are r, g, b(, a)
 #define CHAT_BG_COLOR				0, 0, 0		// History background, only shown while open
 #define CHAT_BG_ALPHA				128
 #define CHAT_FIELD_BG_COLOR			0, 0, 0, 128	// Prompt and entry background
 #define CHAT_PROMPT_COLOR			216, 216, 216, 255
 #define CHAT_TEXT_COLOR				255, 255, 255, 255
 #define CHAT_INPUT_ALPHA			128			// Prompt and entry opacity while open
 #define CHAT_SCROLLBAR_FG_COLOR		0, 0, 0, 128
 #define CHAT_SCROLLBAR_BG_COLOR		0, 0, 0, 0
 #define CHAT_SCROLLBAR_TRACK_COLOR	0, 0, 0, 128

// In seconds
#define CHAT_FADE_TIME				0.25f	// Background fade on open and close
#define CHAT_LINE_FADE_TIME			2.5f	// Each line fades out over this after ms_txthud_decaytime

namespace
{
	// The server relays chat in a 192 byte user message, so this stays small
	const int MAX_CHAT_BYTES = 120;
	const int MAX_CHAT_CHARS = 120;
	const int MAX_HISTORY_CHARS = 127 * 100;
	const float DEFAULT_DECAY_TIME = 10.0f;
	const double KEY_BLOCK_AFTER_CLOSE = 0.05;

	// Enable if the engine won't route keys to a keyboard-only popup
	const bool INPUT_USES_MOUSE = false;

	void SanitizeChat(char *pszText)
	{
		char *pOut = pszText;
		for (const char *pIn = pszText; *pIn; pIn++)
		{
			unsigned char c = (unsigned char)*pIn;
			if (c < 32 || c == '%' || c == '\\')
				continue;

			*pOut++ = *pIn;
		}
		*pOut = 0;
	}

	void TruncateUtf8(char *pszText, int iMaxBytes)
	{
		if ((int)strlen(pszText) <= iMaxBytes)
			return;

		int iEnd = iMaxBytes;
		while (iEnd > 0 && ((unsigned char)pszText[iEnd] & 0xC0) == 0x80)
			iEnd--;

		pszText[iEnd] = 0;
	}

	int ScaleX(float flValue)
	{
		int wide, tall;
		vgui2::surface()->GetScreenSize(wide, tall);
		return (int)(flValue * wide / 640.0f + 0.5f);
	}

	int ScaleY(float flValue)
	{
		int wide, tall;
		vgui2::surface()->GetScreenSize(wide, tall);
		return (int)(flValue * tall / 480.0f + 0.5f);
	}

	// Without the file, the panels keep the client scheme
	vgui2::HScheme LoadChatScheme()
	{
		if (!g_pFullFileSystem || !g_pFullFileSystem->FileExists(CHAT_SCHEME_FILE))
			return 0;

		return vgui2::scheme()->LoadSchemeFromFile(CHAT_SCHEME_FILE, "ChatScheme");
	}

	// Panels keep the default scheme they were created with instead of asking their parent
	void SetSchemeRecursive(vgui2::Panel *pPanel, vgui2::HScheme hScheme)
	{
		pPanel->SetScheme(hScheme);
		for (int i = 0; i < pPanel->GetChildCount(); i++)
		{
			if (vgui2::Panel *pChild = pPanel->GetChild(i))
				SetSchemeRecursive(pChild, hScheme);
		}
	}

	// The scheme's ChatFont, else CHAT_FONT_NAME, else the scheme's Default
	vgui2::HFont GetChatFont(vgui2::IScheme *pScheme)
	{
		vgui2::HFont hSchemeFont = pScheme->GetFont("ChatFont", false);
		if (hSchemeFont != vgui2::INVALID_FONT)
			return hSchemeFont;

		static vgui2::HFont s_hFont = vgui2::INVALID_FONT;
		static bool s_bCreated = false;

		if (!s_bCreated)
		{
			s_bCreated = true;

			vgui2::HFont hFont = vgui2::surface()->CreateFont();
			if (vgui2::surface()->AddGlyphSetToFont(hFont, CHAT_FONT_NAME, CHAT_FONT_TALL, CHAT_FONT_WEIGHT, 0, 0, CHAT_FONT_FLAGS, 0, 0))
				s_hFont = hFont;
		}

		return s_hFont != vgui2::INVALID_FONT ? s_hFont : pScheme->GetFont("Default", false);
	}

	// The engine keeps the wheel from HUD_Key_Event while a VGUI2 panel has key focus, so it is read from SDL.
	// Only queued here, SDL calls this from inside the engine's event pump.
	int SDLCALL ChatEventWatch(void *pData, SDL_Event *pEvent)
	{
		if (pEvent->type == SDL_MOUSEWHEEL && pEvent->wheel.y)
			static_cast<CChat *>(pData)->QueueWheel(pEvent->wheel.y);

		return 0;
	}
}

CChatEntry::CChatEntry(vgui2::Panel *pParent, const char *pszName) : BaseClass(pParent, pszName)
{
	SetAllowNonAsciiCharacters(true);
	SetMaximumCharCount(MAX_CHAT_CHARS);
	SetMultiline(false);
}

void CChatEntry::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_hFont = GetChatFont(pScheme);

	// Without ChatScheme.res the game's schemes have none of these keys, which leaves everything white
	SetFont(m_hFont);
	SetBgColor(pScheme->GetColor("TextEntry.BgColor", Color(CHAT_FIELD_BG_COLOR)));
	SetFgColor(pScheme->GetColor("Chat.TypingText", Color(CHAT_TEXT_COLOR)));
	SetBorder(nullptr);
}

// Posted rather than called, so the panel isn't hidden in the middle of its own key event
void CChatEntry::OnKeyCodeTyped(vgui2::KeyCode code)
{
	if (code == vgui2::KEY_ENTER || code == vgui2::KEY_PAD_ENTER)
		PostMessage(GetParent(), new KeyValues("ChatSend"));
	else if (code == vgui2::KEY_ESCAPE)
		PostMessage(GetParent(), new KeyValues("ChatCancel"));
	else if (code == vgui2::KEY_PAGEUP || code == vgui2::KEY_PAGEDOWN)
		PostMessage(GetParent(), new KeyValues("ChatScroll", "down", code == vgui2::KEY_PAGEDOWN));
	else if (code != vgui2::KEY_TAB)
		BaseClass::OnKeyCodeTyped(code);
}

CChatPrompt::CChatPrompt(vgui2::Panel *pParent, const char *pszName) : BaseClass(pParent, pszName, "")
{
	SetPaintBackgroundEnabled(true);
}

void CChatPrompt::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_hFont = GetChatFont(pScheme);

	SetFont(m_hFont);
	SetBgColor(pScheme->GetColor("Label.BgColor", Color(CHAT_FIELD_BG_COLOR)));
	SetFgColor(pScheme->GetColor("Label.TextColor", Color(CHAT_TEXT_COLOR)));
	SetContentAlignment(vgui2::Label::a_west);
	SetTextInset(CHAT_PROMPT_INSET, 0);
}

CChatInput::CChatInput(vgui2::Panel *pParent, CChat *pChat) : BaseClass(pParent, "MSChatInput")
{
	m_pChat = pChat;
	m_pPrompt = new CChatPrompt(this, "Prompt");
	m_pEntry = new CChatEntry(this, "Entry");

	MakePopup(false);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	SetVisible(false);
}

void CChatInput::SetPrompt(const char *pszPrompt)
{
	m_pPrompt->SetText(pszPrompt);
	InvalidateLayout();
}

void CChatInput::SetFieldAlpha(int iAlpha)
{
	m_pPrompt->SetAlpha(iAlpha);
	m_pEntry->SetAlpha(iAlpha);
}

void CChatInput::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBackgroundEnabled(false);
}

void CChatInput::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide, tall, promptWide, promptTall;
	GetSize(wide, tall);
	m_pPrompt->GetContentSize(promptWide, promptTall);

	m_pPrompt->SetBounds(0, 0, promptWide, tall);
	m_pEntry->SetBounds(promptWide + CHAT_PROMPT_GAP, 0, wide - promptWide - CHAT_PROMPT_GAP, tall);
}

void CChatInput::OnChatSend()
{
	m_pChat->Send();
	m_pChat->Close();
}

void CChatInput::OnChatCancel()
{
	m_pChat->Close();
}

void CChatInput::OnChatScroll(int down)
{
	m_pChat->Scroll(down != 0);
}

CChatHistory::CChatHistory(vgui2::Panel *pParent) : BaseClass(pParent, "MSChatHistory")
{
	SetMaximumCharCount(MAX_HISTORY_CHARS);
	SetVerticalScrollbar(false);
	SetPanelInteractive(false);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	SetVisible(false);
}

void CChatHistory::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_hFont = GetChatFont(pScheme);

	SetFont(m_hFont);
	m_iFontTall = vgui2::surface()->GetFontTall(m_hFont);

	m_OpenBgColor = pScheme->GetColor("ChatBgColor", Color(CHAT_BG_COLOR));
	SetBgColor(Color(m_OpenBgColor.r(), m_OpenBgColor.g(), m_OpenBgColor.b(), 0));
}

// Styled here, the scrollbar's own scheme pass would undo it in ApplySchemeSettings.
// Without ChatScheme.res the game's schemes have no ScrollBar.Wide, which leaves it 0 wide.
void CChatHistory::ShowScrollBar()
{
	vgui2::IScheme *pScheme = vgui2::scheme()->GetIScheme(GetScheme());

	const char *pszWide = pScheme->GetResourceString("ScrollBar.Wide");
	int iWide = pszWide ? atoi(pszWide) : 0;
	_vertScrollBar->SetWide(iWide > 0 ? iWide : CHAT_SCROLLBAR_WIDTH);
	SetVerticalScrollbar(true);

	Color fg = pScheme->GetColor("ScrollBarButton.FgColor", Color(CHAT_SCROLLBAR_FG_COLOR));
	Color bg = pScheme->GetColor("ScrollBarButton.BgColor", Color(CHAT_SCROLLBAR_BG_COLOR));
	Color armedFg = pScheme->GetColor("ScrollBarButton.ArmedFgColor", fg);
	Color armedBg = pScheme->GetColor("ScrollBarButton.ArmedBgColor", bg);
	Color depressedFg = pScheme->GetColor("ScrollBarButton.DepressedFgColor", fg);
	Color depressedBg = pScheme->GetColor("ScrollBarButton.DepressedBgColor", bg);
	for (int i = 0; i < 2; i++)
	{
		vgui2::Button *pButton = _vertScrollBar->GetButton(i);
		pButton->SetDefaultColor(fg, bg);
		pButton->SetArmedColor(armedFg, armedBg);
		pButton->SetDepressedColor(depressedFg, depressedBg);
	}

	_vertScrollBar->GetSlider()->SetFgColor(pScheme->GetColor("ScrollBarSlider.FgColor", bg));
	_vertScrollBar->GetSlider()->SetBgColor(pScheme->GetColor("ScrollBarSlider.BgColor", Color(CHAT_SCROLLBAR_TRACK_COLOR)));

	InvalidateLayout(true);
}

// RichText only scrolls while its scrollbar is shown
void CChatHistory::Scroll(bool bDown)
{
	if (!_vertScrollBar->IsVisible())
		ShowScrollBar();

	OnMouseWheeled(bDown ? -1 : 1);
}

CChat::CChat(vgui2::Panel *pParent)
{
	m_pHistory = new CChatHistory(pParent);
	m_pInput = new CChatInput(pParent, this);

	if (vgui2::HScheme hScheme = LoadChatScheme())
	{
		SetSchemeRecursive(m_pHistory, hScheme);
		SetSchemeRecursive(m_pInput, hScheme);
	}

	m_pHistory->MakeReadyForUse();
	m_pInput->MakeReadyForUse();

	SDL_AddEventWatch(ChatEventWatch, this);
}

// SDL outlives the client dll, so the watch has to go before it unloads
void CChat::Shutdown()
{
	SDL_DelEventWatch(ChatEventWatch, this);
}

void CChat::QueueWheel(int iSteps)
{
	if (m_bOpen)
		m_iWheelSteps += iSteps;
}

void CChat::Reset()
{
	Close();
}

void CChat::OnHideClientUI()
{
	Close();
}

void CChat::Think()
{
	MSVGUI2::IHost *pHost = MSVGUI2::GetHost();
	bool bExpired = !m_bOpen && vgui2::system()->GetCurrentTime() >= m_flHideTime;
	bool bShow = pHost && pHost->ShowHUD() && (m_bOpen || (m_bHasText && !bExpired));

	if (bExpired && m_bHeld)
		ReleaseHistory(0.0f);

	for (; m_iWheelSteps > 0; m_iWheelSteps--)
		Scroll(false);
	for (; m_iWheelSteps < 0; m_iWheelSteps++)
		Scroll(true);

	if (bShow)
		Layout();

	UpdateFade();
	m_pHistory->SetVisible(bShow);
}

// History fills the box down to the input line, which sits at the bottom
void CChat::Layout()
{
	int iLineTall = m_pHistory->GetFontTall() + 2;

	int iX = ScaleX(CHAT_BOX_X + CHAT_BOX_INSET);
	int iY = ScaleY(CHAT_BOX_BOTTOM_Y - CHAT_BOX_HEIGHT + CHAT_BOX_INSET);
	int iWide = ScaleX(CHAT_BOX_WIDTH - CHAT_BOX_INSET * 2);
	int iBottom = ScaleY(CHAT_BOX_BOTTOM_Y);

	int iInputY = iBottom - (int)(iLineTall * (1.0f + CHAT_INPUT_MARGIN_BOTTOM));
	int iHistoryTall = iInputY - (int)(iLineTall * CHAT_INPUT_MARGIN_TOP) - iY;

	m_pHistory->SetBounds(iX, iY, iWide, iHistoryTall);
	m_pInput->SetBounds(iX, iInputY, iWide, iLineTall);
}

void CChat::UpdateFade()
{
	float flFrac = (float)((m_flFadeTime - vgui2::system()->GetCurrentTime()) / CHAT_FADE_TIME);
	if (flFrac < 0.0f)
		flFrac = 0.0f;
	else if (flFrac > 1.0f)
		flFrac = 1.0f;

	Color bg = m_pHistory->GetOpenBgColor();
	int iFade = (int)(bg.a() * flFrac);
	if (m_bOpen)
	{
		m_pHistory->SetBgColor(Color(bg.r(), bg.g(), bg.b(), bg.a() - iFade));
		m_pInput->SetFieldAlpha(CHAT_INPUT_ALPHA - iFade);
	}
	else
	{
		m_pHistory->SetBgColor(Color(bg.r(), bg.g(), bg.b(), iFade));
	}
}

float CChat::GetDecayTime()
{
	MSVGUI2::IHost *pHost = MSVGUI2::GetHost();
	float flDecay = pHost ? pHost->GetCvarFloat("ms_txthud_decaytime") : DEFAULT_DECAY_TIME;

	return flDecay < 1.0f ? 1.0f : flDecay;
}

// Keeps every line opaque, for while the box is open or being scrolled
void CChat::HoldHistory()
{
	m_pHistory->ResetAllFades(true);
	m_bHeld = true;
}

void CChat::ReleaseHistory(float flSustain)
{
	m_pHistory->SetVerticalScrollbar(false);
	m_pHistory->GotoTextEnd();
	m_pHistory->ResetAllFades(false, true, flSustain);
	m_bHeld = false;
}

void CChat::Open(int iType, const char *pszPrompt)
{
	if (!MSVGUI2::GetHost())
		return;

	m_iType = iType;
	m_bOpen = true;
	m_flFadeTime = vgui2::system()->GetCurrentTime() + CHAT_FADE_TIME;

	m_pInput->SetPrompt(pszPrompt);
	m_pInput->GetEntry()->SetText("");
	m_pInput->SetFieldAlpha(CHAT_INPUT_ALPHA - m_pHistory->GetOpenBgColor().a());
	Layout();
	m_pInput->SetVisible(true);
	m_pInput->SetKeyBoardInputEnabled(true);
	m_pInput->SetMouseInputEnabled(INPUT_USES_MOUSE);
	m_pInput->MoveToFront();
	m_pInput->GetEntry()->RequestFocus();

	HoldHistory();
	m_pHistory->ShowScrollBar();
}

void CChat::Close()
{
	if (!m_bOpen)
		return;

	double flNow = vgui2::system()->GetCurrentTime();
	float flDecay = GetDecayTime();

	m_bOpen = false;
	m_iWheelSteps = 0;
	m_flCloseTime = flNow;
	m_flFadeTime = flNow + CHAT_FADE_TIME;
	m_flHideTime = flNow + flDecay + CHAT_LINE_FADE_TIME;

	m_pInput->SetKeyBoardInputEnabled(false);
	m_pInput->SetMouseInputEnabled(false);
	m_pInput->SetVisible(false);

	ReleaseHistory(flDecay);
}

void CChat::Send()
{
	MSVGUI2::IHost *pHost = MSVGUI2::GetHost();
	if (!m_bOpen || !pHost)
		return;

	wchar_t wszText[MAX_CHAT_CHARS + 1];
	m_pInput->GetEntry()->GetText(wszText, sizeof(wszText));

	char szText[MAX_CHAT_CHARS * 4 + 1];
	Q_UnicodeToUTF8(wszText, szText, sizeof(szText), STRINGCONVERT_REPLACE);

	SanitizeChat(szText);
	TruncateUtf8(szText, MAX_CHAT_BYTES);

	if (szText[0])
		pHost->SendChat(m_iType, szText);
}

// Binds must not fire for the key press that closed the box either
bool CChat::BlocksKeys()
{
	if (m_bOpen)
		return true;

	return m_flCloseTime >= 0.0 && vgui2::system()->GetCurrentTime() - m_flCloseTime < KEY_BLOCK_AFTER_CLOSE;
}

void CChat::Print(int r, int g, int b, const char *pszUtf8)
{
	if (!pszUtf8 || !pszUtf8[0])
		return;

	wchar_t wszText[1024];
	Q_UTF8ToUnicode(pszUtf8, wszText, sizeof(wszText), STRINGCONVERT_REPLACE);

	int iLen = 0;
	while (wszText[iLen])
		iLen++;

	while (iLen > 0 && (wszText[iLen - 1] == '\n' || wszText[iLen - 1] == '\r'))
		wszText[--iLen] = 0;

	if (!iLen)
		return;

	if (m_bHasText)
		m_pHistory->InsertString("\n");

	float flDecay = GetDecayTime();

	m_pHistory->InsertColorChange(Color(r, g, b, 255));
	m_pHistory->InsertString(wszText);
	m_pHistory->InsertFade(flDecay, CHAT_LINE_FADE_TIME);

	if (m_bOpen)
		HoldHistory();

	m_bHasText = true;
	m_flHideTime = vgui2::system()->GetCurrentTime() + flDecay + CHAT_LINE_FADE_TIME;
}

void CChat::Scroll(bool bDown)
{
	if (!m_bOpen || !m_bHasText)
		return;

	HoldHistory();
	m_pHistory->Scroll(bDown);
}
