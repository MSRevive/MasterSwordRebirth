#ifndef HUD_MAGIC_H
#define HUD_MAGIC_H

#include "hudbase.h"

class CHudMagic : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void Think(void);
	void Reset(void);
	int MsgFunc_Spells(const char *pszName, int iSize, void *pbuf);
	//void UserCmd_ListSpells( void );
	void SelectMenuItem(int menu_item);
	int SpellsMemorized(void);
	void InitHUDData(void);

	int SpellMenuIndex[256];
};
#endif //HUD_MAGIC_H
