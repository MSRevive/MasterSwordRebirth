#include <string.h>

#include <tier0/dbg.h>
#include <tier1/strtools.h>
#include <tier1/KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ScrollBar.h>

#include "CChatPanel.h"
#include "ms_vgui2.h"

namespace
{
	// The server relays chat in a 192 byte user message, so this stays small
	const int MAX_CHAT_BYTES = 120;
	const int MAX_CHAT_CHARS = 120;
	const int MAX_HISTORY_CHARS = 127 * 100;
	const int MAX_HISTORY_LINES = 32;
	const int HISTORY_PADDING = 4;
	const int INPUT_PADDING = 4;
	const float FADE_LENGTH = 1.0f;
	const float DEFAULT_DECAY_TIME = 10.0f;
	const double KEY_BLOCK_AFTER_CLOSE = 0.05;

	// Enable if the engine won't route keys to a keyboard-only popup
	const bool INPUT_USES_MOUSE = false;

	vgui2::HFont GetChatFont(vgui2::IScheme *pScheme)
	{
		vgui2::HFont hFont = pScheme->GetFont("ChatFont", false);
		if (hFont == vgui2::INVALID_FONT)
			hFont = pScheme->GetFont("Default", false);

		return hFont;
	}

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

	int Clamp(int iValue, int iMin, int iMax)
	{
		if (iValue < iMin)
			return iMin;

		return iValue > iMax ? iMax : iValue;
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

	// The game's TrackerScheme has none of the TextEntry keys, which leaves everything white
	SetFont(GetChatFont(pScheme));
	SetBgColor(Color(0, 0, 0, 0));
	SetFgColor(Color(255, 255, 255, 255));
	SetBorder(nullptr);
}

// Posted rather than called, so the panel isn't hidden in the middle of its own key event
void CChatEntry::OnKeyCodeTyped(vgui2::KeyCode code)
{
	if (code == vgui2::KEY_ENTER || code == vgui2::KEY_PAD_ENTER)
		PostMessage(GetParent(), new KeyValues("ChatSend"));
	else if (code == vgui2::KEY_ESCAPE)
		PostMessage(GetParent(), new KeyValues("ChatCancel"));
	else if (code != vgui2::KEY_TAB)
		BaseClass::OnKeyCodeTyped(code);
}

CChatPrompt::CChatPrompt(vgui2::Panel *pParent, const char *pszName) : BaseClass(pParent, pszName, "")
{
	SetPaintBackgroundEnabled(false);
}

void CChatPrompt::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetFont(GetChatFont(pScheme));
	SetFgColor(Color(255, 255, 255, 255));
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

void CChatInput::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_iFontTall = vgui2::surface()->GetFontTall(GetChatFont(pScheme));

	SetBgColor(Color(0, 0, 0, 128));
	SetPaintBackgroundEnabled(true);
}

void CChatInput::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide, tall, promptWide, promptTall;
	GetSize(wide, tall);
	m_pPrompt->GetContentSize(promptWide, promptTall);

	m_pPrompt->SetBounds(INPUT_PADDING, 0, promptWide, tall);
	m_pEntry->SetBounds(promptWide + INPUT_PADDING * 2, 0, wide - promptWide - INPUT_PADDING * 3, tall);
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

	vgui2::HFont hFont = GetChatFont(pScheme);
	SetFont(hFont);
	SetBgColor(Color(0, 0, 20, 127));
	m_iFontTall = vgui2::surface()->GetFontTall(hFont);
}

// RichText only scrolls while its scrollbar is shown
void CChatHistory::Scroll(bool bDown)
{
	if (!_vertScrollBar->IsVisible())
	{
		SetVerticalScrollbar(true);
		InvalidateLayout(true);
	}

	OnMouseWheeled(bDown ? -1 : 1);
}

CChat::CChat(vgui2::Panel *pParent)
{
	m_pHistory = new CChatHistory(pParent);
	m_pInput = new CChatInput(pParent, this);

	m_pHistory->MakeReadyForUse();
	m_pInput->MakeReadyForUse();
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
	bool bShow = pHost && pHost->ShowHUD() && m_bHasText && !bExpired;

	if (bExpired && m_bHeld)
		ReleaseHistory(0.0f);

	if (bShow)
		LayoutHistory();

	m_pHistory->SetVisible(bShow);
}

void CChat::LayoutHistory()
{
	MSVGUI2::IHost *pHost = MSVGUI2::GetHost();
	MSVGUI2::ChatLayout Layout;
	pHost->GetChatLayout(Layout);

	// Grows upwards from its bottom edge like the old console did
	int iMaxLines = Clamp((int)pHost->GetCvarFloat("ms_txthud_size"), 1, MAX_HISTORY_LINES);
	int iLines = Clamp(m_pHistory->GetNumLines(), 1, iMaxLines);
	int iTall = iLines * (m_pHistory->GetFontTall() + 1) + HISTORY_PADDING;
	m_pHistory->SetBounds(Layout.HistoryX, Layout.HistoryBottomY - iTall, Layout.HistoryWidth, iTall);

	float flTrans = pHost->GetCvarFloat("ms_txthud_bgtrans");
	if (flTrans < -1.0f)
		flTrans = -1.0f;
	else if (flTrans > 1.0f)
		flTrans = 1.0f;

	m_pHistory->SetBgColor(Color(0, 0, 20, (int)(127.0f - flTrans * 127.0f)));
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
	MSVGUI2::IHost *pHost = MSVGUI2::GetHost();
	if (!pHost)
		return;

	MSVGUI2::ChatLayout Layout;
	pHost->GetChatLayout(Layout);

	int iTall = Layout.InputHeight;
	if (iTall < m_pInput->GetFontTall() + INPUT_PADDING * 2)
		iTall = m_pInput->GetFontTall() + INPUT_PADDING * 2;

	m_iType = iType;
	m_bOpen = true;

	m_pInput->SetPrompt(pszPrompt);
	m_pInput->GetEntry()->SetText("");
	m_pInput->SetBounds(Layout.InputX, Layout.InputY, Layout.InputWidth, iTall);
	m_pInput->SetVisible(true);
	m_pInput->SetKeyBoardInputEnabled(true);
	m_pInput->SetMouseInputEnabled(INPUT_USES_MOUSE);
	m_pInput->MoveToFront();
	m_pInput->GetEntry()->RequestFocus();

	HoldHistory();
}

void CChat::Close()
{
	if (!m_bOpen)
		return;

	double flNow = vgui2::system()->GetCurrentTime();
	float flDecay = GetDecayTime();

	m_bOpen = false;
	m_flCloseTime = flNow;
	m_flHideTime = flNow + flDecay + FADE_LENGTH;

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
	m_pHistory->InsertFade(flDecay, FADE_LENGTH);

	if (m_bOpen)
		HoldHistory();

	m_bHasText = true;
	m_flHideTime = vgui2::system()->GetCurrentTime() + flDecay + FADE_LENGTH;
}

void CChat::Scroll(bool bDown)
{
	if (!m_bHasText)
		return;

	HoldHistory();
	m_pHistory->Scroll(bDown);

	if (!m_bOpen)
		m_flHideTime = vgui2::system()->GetCurrentTime() + GetDecayTime() + FADE_LENGTH;
}
