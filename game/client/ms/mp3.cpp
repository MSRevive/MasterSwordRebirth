//mp3 support added by Killar

#include "../hud.h"
#include "../cl_util.h"
#include "mp3.h"

int CMP3::Initialize()
{
	m_flFadeoutStart = 0;
	m_flFadeoutDuration = 0;

	char fmodlib[256];

	m_iIsPlaying = 0;
	snprintf(fmodlib, sizeof(fmodlib), "%s/cl_dlls/fmod.dll", gEngfuncs.pfnGetGameDirectory());
	// replace forward slashes with backslashes
	for( int i=0; i < 256; i++ )
		if( fmodlib[i] == '/' ) fmodlib[i] = '\\';
	
	m_hFMod = LoadLibrary( fmodlib );

	if( m_hFMod != NULL )
	{
		// fill in the function pointers
		(FARPROC&)VER = 	GetProcAddress(m_hFMod, "_FSOUND_GetVersion@0");
		(FARPROC&)SCL = 	GetProcAddress(m_hFMod, "_FSOUND_Stream_Close@4");
		(FARPROC&)SOP = 	GetProcAddress(m_hFMod, "_FSOUND_SetOutput@4");
		(FARPROC&)SBS = 	GetProcAddress(m_hFMod, "_FSOUND_SetBufferSize@4");
		(FARPROC&)SDRV = 	GetProcAddress(m_hFMod, "_FSOUND_SetDriver@4");
		(FARPROC&)INIT = 	GetProcAddress(m_hFMod, "_FSOUND_Init@12");
		(FARPROC&)SOF = 	GetProcAddress(m_hFMod, "_FSOUND_Stream_OpenFile@12");
		//(FARPROC&)LNGTH = 	GetProcAddress(m_hFMod, "_FSOUND_Stream_GetLength@4");
		(FARPROC&)SO = 		GetProcAddress(m_hFMod, "_FSOUND_Stream_Open@16");//AJH Use new version of fmod
		(FARPROC&)SPLAY = 	GetProcAddress(m_hFMod, "_FSOUND_Stream_Play@8");
		(FARPROC&)CLOSE = 	GetProcAddress(m_hFMod, "_FSOUND_Close@0");

		(FARPROC&)SETVOL =	GetProcAddress(m_hFMod, "_FSOUND_SetVolume@8");
		
		if( !(SETVOL && SCL && SOP && SBS && SDRV && INIT && (SOF||SO) && SPLAY && CLOSE) )
		{
			FreeLibrary( m_hFMod );
			m_hFMod = NULL;
			gEngfuncs.Con_Printf("Fatal Error: FMOD functions couldn't be loaded!\n");
			return 0;
		}
	}

	else
	{
		gEngfuncs.Con_Printf("Fatal Error: FMOD library couldn't be loaded!\n");
		return 0;
	}
	gEngfuncs.Con_Printf("FMOD v.%f library loaded.\n", VER());
	return 1;
}

int CMP3::Shutdown()
{
	if( m_hFMod )
	{
		CLOSE();

		FreeLibrary( m_hFMod );
		m_hFMod = NULL;
		m_iIsPlaying = 0;
		return 1;
	}
	else
		return 0;
}

int CMP3::StopMP3( float fade )
{
	if (!m_hFMod) return 1;
	
	if (fade == 0)
	{
		SCL( m_Stream );
		m_iIsPlaying = 0;
		m_flFadeoutStart = 0;
		m_flFadeoutDuration = 0;
//		gEngfuncs.Con_Printf("mp3stop\n");
		return 1;
	}

	m_flFadeoutStart = gEngfuncs.GetClientTime();
	m_flFadeoutDuration = fade;
	return 1;
}

int CMP3::PlayMP3( const char *pszSong )
{
	if (!m_hFMod) return 0;

	m_flFadeoutStart = 0;
	m_flFadeoutDuration = 0;

	// Wargon: fmod_volume ���������� ��������� ������, ��������������� ����� FMOD.
	SETVOL(0, (CVAR_GET_FLOAT("fmod_volume") * 255));

	if( m_iIsPlaying )
	{
		if (!strcmp(m_szMP3File, pszSong))
		{
			// buz: we are playing this song now, dont do anything
			return 1;
		}
		
		// sound system is already initialized
		SCL( m_Stream );
	} 
	else
	{
		SOP(FSOUND_OUTPUT_DSOUND);
		SBS(200);
		SDRV(0);
		INIT(44100, 1, 0); // we need just one channel, multiple mp3s at a time would be, erm, strange...	
	}//AJH not for really cool effects, say walking past cars in a street playing different tunes

	char song[256];

	snprintf(song, sizeof(song), "%s/%s", gEngfuncs.pfnGetGameDirectory(), pszSong);

//	gEngfuncs.Con_Printf("Using fmod.dll version %f\n",VER());
	if( SOF )
	{													
		m_Stream = SOF( song, FSOUND_NORMAL | FSOUND_LOOP_NORMAL, 1 );
	}
	else if (SO)
	{
		m_Stream = SO( song, FSOUND_NORMAL | FSOUND_LOOP_NORMAL, 0 ,0);
	}
	if(m_Stream)
	{
		SPLAY(0, m_Stream);
		m_iIsPlaying = 1;
		strncpy(m_szMP3File, pszSong, sizeof(m_szMP3File));
	
	//	int vol = GETVOL(0);
	//	gEngfuncs.Con_Printf("VOLUME IS: %d\n",vol);
		return 1;
	}
	else
	{
		m_iIsPlaying = 0;
		gEngfuncs.Con_Printf("Error: Could not load %s\n",song);
		return 0;
	}
}

// buz - ��������� �������� ��������� ������
void CMP3::Frame()
{
	if (m_flFadeoutDuration)
	{
		float delta = (gEngfuncs.GetClientTime() - m_flFadeoutStart) / m_flFadeoutDuration;
		if (delta > 1)
		{
			// fadeout finished, turn off
			StopMP3(0);
			return;
		}
		// Wargon: ��������� ���������� � ������� ���������.
		int vol = (CVAR_GET_FLOAT("fmod_volume") * 255) - (int)(delta * 255);
		SETVOL( 0, vol );
	}
	// Wargon: ����������� ����������� ��������� � �������� �������.
	else
	{
		SETVOL( 0, (CVAR_GET_FLOAT("fmod_volume") * 255) );
	}
}