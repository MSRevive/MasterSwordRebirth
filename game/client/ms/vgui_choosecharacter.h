#ifdef TEAMFORTRESSVIEWPORT_H
#include "vgui_storemainwin.h"
#include "clrender.h"

class CMenuHandler_PutInPack;
#define CONTAINER_INFO_LABELS 1
#define MAX_CONTAINTER_ITEMS 128
#define MAX_NPCHANDS 3
#define CHOOSEPANEL_MAINBTNS 3
#define GENDERPANEL_MAINBTNS 2
#define WEAPONPANEL_MAINBTNMAX 9
#define WEAPONPANEL_MAINBTNS (signed)(MSGlobals::DefaultWeapons.size())

enum stage_e
{
	STG_FIRST,
	STG_CHOOSECHAR = STG_FIRST,
	STG_CHOOSEGENDER,
	STG_CHOOSEWEAPON,
	STG_LAST = STG_CHOOSEWEAPON
};

class CRenderChar : public CRenderPlayer
{
public:
	void Init(int Idx);
	void Init(int Idx, msstring model); // MIB FEB2015_21 [RACE_MENU] - New model-specified init
	void Render();
	void UnRegister()
	{
		m_Done = true;
		CRenderEntity::UnRegister();
	}
	void Highlight(bool Highlighted);
	void Select();
	void SetActive(bool Active);
	void PlayAttnAnim();
	~CRenderChar();

	int m_Idx;
	int m_zAdj;
	stage_e m_Stage;
	float m_TimeRandomIdle;
	bool m_Active;
	bool m_Highlighted;
	bool m_ReturnToAttention;
	bool m_Done;
	bool m_ItemInHand;
	mslist<CGenericItem> m_GearItems;
	enum rendercharstate_e
	{
		RCS_IDLE,
		RCS_FIDGET,
		RCS_HIGHLIGHT,
		RCS_INACTIVE
	} m_AnimState;
};
class CRenderSpawnbox : public CRenderEntity
{
public:
	void Init();
	void Render();
	void UnRegister()
	{
		m_Done = true;
		CRenderEntity::UnRegister();
	}
	void CB_UnRegistered() {}
	bool m_Done;
	~CRenderSpawnbox();
};

class CNewCharacterPanel : public CMenuPanel
{
public:
	CTFScrollPanel *m_pScrollPanel;
	Label *m_pTitleLabel;
	MSButton *m_pCancelButton;

	CTransparentPanel *m_ChoosePanel,
		*m_GenderPanel,
		*m_WeaponPanel;

	MSButton *m_BackBtn;

	//CImageLabel			*Choose_Image[3];
	CTransparentPanel *Choose_ImageCover[3];

	Label *Choose_MainLabel;
	MSLabel *Choose_CharHandlingLabel;
	MSButton *Choose_MainBtn[CHOOSEPANEL_MAINBTNS];
	ActionSignal *Choose_MainActionSig[CHOOSEPANEL_MAINBTNS];
	InputSignal *Choose_MainInputSig[CHOOSEPANEL_MAINBTNS];
	Label *Choose_CharLabel[CHOOSEPANEL_MAINBTNS][2];
	MSButton *Choose_DeleteChar[CHOOSEPANEL_MAINBTNS];
	ActionSignal *Choose_DelActionSig[CHOOSEPANEL_MAINBTNS];
	MSLabel *Choose_UploadStatus;

	TextPanel *Gender_MainLabel;
	Label *Gender_NameLabel;
	Label *Gender_GenderLabel;
	MSButton *Gender_MainBtn[GENDERPANEL_MAINBTNS];
	ActionSignal *Gender_MainActionSig[GENDERPANEL_MAINBTNS];
	Label *Gender_BtnLabel[GENDERPANEL_MAINBTNS];
	VGUI_TextPanel *Gender_NameTextPanel;
	msstring Gender_Name;
	CRenderChar Gender_CharEnts[GENDERPANEL_MAINBTNS];
	int Gender_Item;
	MSButton *Gender_NameOK;

	TextPanel *Weapon_MainLabel;
	class WeaponButton *Weapon_MainBtn[WEAPONPANEL_MAINBTNMAX];
	class CImageDelayed *Weapon_MainBtnImg[WEAPONPANEL_MAINBTNMAX];
	ActionSignal *Weapon_MainActionSig[WEAPONPANEL_MAINBTNMAX];
	Label *Weapon_BtnLabel[WEAPONPANEL_MAINBTNMAX];

	int iButtons;
	bool fClosingMenu;

	CRenderChar m_CharEnts[3];
	CRenderSpawnbox m_SpawnBox;

public:
	CNewCharacterPanel(int iTrans, int iRemoveMe, int x, int y, int wide, int tall);
	~CNewCharacterPanel();

	virtual bool SlotInput(int iSlot);
	virtual void Open(void);
	virtual void Close(void);
	virtual void Update(void);
	virtual void Initialize(void);
	virtual bool KeyInput(int down, int keynum, const char *pszCurrentBinding);
	virtual void Think(void);

	void UpdateUpload();
	void UpdateModels();
	void Gender_SelectItem(int Btn);
	void Gender_NameSelected();

	CTransparentPanel *m_ConfirmPanel;
	int m_DeleteChar;
	stage_e m_Stage;
};

#endif
