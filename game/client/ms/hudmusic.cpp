#include "inc_huditem.h"
#include "sharedutil.h"
#include "hudmusic.h"

MS_DECLARE_MESSAGE(m_Music, Music);

int CHudMusic::Init(void)
{
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
	m_MP3.Think();
}

void CHudMusic::Reload()
{
	m_MP3.StopMusic(false);
}

int CHudMusic::MsgFunc_Music(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	/*
		-1 - stop music
		0 - area music
		1 - combat music
		2 - system music
	*/
	int iCmd = READ_BYTE();
	switch (iCmd) {
	case -1: // stop music
	{
		gEngfuncs.Con_Printf("stopping musix!!\n");
		m_MP3.StopMusic(true);
		break;
	}
	case 0: // area music
	{
		if (!m_bSystem)
		{
			const char *musicFile = READ_STRING();
			m_MP3.TransitionMusic(musicFile, 0); //sound engine handles the including of dir now.
		}
		break;
	}
	case 1: // combat music
	{
		const char *musicFile = READ_STRING();
		m_MP3.TransitionMusic(musicFile, 1); //sound engine handles the including of dir now.
		break;
	}
	case 2: // system music
	{
		m_bSystem = true;
		const char *musicFile = READ_STRING();
		m_MP3.TransitionMusic(musicFile, 0); //sound engine handles the including of dir now.
		break;
	}
	}

	return 1;
}