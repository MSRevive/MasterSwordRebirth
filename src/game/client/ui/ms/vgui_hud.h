#pragma once

#include <VGUI_Color.h>

class VGUI_MainPanel *CreateHUDPanel(class Panel *pParent);
class VGUI_MainPanel *CreateHUD_MenuMain(class Panel *pParent);
class VGUI_MainPanel *CreateHUD_MenuInteract(class Panel *pParent);

void VGUI_AddMenuOption();
void VGUI_ShowMenuInteract();

void HUD_ShowInfoWin(const char* Title, const char* Text);
void HUD_ShowHelpWin(const char* Title, const char* Text);
void HUD_PrintEvent(vgui::Color color, const char* Text);
void HUD_SayTextEvent(vgui::Color color, const char* Text);
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