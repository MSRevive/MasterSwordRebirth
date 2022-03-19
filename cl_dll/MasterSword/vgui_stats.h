#include "vgui_mscontrols.h"
#include "stats\statdefs.h"

class CMenuHandler_PutInPack;
#define CONTAINER_INFO_LABELS 1
#define GENDERPANEL_MAINBTNS 2
#define INFO_STAT_NUM 6

class CStatPanel : public CMenuPanel
{
private:
	ActionSignal *Gender_MainActionSig[GENDERPANEL_MAINBTNS];
	TextPanel *Info_StatLabel[INFO_STAT_NUM],
		*Nat_StatLabel[NATURAL_MAX_STATS],
		*Skill_StatLabel[SKILL_MAX_STATS + 1];
	MSLabel *m_SkillInfoLabel, *m_StatTypeLabel[STAT_MAGIC_TOTAL];
	CTransparentPanel *pMainPanel, *m_InfoPanel;
	ScrollPanel *m_pScrollPanel;

public:
	CStatPanel(Panel *pParent);

	virtual bool SlotInput(int iSlot);
	virtual void Open(void);
	virtual void Close(void);
	virtual void Update(void);
	virtual void Initialize(void);
	virtual void StepInput(hudscroll_e ScrollCmd);

	int m_ActiveStat;
};
