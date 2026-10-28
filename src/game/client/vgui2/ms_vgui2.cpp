#include "ms_vgui2.h"
#include "CClientViewport.h"
#include "chat/CChatPanel.h"

static MSVGUI2::IHost *s_pHost = nullptr;

static CChat *GetChat()
{
	return g_pViewport ? g_pViewport->GetChat() : nullptr;
}

void MSVGUI2::SetHost(IHost *pHost)
{
	s_pHost = pHost;
}

MSVGUI2::IHost *MSVGUI2::GetHost()
{
	return s_pHost;
}

void MSVGUI2::VidInit()
{
	if (g_pViewport)
		g_pViewport->Reset();
}

// The engine has already unloaded vgui2.dll by HUD_Shutdown. The panels are
// leaked on purpose, their destructors would call into it.
void MSVGUI2::Shutdown()
{
	if (CChat *pChat = GetChat())
		pChat->Shutdown();

	g_pViewport = nullptr;
	s_pHost = nullptr;
}

void MSVGUI2::Chat_Open(int Type, const char *Prompt)
{
	if (CChat *pChat = GetChat())
		pChat->Open(Type, Prompt);
}

void MSVGUI2::Chat_Close()
{
	if (CChat *pChat = GetChat())
		pChat->Close();
}

bool MSVGUI2::Chat_IsOpen()
{
	CChat *pChat = GetChat();
	return pChat && pChat->IsOpen();
}

bool MSVGUI2::Chat_BlocksKeys()
{
	CChat *pChat = GetChat();
	return pChat && pChat->BlocksKeys();
}

void MSVGUI2::Chat_Print(int r, int g, int b, const char *Utf8Text)
{
	if (CChat *pChat = GetChat())
		pChat->Print(r, g, b, Utf8Text);
}
