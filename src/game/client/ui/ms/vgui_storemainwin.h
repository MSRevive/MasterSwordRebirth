#ifndef MS_STORE_MAINWIN
#define MS_STORE_MAINWIN

#include "vgui_container.h"

#define TITLE_X	ITEM_CONTAINER_X
#define TITLE_Y	0

#define SELLLABEL_X	ITEM_CONTAINER_X 
#define SELLLABEL_Y ITEM_CONTAINER_Y + ITEM_CONTAINER_SIZE_Y
#define SELLLABEL_W ITEM_CONTAINER_SIZE_X - ACTBTN_SIZE_X
#define SELLLABEL_H YRES(30)

class CStorePanel : public VGUI_ContainerPanel
{
protected:
	MSLabel *m_SaleLabel;

public:
	CStorePanel();

	virtual void Update(void);
	virtual void Close(void);

	static msstring StoreVendorName;
	static mslist<storeitem_t> StoreItems;
	static unsigned long StoreGold;
	static int iStoreBuyFlags, StoreItemMsgCount;
	static msstring Text_StoreGold;
};

#endif //MS_STORE_MAINWIN
