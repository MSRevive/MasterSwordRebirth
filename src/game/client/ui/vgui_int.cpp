
#include "vgui_int.h"
#include <vgui_label.h>
#include <vgui_borderlayout.h>
#include <vgui_lineborder.h>
#include <vgui_surfacebase.h>
#include <vgui_textentry.h>
#include <vgui_actionsignal.h>
#include <string.h>
#include "hud.h"
#include "cl_util.h"
#include "camera.h"
#include "kbutton.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"
#include "camera.h"
#include "in_defs.h"
#include "vgui_teamfortressviewport.h"
#include "vgui_controlconfigpanel.h"
#include "logger.h"
#include "clenv.h"

class TexturePanel : public Panel, public ActionSignal
{
private:
	int _bindIndex;
	TextEntry *_textEntry;

public:
	TexturePanel() : Panel(0, 0, 256, 276)
	{
		_bindIndex = 2700;
		_textEntry = new TextEntry("2700", 0, 0, 128, 20);
		_textEntry->setParent(this);
		_textEntry->addActionSignal(this);
	}

public:
	virtual bool isWithin(int x, int y)
	{
		return _textEntry->isWithin(x, y);
	}

public:
	virtual void actionPerformed(Panel *panel)
	{
		char buf[256];
		_textEntry->getText(0, buf, 256);
		sscanf(buf, "%d", &_bindIndex);
	}

protected:
	virtual void paintBackground()
	{
		Panel::paintBackground();

		int wide, tall;
		getPaintSize(wide, tall);

		drawSetColor(0, 0, 255, 0);
		drawSetTexture(_bindIndex);
		drawTexturedRect(0, 19, 257, 257);
	}
};

void *VGui_GetPanel()
{
	return (Panel*)gEngfuncs.VGui_GetPanel();
}

void VGUISystem::Startup()
{
	if (!CRender::CheckOpenGL()) //This exits if not in OpenGL mode
		return;

	Panel *root = (Panel *)VGui_GetPanel();
	root->setBgColor(128, 128, 0, 0);
	//root->setNonPainted(false);
	//root->setBorder(new LineBorder());
	root->setLayout(new BorderLayout(0));

	//root->getSurfaceBase()->setEmulatedCursorVisible(true);

	if (gViewPort != nullptr)
	{
		gViewPort->Initialize();
	}
	else
	{
		gViewPort = new TeamFortressViewport(0, 0, root->getWide(), root->getTall());
		gViewPort->setParent(root);
		gViewPort->Initialize(); //Master Sword - call Initialize the first time the viewport is created
	}

	logfile << Logger::LOG_INFO << "[VGui_Startup: Complete]\n";

	/*
	TexturePanel* texturePanel=new TexturePanel();
	texturePanel->setParent(gViewPort);
	*/
}

void VGUISystem::Shutdown()
{
	delete gViewPort;
	gViewPort = nullptr;
}

void VGUISystem::ViewportPaintBackground(int extents[4])
{
	gEngfuncs.VGui_ViewportPaintBackground(extents);
}