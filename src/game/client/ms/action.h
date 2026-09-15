#ifndef HUD_ACTION_H
#define HUD_ACTION_H

#include "hudbase.h"

struct playeraction_t
{
	msstring Name;
	msstring ID;
};

class CHudAction : public CHudBase
{
public:
	int Init(void);
	int MsgFunc_Action(const char *pszName, int iSize, void *pbuf);
	void UserCmd_Action(void);
	void SelectMenuItem(int menu_item);

	mslist<playeraction_t> PlayerActions;
};
#endif //HUD_ACTION_H
