class VGUI
{
public:
	static bool ShowMenu(msstring_ref Name);
	static bool HideMenu(VGUI_MainPanel *pPanel);
	static bool ToggleMenuVisible(msstring_ref Name);
	static VGUI_MainPanel *FindPanel(msstring_ref Name);
};