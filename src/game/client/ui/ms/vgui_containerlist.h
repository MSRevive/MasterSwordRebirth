#include "vgui_storemainwin.h"

class ContainerButton;
#define CONTAINER_INFO_LABELS 1
#define MAX_CONTAINTER_ITEMS 128
#define MAX_NPCHANDS 3

class CContainerPanel : public VGUI_ContainerPanel
{
public:
	ulong m_OpenContainerID;

	CContainerPanel(int iTrans, int iRemoveMe, int x, int y, int wide, int tall);
	void UpdateSubtitle();
	void RemoveGear();

	virtual void Open(void);
	virtual void Close(void);

	//From VGUI_ItemCallbackPanel
	void ItemSelectChanged(ulong ID, bool fSelected);
	void GearItemSelected(ulong ID);
	bool GearItemClicked(ulong ID);
	bool GearItemDoubleClicked(ulong ID);
	void ItemDoubleclicked(ulong ID);

	void StepInput(bool bDirUp); // MIB FEB2015_21 [INV_SCROLL]
};