#pragma once

// The only VGUI2 header the rest of the client includes.

namespace MSVGUI2
{
	class IHost
	{
	public:
		virtual void SendChat(int Type, const char *Utf8Text) = 0;
		virtual float GetCvarFloat(const char *Name) = 0;
		virtual bool ShowHUD() = 0;
	};

	void SetHost(IHost *pHost);
	IHost *GetHost();

	void *GetClientVGUIInterface();
	void VidInit();
	void Shutdown();

	void Chat_Open(int Type, const char *Prompt);
	void Chat_Close();
	bool Chat_IsOpen();
	bool Chat_BlocksKeys();
	void Chat_Print(int r, int g, int b, const char *Utf8Text);
	void Chat_Scroll(bool Down);
}
