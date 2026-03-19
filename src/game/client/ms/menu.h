//Master Sword
#include "sharedmenu.h"

constexpr int MAX_MENU_STRING = 512;

enum MenuType
{ //menutypes
	MENU_NORMAL,
	MENU_LISTSPELLS,
	MENU_ACTIONS,
	MENU_REMOVEPACK,
	MENU_OFFER,
	MENU_DYNAMICNPC,
};
//------------

class CHudMenu : public CHudBase
{
public:
	int Init(void);
	void InitHUDData(void);
	int VidInit(void);
	void Reset(void);
	int Draw(float flTime);
	int MsgFunc_ShowMenu(const char *pszName, int iSize, void *pbuf);

	void SelectMenuItem(int menu_item);

	int m_fMenuDisplayed;
	int m_bitsValidSlots;
	float m_flShutoffTime;
	int m_fWaitingForMore;
	bool m_fOfferedNextMenu;
	char Name[32] = "CHudMenu";

	//Master Sword
	TCallbackMenu m;

	int ShowMenu(int bitsValidSlots, const char *pszMenuString, MenuCallback CallBack, MenuType mtMenuType = MenuType::MENU_NORMAL, float ShutOffTime = 0);
	bool HideMyMenu(int mtMenuType);
	//------------
};
