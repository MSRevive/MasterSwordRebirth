//
// Music - Client-side playback of midi files
//

#ifndef HUD_MUSIC_SYSTEM_H
#define HUD_MUSIC_SYSTEM_H

#include "music.h"

class CHudMusic : public CHudBase
{
public:
	int Init(void);
	void InitHUDData(void);
	void Think(void);
	int MsgFunc_Music(const char* pszName, int iSize, void* pbuf);

	void PlayMusic();
	void StopMusic();

	int m_CurrentSong;
	bool m_CurrentSongMp3; //True if mp3. False if midi
	bool m_PlayingSong;
	ulong m_SongsPlayed, //Bits for songs already played
		m_FailedSongs;	 //Bits for songs that failed to play
	float m_TimePlayNextSong;

	songplaylist m_Songs;
	int m_MidiDevice;
};

#endif // HUD_MUSIC_SYSTEM_H