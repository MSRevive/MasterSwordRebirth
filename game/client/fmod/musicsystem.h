//based on work from https://developer.valvesoftware.com/wiki/Implementing_FMOD and https://github.com/Spirrwell/source-sdk-2013-FMOD

#pragma once

#include <fmod/fmod.hpp>

class CMusicSystem
{
public:
	CMusicSystem() = default;
	~CMusicSystem() = default;
 
	void Init();
	void Shutdown();

	bool FadeThink();

	bool IsPlaying();

	bool PlayMusic( const char* pszSong, bool fadeIn = false );
	void StopMusic(bool fadeOut = false);
	void TransitionMusic( const char* pszSong );

private:
	const char *GetCurrentSoundName( void );

	const char *m_TranSound = "";
	bool m_bShouldTransition = false;
	bool m_bFadeIn = false;
	bool m_bFadeOut = false;
	float m_fFadeDelay = 0.0;
	float m_fVolume = CVAR_GET_FLOAT("MP3Volume");

	FMOD::System *m_pSystem;
	FMOD::Sound *m_pSound;
	FMOD::Channel *m_pChannel;
	FMOD::ChannelGroup *m_pChannelGroup;
};