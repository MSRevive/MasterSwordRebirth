#ifndef _FATIGUE
#define _FATIGUE
#include "hudbase.h"

class CHudFatigue : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void DoThink(void);
	void Reset(void);
	void InitHUDData(void);
	int MsgFunc_Fatigue(const char *pszName, int iSize, void *pbuf);
	void UserCmd_ToggleFatigue(void);

private:
	float fBreatheTime;
	bool m_DrawFatigue;
	//void Breathe( );
};
#endif //_FATIGUE
