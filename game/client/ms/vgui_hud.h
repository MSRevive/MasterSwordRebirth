#ifndef VALVE_DLL

#include "vgui_color.h"

class VGUI_MainPanel *CreateHUDPanel(class Panel *pParent);
class VGUI_MainPanel *CreateHUD_MenuMain(class Panel *pParent);
class VGUI_MainPanel *CreateHUD_MenuInteract(class Panel *pParent);

void VGUI_AddMenuOption();
void VGUI_ShowMenuInteract();

void HUD_ShowInfoWin(msstring_ref Title, msstring_ref Text);
void HUD_ShowHelpWin(msstring_ref Title, msstring_ref Text);
void HUD_PrintEvent(vgui::Color color, msstring_ref Text);
void HUD_SayTextEvent(vgui::Color color, msstring_ref Text);
#ifdef TEAMFORTRESSVIEWPORT_H
void HUD_StepInput(hudscroll_e ScrollCmd);
#endif
bool HUD_KeyInput(int down, int keynum, const char *pszCurrentBinding);
void HUD_StartSayText(int Type);
void HUD_CancelSayText();
void HUD_DrawID(struct entinfo_t *pEntinfo);
void HUD_NewLevel();
bool QuickSlotConfirm();

extern COLOR Color_Transparent, Color_Text_White;

#define CVAR_HELPTIPS "ms_help"

class IHUD_Interface
{
public:
	virtual void Init() {}
	virtual void Reset() {}									//New level
	virtual void Update() {}								//Every frame
	virtual void SetVisible(bool fVisible) {}				//Show/Hide
	virtual void Close() {}									//Close
	virtual IHUD_Interface *GetInterface() { return this; } //Get pointer to this interface
};

#endif

enum saytext_e
{
	SAYTEXT_GLOBAL,
	SAYTEXT_LOCAL,
	SAYTEXT_PARTY,
	SAYTEXT_NPC,
};
