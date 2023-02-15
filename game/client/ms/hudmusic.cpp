#include "inc_huditem.h"
#include "sharedutil.h"
#include "hudmusic.h"

MS_DECLARE_MESSAGE(m_Music, Music);

int CHudMusic::Init(void)
{
	CVAR_CREATE("fmod_volume", "1.0", FCVAR_ARCHIVE);
	HOOK_MESSAGE(Music);

	if (m_MP3.InitFMOD())
	{
		m_MP3.PlayAmbientSound("ara.mp3");
		return 1;
	}

	return 0;
}

void CHudMusic::Shutdown(void)
{
	m_MP3.ExitFMOD();
}

void CHudMusic::Think()
{
	m_MP3.FadeThink();
}

int CHudMusic::MsgFunc_Music(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iCmd = READ_BYTE();
	switch (iCmd) {
	case 0:
	{
		char *musicFile = READ_STRING();
		m_MP3.TransitionAmbientSounds(musicFile); //sound engine handles the including of dir now.
	}
	case 1:
		m_MP3.StopAmbientSound();
	}

	return 1;
}