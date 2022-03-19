#ifndef MS_STORE_MAINWIN
#define MS_STORE_MAINWIN

#include "vgui_container.h"

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
	static string_i Text_StoreGold;
};

#endif //MS_STORE_MAINWIN
