#pragma once

// A UI element owned by CClientViewport.
class IViewportPanel
{
public:
	virtual ~IViewportPanel() {}

	virtual const char *GetName() = 0;
	virtual void Reset() = 0;
	virtual void Think() = 0;
	virtual void OnHideClientUI() = 0;
};
