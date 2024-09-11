#include "vgui_storemainwin.h"
enum
{
	OPT_PARTY,
	OPT_VOTEKICK,
	OPT_VOTETIME,
	OPT_CANCEL,
	OPT_MAX
};
#define MAX_MAINBUTTONS OPT_MAX

static COLOR Color_BtnArmed(255, 0, 0, 0), Color_BtnUnarmed(255, 178, 0, 0), Color_BtnDisabled(128, 128, 128, 0);
static COLOR Color_Border(100, 140, 100, 255);
static COLOR BtnColor(128, 0, 0, 0), BtnTextColor(128, 0, 0, 0), DisabledColor(128, 128, 128, 0);

class CAction_SelectMainOption : public ActionSignal
{
protected:
	class VGUI_MenuBase *m_pPanel;
	int m_Value;
	msvariant m_Data;

public:
	CAction_SelectMainOption(class VGUI_MenuBase *pPanel, int iValue, msvariant Data);
	virtual void actionPerformed(Panel *panel);
};

class VGUI_MenuBase : public CMenuPanel
{
public:
	MSLabel *m_Title;
	//MSButton *m_pButton[MAX_MAINBUTTONS];
	mslist<Panel *> m_FadePanels;
	int m_FadeAmt;
	float m_OpenTime;

	int m_ButtonY;
	mslist<MSButton *> m_Buttons;
	mslist<CAction_SelectMainOption *> m_Actions;
	bool m_AllowKeys; // MiB NOV2014_25, disable number shortcuts: NpcInteractMenus.rft

	CTransparentPanel *m_pMainPanel;
	CTransparentPanel *m_TitleSep; // MiB NOV2014_25, center the title and separator NpcInteractMenus.rft
	LineBorder *m_Border, *m_Spacer;

	VGUI_MenuBase(Panel *myParent);
	virtual void UpdateFade(void);
	virtual void Init();
	virtual MSButton *AddButton(msstring_ref Name, int Width, msvariant ID);
	virtual void Select(int BtnIdx, msvariant &Data){};

	virtual bool SlotInput(int iSlot);
	virtual void Open(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Think() { UpdateFade(); }
	virtual void Initialize(void);
};
