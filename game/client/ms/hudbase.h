#ifndef _HUDBASE
#define _HUDBASE

class CHudBase
{
public:
	POSITION m_pos;
	int m_type;
	int m_iFlags;  // active, moving,
	char Name[32]; //Master Sword
	virtual ~CHudBase() {}
	virtual int Init(void) { return 0; }
	virtual int VidInit(void) { return 0; }
	virtual int Draw(float flTime) { return 0; }
	virtual void Think(void) { return; }
	virtual void Reset(void) { return; }
	virtual void InitHUDData(void) {} // called every time a server is connected to
};

#endif //_HUDBASE
