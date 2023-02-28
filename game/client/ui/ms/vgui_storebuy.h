#include "vgui_storemainwin.h"

class CStoreBuyPanel : public CStorePanel
{
public:
	static const char *Text_BuySubtitle;
	static const char *Text_InvSubtitle;

	CStoreBuyPanel(int iTrans, int iRemoveMe, int x, int y, int wide, int tall);

	void AddInventoryItems();

	//Callbacks
	void ItemHighlighted(void *pData);
	bool ItemClicked(void *pData);
};

void MsgFunc_Store();
