#ifndef HUD_MUSIC_SYSTEM_H
#define HUD_MUSIC_SYSTEM_H

#include "fmod/musicsystem.h"

class CHudMusic : public CHudBase
{
public:
	int Init(void);
	void InitHUDData(void) {  }
	void Think(void);
	//int Redraw(float flTime, int intermission) { };
	void Shutdown(void);
	void Reload(void);
	int MsgFunc_Music(const char* pszName, int iSize, void* pbuf);

private:
	CMusicSystem m_MP3;
	bool m_bSystem = false;
};

#endif