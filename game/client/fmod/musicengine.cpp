//based on work from https://developer.valvesoftware.com/wiki/Implementing_FMOD and https://github.com/Spirrwell/source-sdk-2013-FMOD

#include <fmod/fmod_errors.h>
#include "hud.h"
#include "cl_util.h"
#include "clglobal.h"
#include "soundengine.h"

#include "musicengine.h"

//extern CSoundEngine gSoundEngine;

CMusicEngine::CMusicEngine()
{
	m_fFadeDelay = 0.0;
	m_TranSound = "NULL";
	m_CurSound = "NULL";
	m_bShouldTransition = false;
	m_bFadeIn = false;
	m_bFadeOut = false;
	m_fVolume = CVAR_GET_FLOAT("MP3Volume");
	m_pSystem = nullptr;
}

CMusicEngine::~CMusicEngine()
{
	m_fFadeDelay = 0.0;
	m_TranSound = "NULL";
	m_CurSound = "NULL";
	m_bShouldTransition = false;
	m_bFadeIn = false;
	m_bFadeOut = false;
	m_pSystem = nullptr;
}

// Starts FMOD
void CMusicEngine::Init( void )
{
	m_pSystem = gSoundEngine.GetSystem();
	m_pChannelGroup = gSoundEngine.GetChannelGroup(CHANNELGROUP_MUSIC);
	m_bFadeOut = false;
}

// Stops FMOD
void CMusicEngine::Shutdown( void )
{
	m_pChannel->stop();
	m_pSound->release();
	m_pSystem = nullptr;
}

// Returns the name of the current ambient sound being played
// If there is an error getting the name of the ambient sound or if no ambient sound is currently being played, returns "NULL"
const char* CMusicEngine::GetCurrentSoundName( void )
{
	return m_CurSound;
}

// Handles all fade-related sound stuffs
// Called every frame when the client is in-game
bool CMusicEngine::FadeThink( void )
{
	if ( m_bFadeOut )
	{
		if ( gEngfuncs.GetClientTime() >= m_fFadeDelay )
		{
			float tempvol;
			m_pChannel->getVolume( &tempvol );

			if ( tempvol > 0.0 )
			{
				m_pChannel->setVolume( tempvol - 0.05 );
				m_fFadeDelay = gEngfuncs.GetClientTime() + 0.1;
			}
			else
			{
				m_pChannel->stop(); //stop channel when fadeout is done.
				m_bFadeOut = false;
				m_fFadeDelay = 0.0;
			}
		}
	}
	else if ( m_bShouldTransition )
	{
		char songPath[256];
		_snprintf(songPath, 256, "%s/music/%s", gEngfuncs.pfnGetGameDirectory(), m_TranSound);
		FMOD_RESULT	result = m_pSystem->createStream(songPath, FMOD_DEFAULT, 0, &m_pSound);

		if (result != FMOD_OK)
		{
			gEngfuncs.Con_Printf("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", m_TranSound, result);
			m_TranSound = "NULL";
			m_bShouldTransition = false;
			return false;
		}

		result = m_pSystem->playSound(m_pSound, m_pChannelGroup, false, &m_pChannel);

		if (result != FMOD_OK)
		{
			gEngfuncs.Con_Printf("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", m_TranSound, result);
			m_TranSound = "NULL";
			m_bShouldTransition = false;
			return false;
		}

		m_CurSound = m_TranSound;
		m_TranSound = "NULL";
		m_bShouldTransition = false;
	}
	else if ( m_bFadeIn )
	{
		if ( gEngfuncs.GetClientTime() >= m_fFadeDelay )
		{
			float tempvol;
			m_pChannel->getVolume( &tempvol );

			if ( tempvol < 1.0 )
			{
				m_pChannel->setVolume( tempvol + 0.05 );
				m_fFadeDelay = gEngfuncs.GetClientTime() + 0.1;
			}
			else
			{
				m_pChannel->setVolume( m_fVolume );
				m_bFadeIn = false;
				m_fFadeDelay = 0.0;
			}
		}
	}

	return true;
}

// Compares specified ambient sound with the current ambient sound being played
// Returns true if they match, false if they do not or if no sound is being played
bool CMusicEngine::IsPlaying()
{
	bool *playing = false;
	FMOD_RESULT	result = m_pChannel->isPlaying(playing);
	if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
		return false;

	return true;
}

// Abruptly starts playing a specified ambient sound
// In most cases, we'll want to use TransitionAmbientSounds instead
bool CMusicEngine::PlayMusic( const char* pszSong, bool fadeIn )
{
	char songPath[256];
	_snprintf(songPath, 256, "%s/music/%s", gEngfuncs.pfnGetGameDirectory(), pszSong);
	FMOD_RESULT	result = m_pSystem->createStream(songPath, FMOD_DEFAULT, 0, &m_pSound);

	if (result != FMOD_OK)
	{
		gEngfuncs.Con_Printf("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", pszSong, result);
		return false;
	}

	result = m_pSystem->playSound(m_pSound, m_pChannelGroup, false, &m_pChannel);

	if (result != FMOD_OK)
	{
		gEngfuncs.Con_Printf("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", pszSong, result);
		return false;
	}

	m_pChannel->setVolume(m_fVolume);
	if ( fadeIn )
	{
		m_pChannel->setVolume( 0.0 );
		m_bFadeIn = true;
	}

	m_CurSound = pszSong;
	return true;
}

// Abruptly stops playing all ambient sounds
void CMusicEngine::StopMusic(bool fadeOut)
{
	if (fadeOut)
	{
		m_pChannel->setVolume(m_fVolume);
		m_bFadeOut = true;
	}
	else
		m_pChannel->stop();

	m_CurSound = "NULL";
}

// Transitions between two ambient sounds if necessary
// If a sound isn't already playing when this is called, don't worry about it
void CMusicEngine::TransitionMusic(const char* pszSong)
{
	m_pChannel->setVolume(m_fVolume);
	m_TranSound = pszSong;

	if (IsPlaying())
		m_bFadeOut = true;
		
	m_bShouldTransition = true;
	m_bFadeIn = true;
}