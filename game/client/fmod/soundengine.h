//based on work from https://developer.valvesoftware.com/wiki/Implementing_FMOD and https://github.com/Spirrwell/source-sdk-2013-FMOD

#pragma once

#include <fmod/fmod.hpp>

enum eChannelGroupType
{
	CHANNELGROUP_MUSIC = 0,
	NUM_CHANNELGROUPS
};

//we should expand this later to accommodate 3D sounds as well.
class CSoundEngine
{
public:
	CSoundEngine();
	~CSoundEngine();
 
	bool InitFMOD();
	bool ExitFMOD();

	bool FadeThink();

	bool IsPlaying();

	bool PlayMusic( const char* pszSong, bool fadeIn = false );
	void StopMusic();
	void TransitionMusic( const char* pszSong );

	FMOD::ChannelGroup *GetChannelGroup(eChannelGroupType channelgroupType) { return m_pChannelGroups[channelgroupType]; }

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
	FMOD::ChannelGroup *m_pChannelGroups[NUM_CHANNELGROUPS];
};