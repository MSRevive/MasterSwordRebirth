#pragma once

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/TextEntry.h>

#include "vgui2/IViewportPanel.h"

class CChat;

class CChatEntry : public vgui2::TextEntry
{
	DECLARE_CLASS_SIMPLE(CChatEntry, vgui2::TextEntry);

public:
	CChatEntry(vgui2::Panel *pParent, const char *pszName);

protected:
	void ApplySchemeSettings(vgui2::IScheme *pScheme) override;
	void OnKeyCodeTyped(vgui2::KeyCode code) override;

private:
	vgui2::HFont m_hFont = vgui2::INVALID_FONT;
};

class CChatPrompt : public vgui2::Label
{
	DECLARE_CLASS_SIMPLE(CChatPrompt, vgui2::Label);

public:
	CChatPrompt(vgui2::Panel *pParent, const char *pszName);

protected:
	void ApplySchemeSettings(vgui2::IScheme *pScheme) override;

private:
	vgui2::HFont m_hFont = vgui2::INVALID_FONT;
};

// EditablePanel, not Panel: it is what hands key focus to the entry
class CChatInput : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CChatInput, vgui2::EditablePanel);

public:
	CChatInput(vgui2::Panel *pParent, CChat *pChat);

	void SetPrompt(const char *pszPrompt);
	void SetFieldAlpha(int iAlpha);
	CChatEntry *GetEntry() { return m_pEntry; }

protected:
	void ApplySchemeSettings(vgui2::IScheme *pScheme) override;
	void PerformLayout() override;

	MESSAGE_FUNC(OnChatSend, "ChatSend");
	MESSAGE_FUNC(OnChatCancel, "ChatCancel");
	MESSAGE_FUNC_INT(OnChatScroll, "ChatScroll", down);

private:
	CChat *m_pChat;
	CChatPrompt *m_pPrompt;
	CChatEntry *m_pEntry;
};

class CChatHistory : public vgui2::RichText
{
	DECLARE_CLASS_SIMPLE(CChatHistory, vgui2::RichText);

public:
	CChatHistory(vgui2::Panel *pParent);

	void ShowScrollBar();
	void Scroll(bool bDown);
	int GetFontTall() { return m_iFontTall; }
	vgui2::HFont GetFont() { return m_hFont; }

protected:
	void ApplySchemeSettings(vgui2::IScheme *pScheme) override;

private:
	vgui2::HFont m_hFont = vgui2::INVALID_FONT;
	int m_iFontTall = 0;
};

// Chat input line and history. Not a panel itself, it owns the two that are.
class CChat : public IViewportPanel
{
public:
	CChat(vgui2::Panel *pParent);

	const char *GetName() override { return "chat"; }
	void Reset() override;
	void Think() override;
	void OnHideClientUI() override;

	void Shutdown();
	void QueueWheel(int iSteps);
	void Open(int iType, const char *pszPrompt);
	void Close();
	void Send();
	bool IsOpen() { return m_bOpen; }
	bool BlocksKeys();
	void Print(int r, int g, int b, const char *pszUtf8);
	void Scroll(bool bDown);

private:
	float GetDecayTime();
	void Layout();
	void UpdateFade();
	void HoldHistory();
	void ReleaseHistory(float flSustain);

	CChatInput *m_pInput;
	CChatHistory *m_pHistory;
	vgui2::HFont m_hFont = vgui2::INVALID_FONT;

	int m_iType = 0;
	int m_iWheelSteps = 0;
	bool m_bOpen = false;
	bool m_bHasText = false;
	bool m_bHeld = false;
	double m_flCloseTime = -1.0;
	double m_flHideTime = 0.0;
	double m_flFadeTime = 0.0;
};
