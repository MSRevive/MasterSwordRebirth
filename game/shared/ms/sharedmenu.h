#ifndef SHAREDMENU_H
#define SHAREDMENU_H

#ifdef VALVE_DLL
typedef void (CBaseEntity::*MenuCallback)(CBasePlayer *pPlayer, int slot, struct TCallbackMenu *pcbMenu);
#else
typedef void (*MenuCallback)(int idx, struct TCallbackMenu *pcbMenu); // The idx is offset back to starting at 0
#endif

struct TCallbackMenu
{
	CHAR cMenuText[2048];
	int iValidslots;
	MenuCallback m_MenuCallback;
	int m_MenuType;

#ifdef VALVE_DLL
	entvars_t *pevOwner;
	void *vData; //User defined data
#endif
};

#endif