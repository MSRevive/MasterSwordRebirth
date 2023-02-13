#include "hudmusic.h"
#include "inc_huditem.h"

int CHudMusic::Initalize(void)
{
	if (m_MP3.Initialize())
	{
		HOOK_COMMAND("stopmp3", m_MP3.StopMP3);
		return 1;
	}
}

void CHudMusic::Shutdown(void)
{
	m_MP3.Shutdown();
}

void CHudMusic::Think(void)
{
	m_MP3.Frame();
}

void CHudMusic::MsgFunc_Music(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iCmd = READ_BYTE();
	switch (iCmd) {
	case 0:
		m_MP3.PlayMP3(READ_STRING());
	case 1:
		m_MP3.StopMP3(0);
	}
}