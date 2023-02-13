#pragma once

#include "mp3.h"

class CHudMusic : public CHudBase
{
public:
	int Initalize(void);
	void Shutdown(void);
	void Think(void);
	int MsgFunc_Music(const char* pszName, int iSize, void* pbuf);

private:
	CMP3 m_MP3;
}