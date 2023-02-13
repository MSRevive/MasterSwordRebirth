#include <windows.h>
#include <mmsystem.h>
#include "inc_huditem.h"
#include "sharedutil.h"
#include "hudmusic.h"

int CHudMusic::Init()
{
	if (m_MP3.Initialize())
	{
		return 1;
	}
}

void CHudMusic::Shutdown()
{
	m_MP3.Shutdown();
}

int CHudMusic::Redraw(float flTime, int intermission)
{
	m_MP3.Frame();
	return 1;
}

void CHudMusic::MsgFunc_Music(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iCmd = READ_BYTE();
	switch (iCmd) {
	case 0:
		m_MP3.PlayMP3(READ_STRING());
	case 1:
		m_MP3.StopMP3(READ_FLOAT());
	}
}

void CHudMusic::StopMusic()
{
	m_MP3.StopMP3(0);
}