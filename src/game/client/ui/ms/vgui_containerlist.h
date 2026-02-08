#include "vgui_storemainwin.h"
#include "vgui_moveitempanel.h"

class ContainerButton;
#define CONTAINER_INFO_LABELS 1
#define MAX_CONTAINTER_ITEMS 128
#define MAX_NPCHANDS 3

class CContainerPanel : public VGUI_ContainerPanel
{
private:
	VGUI_MoveItemPanel *mpMoveItemPanel;

public:
	ulong m_OpenContainerID;

	CContainerPanel(int iTrans, int iRemoveMe, int x, int y, int wide, int tall);
	void UpdateSubtitle();
	void RemoveGear();
	void DropAllSelected();

	virtual void Open(void);
	virtual void Close(void);

	//From VGUI_ItemCallbackPanel
	void ItemSelectChanged(ulong ID, bool fSelected);
	void GearItemSelected(ulong ID);
	bool GearItemClicked(ulong ID);
	bool GearItemDoubleClicked(ulong ID);
	void ItemDoubleclicked(ulong ID);

	virtual void ItemRightClicked(void *pData);
	virtual void MoveItem(VGUI_ItemButton *pButton, int vNumMove);
	virtual bool ItemClicked(void *pData);
	virtual void InvTypeChanged(int vInvType);
	virtual void AlphabeticChanged(bool bAlphabetic);

	void ActionPerformed();

	void StepInput(bool bDirUp); // MIB FEB2015_21 [INV_SCROLL]
};