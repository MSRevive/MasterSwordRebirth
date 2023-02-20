#include "inc_huditem.h"
#include "sharedutil.h"
#include "hudmusic.h"

MS_DECLARE_MESSAGE(m_Music, Music);

int CHudMusic::Init(void)
{
	CVAR_CREATE("fmod_volume", "1.0", FCVAR_ARCHIVE);
	HOOK_MESSAGE(Music);
	m_MP3.Init();
	return 1;
}

void CHudMusic::Shutdown(void)
{
	m_MP3.Shutdown();
}

void CHudMusic::Think()
{
	m_MP3.FadeThink();
}

void CHudMusic::Reload()
{
	m_MP3.StopMusic(false);
}

int CHudMusic::MsgFunc_Music(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iCmd = READ_BYTE();
	switch (iCmd) {
	case 0:
	{
		char *musicFile = READ_STRING();
		m_MP3.TransitionMusic(musicFile); //sound engine handles the including of dir now.
	}
	case 1:
		m_MP3.StopMusic(true);
	}

	return 1;
}