#ifndef HUD_MUSIC_SYSTEM_H
#define HUD_MUSIC_SYSTEM_H

#include "fmod/soundengine.h"

class CHudMusic : public CHudBase
{
public:
	int Init(void);
	void InitHUDData(void) {  }
	void Think(void);
	//int Redraw(float flTime, int intermission) { };
	void Shutdown(void);
	int MsgFunc_Music(const char* pszName, int iSize, void* pbuf);

private:
	CSoundEngine m_MP3;
};

#endif