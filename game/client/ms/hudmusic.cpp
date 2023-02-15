#include "inc_huditem.h"
#include "sharedutil.h"
#include "hudmusic.h"

MS_DECLARE_MESSAGE(m_Music, Music);

int CHudMusic::Init(void)
{
	CVAR_CREATE("fmod_volume", "1.0", FCVAR_ARCHIVE);

	if (m_MP3.Initialize())
	{
		HOOK_MESSAGE(Music);
		//m_MP3.PlayMP3("music/ara.mp3");
		return 1;
	}

	return 0;
}

void CHudMusic::Shutdown(void)
{
	m_MP3.Shutdown();
}

int CHudMusic::Redraw(float flTime, int intermission)
{
	m_MP3.Frame();
	return 1;
}

int CHudMusic::MsgFunc_Music(const char* pszName, int iSize, void* pbuf)
{
	m_MP3.PlayMP3("music/ara.mp3");
	BEGIN_READ(pbuf, iSize);
	int iCmd = READ_BYTE();
	switch (iCmd) {
	case 0:
	{
		msstring musicFile = READ_STRING();
		msstring songPath = msstring("music/") + musicFile;
		m_MP3.PlayMP3(songPath);
	}
	case 1:
		m_MP3.PlayMP3("music/ara.mp3");
		//m_MP3.StopMP3(READ_FLOAT());
	}

	return 1;
}