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
		const char *musicFile = READ_STRING();
		gEngfuncs.Con_Printf("MUSIC: SENT '%s'\n", musicFile);
		m_MP3.TransitionMusic(musicFile); //sound engine handles the including of dir now.
		break;
	}
	case 1:
		gEngfuncs.Con_Printf("stopping musix!!\n");
		m_MP3.StopMusic(true);
		break;
	}

	return 1;
}