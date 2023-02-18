//based on work from https://developer.valvesoftware.com/wiki/Implementing_FMOD and https://github.com/Spirrwell/source-sdk-2013-FMOD

#pragma once

#include <fmod/fmod.hpp>

class CMusicEngine
{
public:
	CMusicEngine();
	~CMusicEngine();
 
	void Init();
	void Shutdown();

	bool FadeThink();

	bool IsPlaying();

	bool PlayMusic( const char* pszSong, bool fadeIn = false );
	void StopMusic(bool fadeOut = false);
	void TransitionMusic( const char* pszSong );

private:
	const char *GetCurrentSoundName( void );

	const char *m_CurSound;
	const char *m_TranSound;
	bool m_bShouldTransition;
	bool m_bFadeIn;
	bool m_bFadeOut;
	float m_fFadeDelay;
	float m_fVolume;

	FMOD::System *m_pSystem;
	FMOD::Sound *m_pSound;
	FMOD::Channel *m_pChannel;
	FMOD::ChannelGroup *m_pChannelGroup;
};