class VGUI
{
public:
	static bool ShowMenu(const char* Name);
	static bool HideMenu(VGUI_MainPanel *pPanel);
	static bool ToggleMenuVisible(const char* Name);
	static VGUI_MainPanel *FindPanel(const char* Name);
};