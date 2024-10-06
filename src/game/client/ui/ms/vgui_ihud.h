#pragma once

class IHUD
{
public:
	virtual void Init() {}
	virtual void Reset() {}									//New level
	virtual void Update() {}								//Every frame
	virtual void SetVisible(bool fVisible) {}				//Show/Hide
	virtual void Close() {}									//Close
	virtual IHUD *GetInterface() { return this; } //Get pointer to this interface
};