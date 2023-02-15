#ifndef HUD_MUSIC_SYSTEM_H
#define HUD_MUSIC_SYSTEM_H

#include "mp3.h"

class CHudMusic : public CHudBase
{
public:
	int Init(void);
	void InitHUDData(void) {  }
	void Think(void) {  }
	int Redraw(float flTime, int intermission);
	void Shutdown(void);
	int MsgFunc_Music(const char* pszName, int iSize, void* pbuf);

private:
	CMP3 m_MP3;
};

#endif