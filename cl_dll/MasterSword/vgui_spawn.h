#include "vgui_mscontrols.h"

class CSpawnPanel : public CMenuPanel
{
private:
	CTransparentPanel *m_GenderPanel, *m_ClassPanel;

	TextPanel *m_Message;
	Label *m_pTitle, *m_pNameLabel;

public:
	CStatusBar *pStatus;
	CSpawnPanel(Panel *pParent);

	virtual bool SlotInput(int iSlot);
	virtual void Open(void);
	virtual void Close(void);
	virtual void Update(void);
	virtual void SetActiveInfo(int iInput);
	virtual void Initialize(void);
};
