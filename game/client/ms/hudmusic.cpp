#include <windows.h>
#include <mmsystem.h>
#include "inc_huditem.h"
#include "sharedutil.h"
#include "hudmusic.h"

int CHudMusic::Init()
{
	CVAR_CREATE("fmod_volume", "1.0", FCVAR_ARCHIVE);

	if (m_MP3.Initialize())
	{
		return 1;
	}

	return 0;
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
	{
		msstring songPath;
		msstring musicFile = READ_STRING();
		songPath = msstring("music/") + musicFile;
		m_MP3.PlayMP3(songPath);
	}
	case 1:
		m_MP3.StopMP3(READ_FLOAT());
	}
}

void CHudMusic::StopMusic()
{
	m_MP3.StopMP3(0);
}