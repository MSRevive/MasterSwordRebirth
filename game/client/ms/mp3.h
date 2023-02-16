// mp3 support added by Killar

#ifndef MP3_H
#define MP3_H

#include <windows.h>
#include "fmod.h"
#include "fmod_errors.h"

class CMP3
{
private:
	float			(_stdcall * VER)	(void);//AJH get fmod dll version
	signed char		(_stdcall * SCL)	(FSOUND_STREAM *stream);
	signed char		(_stdcall * SOP)	(int outputtype);
	signed char		(_stdcall * SBS)	(int len_ms);
	signed char		(_stdcall * SDRV)	(int driver);
	signed char		(_stdcall * INIT)	(int mixrate, int maxsoftwarechannels, unsigned int flags);
	FSOUND_STREAM*		(_stdcall * SOF)	(const char *filename, unsigned int mode,int memlength);				//AJH old fmod
	FSOUND_STREAM*		(_stdcall * SO)	(const char *filename, unsigned int mode,int offset, int memlength);	//AJH use new fmod
	int 			(_stdcall * SPLAY)	(int channel, FSOUND_STREAM *stream);
	void			(_stdcall * CLOSE)	( void );

	int				(_stdcall * SETVOL)	(int channel, int vol); // buz
	
	FSOUND_STREAM  *m_Stream;
	int		m_iIsPlaying;
	HINSTANCE	m_hFMod;

	// buz: mp3 file name:
	char m_szMP3File[128];
	// buz: for fadeout:
	float	m_flFadeoutStart;
	float	m_flFadeoutDuration;

public:
	int		Initialize();
	int		Shutdown();
	int		PlayMP3( const char *pszSong );
	int		StopMP3(float fade);
	void	Frame();
};

#endif