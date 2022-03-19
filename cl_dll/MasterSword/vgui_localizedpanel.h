// MiB MAR2015_01 [LOCAL_PANEL] - Local panel header
#ifndef LocalPage_H
#define LocalPage_H

#include "vgui_mscontrols.h"
#include "../../MSShared/iscript.h"

#define LOCAL_MENU_NAME "localmenu"

class CLocalizedPanel : public CMenuPanel
{
private:
	Panel *m_pMainPanel; // Panel everything sits on
	TextPanel *m_pTitle; // Title label

	mslist<ClassButton *> m_ButtonList; // List of all dynamic buttons
	int m_iButtonTotal;					// Total number of buttons allocated

	mslist<Panel *>	m_SubPanelList; // List of all dynamic sub-panels
	int	m_iSubPanelTotal;		// Total number of sub-panels

	CTFScrollPanel *m_pScroll; // Scrolling window
	Panel *m_pTextPanel;	   // Panel inside scrolling window

	msstring m_ServerEntString;		   // Ent string for server entity
	IScripted *m_pLocalScriptedEntity; // Client entity that set up the menu

	MSButton *m_pCloseButton;

public:
	CLocalizedPanel(Panel *pParent);
	~CLocalizedPanel();

	void Show(void);  // Calls the actual VGUI show panel stuff, which calls open
	void Open(void);  // Does the adjusting of the elements and setting visible
	void Hide(void);  // Calls the actual VGUI hide panel stuff, which calls close
	void Close(void); // Does the hiding of window and other closing things
	void Reset(void); // Resets menu to default

	int GetNextY(void);

	void SetPanelTitle(msstring title);	// Set title of the menu
	void SetServerEntString(msstring sEntString);	// Set the server-side entity for callback, if needed
	void SetClientScriptedEntity(IScripted *pEntity); // Set the client-side entity for callback, if needed

	// Add a button with given info
	void AddButton(msstring sText, bool bEnabled, bool bCloseOnClick, int cbType, msstring sCallback, msstring sCallbackData = "");
	void PositionButtons(void);	// Automatically positions all buttons based on the total number
	void ClearButtons(void); // Clear all buttons
	void DoCallback(bool bDoClose, int callback, msstring sCallback, msstring sCallbackData = "");	// Handle button clicked
	void CallbackServer(msstring sCallback, msstring sCallbackData = "");	// Does callback on server entity
	void CallbackClient(msstring sCallback, msstring sCallbackData = "");	// Does callback on client entity

	void AddImage(const char *pszName, bool bIsTga, bool bBorder = false, int vFrame = 0);

	void AddSubPanel(Panel *pPanel);
	void ClearSubPanels(void);

	void AddParagraph(const char *pszText);			  // Formats text and adds panel
	void ClearParagraphs(void);						  // Clear all paragraphs
	void ReadParagraphsFromLocalized(msstring title); // Set up paragraphs using titles.txt
	void ReadParagraphsFromFile(msstring fname);	  // Set up paragraphs from a local file
};

class CCallback_Signal : public ActionSignal
{
private:
	CLocalizedPanel *m_pCallbackPanel;
	bool m_bCloseOnClick;
	int m_cbType;
	msstring m_sCallback;
	msstring m_sCallbackData;

public:
	CCallback_Signal(CLocalizedPanel *pCallbackPanel, bool bCloseOnClick, int cbType, msstring sCallback, msstring sCallbackData = "")
	{
		m_pCallbackPanel = pCallbackPanel;
		m_bCloseOnClick = bCloseOnClick;
		m_cbType = cbType;
		m_sCallback = sCallback;
		m_sCallbackData = sCallbackData;
	}

	virtual void actionPerformed(Panel *panel)
	{
		m_pCallbackPanel->DoCallback(m_bCloseOnClick, m_cbType, m_sCallback);
	}
};

#endif
