//based on work from https://developer.valvesoftware.com/wiki/Implementing_FMOD and https://github.com/Spirrwell/source-sdk-2013-FMOD

#include <fmod/fmod_errors.h>
#include "hud.h"
#include "cl_util.h"
#include "clglobal.h"
#include "soundengine.h"

CSoundEngine::CSoundEngine()
{
	m_fFadeDelay = 0.0;
	m_TranSound = "NULL";
	m_CurSound = "NULL";
	m_bShouldTransition = false;
	m_bFadeIn = false;
	m_bFadeOut = false;
	m_fVolume = 1.0;
	m_pSystem = nullptr;
	m_pSound = nullptr;
	m_pChannel = nullptr;
}

CSoundEngine::~CSoundEngine()
{
	m_fFadeDelay = 0.0;
	m_TranSound = "NULL";
	m_CurSound = "NULL";
	m_bShouldTransition = false;
	m_bFadeIn = false;
	m_bFadeOut = false;
}

// Starts FMOD
bool CSoundEngine::InitFMOD( void )
{
	FMOD_RESULT	result = FMOD::System_Create( &m_pSystem ); // Create the main system object.
	if (result != FMOD_OK)
	{
		gEngfuncs.Con_Printf("FMOD ERROR: System creation failed!\n");
		return false;
	}
	else
		gEngfuncs.Con_Printf("FMOD system successfully created.\n");
		
	result = m_pSystem->init(100, FMOD_INIT_NORMAL, 0);   // Initialize FMOD system.
	if (result != FMOD_OK)
	{
		gEngfuncs.Con_Printf("FMOD ERROR: Failed to initialize properly!\n");
		return false;
	}
	else
		gEngfuncs.Con_Printf("FMOD initialized successfully.\n");

	return true;
}

// Stops FMOD
bool CSoundEngine::ExitFMOD( void )
{
	FMOD_RESULT	result = m_pSystem->release();

	if (result != FMOD_OK)
	{
		gEngfuncs.Con_Printf("FMOD ERROR: System did not terminate properly!\n");
		return false;
	}
	else
		gEngfuncs.Con_Printf("FMOD system terminated successfully.\n");

	m_pChannel->stop();
	m_pSound->release();

	return true;
}

// Returns the name of the current ambient sound being played
// If there is an error getting the name of the ambient sound or if no ambient sound is currently being played, returns "NULL"
const char* CSoundEngine::GetCurrentSoundName( void )
{
	return m_CurSound;
}

// Handles all fade-related sound stuffs
// Called every frame when the client is in-game
bool CSoundEngine::FadeThink( void )
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
				m_pChannel->setVolume( 0.0 );
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

		result = m_pSystem->playSound(m_pSound, GetChannelGroup(CHANNELGROUP_MUSIC), false, &m_pChannel);

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
bool CSoundEngine::IsSoundPlaying( const char* pszSong )
{
	const char* currentSoundPlaying = GetCurrentSoundName();
	return strcmp(currentSoundPlaying, pszSong) == 0;
}

// Abruptly starts playing a specified ambient sound
// In most cases, we'll want to use TransitionAmbientSounds instead
bool CSoundEngine::PlayAmbientSound( const char* pszSong, bool fadeIn )
{
	char songPath[256];
	_snprintf(songPath, 256, "%s/music/%s", gEngfuncs.pfnGetGameDirectory(), pszSong);
	FMOD_RESULT	result = m_pSystem->createStream(songPath, FMOD_DEFAULT, 0, &m_pSound);

	if (result != FMOD_OK)
	{
		gEngfuncs.Con_Printf("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", pszSong, result);
		return false;
	}

	result = m_pSystem->playSound(m_pSound, GetChannelGroup(CHANNELGROUP_MUSIC), false, &m_pChannel);

	if (result != FMOD_OK)
	{
		gEngfuncs.Con_Printf("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", pszSong, result);
		return false;
	}

	if ( fadeIn )
	{
		m_pChannel->setVolume( 0.0 );
		m_bFadeIn = true;
	}

	m_CurSound = pszSong;
	return true;
}

// Abruptly stops playing all ambient sounds
void CSoundEngine::StopAmbientSound( bool fadeOut )
{
	if ( fadeOut )
	{
		m_pChannel->setVolume( m_fVolume );
		m_bFadeOut = true;
	}
	else
	{
		m_pChannel->setVolume( m_fVolume );
	}

	m_CurSound = "NULL";
}

// Transitions between two ambient sounds if necessary
// If a sound isn't already playing when this is called, don't worry about it
void CSoundEngine::TransitionAmbientSounds( const char* pszSong )
{
	m_pChannel->setVolume( m_fVolume );
	m_TranSound = pszSong;

	m_bFadeOut = true;
	m_bShouldTransition = true;
	m_bFadeIn = true;
}