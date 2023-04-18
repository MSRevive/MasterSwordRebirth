//based on work from https://developer.valvesoftware.com/wiki/Implementing_FMOD and https://github.com/Spirrwell/source-sdk-2013-FMOD

#include <fmod/fmod_errors.h>
#include "hud.h"
#include "cl_util.h"
#include "clglobal.h"
#include "soundengine.h"

#include "musicsystem.h"

void CMusicSystem::Init()
{
	m_pSystem = gSoundEngine.GetSystem();
	//If we ever decide to do submixing replace the NULL pointer.
	m_pChannelGroup = NULL;
	m_bFadeOut = false;

}

void CMusicSystem::Shutdown()
{
	if (m_pChannel)
		m_pChannel->stop();

	m_pSound->release();
}

// Returns the name of the current ambient sound being played
// If there is an error getting the name of the ambient sound or if no ambient sound is currently being played, returns "NULL"
std::string CMusicSystem::GetCurrentSoundName()
{
	return m_CurSound;
}

// Handles all fade-related and volume-related sound stuffs
// Called every frame when the client is in-game
bool CMusicSystem::Think()
{
	//check if volume is the intended volume
	float cvar_fMP3Vol = CVAR_GET_FLOAT("MP3Volume");
	if (m_fVolume != cvar_fMP3Vol) {
		m_fVolume = cvar_fMP3Vol;
		// if we aren't in the middle of fading already set the volume to our wanted volume.
		if (!m_bFadeOut && !m_bFadeIn) {
			m_pChannel->setVolume(m_fVolume);
		}
	}

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
				m_CurSound = "";
				if ( m_bShouldTransition )
				{
					PlayMusic(m_TranSound, true);
					m_bShouldTransition = false;
					m_TranSound = "";
				}	
			}
		}
	}
	else if ( m_bFadeIn )
	{
		if ( gEngfuncs.GetClientTime() >= m_fFadeDelay )
		{
			float tempvol;
			m_pChannel->getVolume( &tempvol );

			//we only want to fade up the the wanted volume not to 1.0
			if ( tempvol < m_fVolume )
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

	if (!IsPlaying() && m_AreaMusic.size() > 0)
		PlayMusic(m_AreaMusic, true);

	return true;
}

// Compares specified ambient sound with the current ambient sound being played
// Returns true if they match, false if they do not or if no sound is being played
bool CMusicSystem::IsPlaying()
{
	bool playing = false;

	FMOD_RESULT	result = m_pChannel->isPlaying(&playing);
	if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
		return false;

	return playing;
}

// Abruptly starts playing a specified ambient sound
// In most cases, we'll want to use TransitionAmbientSounds instead
bool CMusicSystem::PlayMusic(std::string pszSong, bool fadeIn)
{
	m_bFadeOut = false;
	m_CurSound = pszSong;
	char songPath[256];
	_snprintf(songPath, 256, "%s/music/%s", gEngfuncs.pfnGetGameDirectory(), pszSong.c_str());
	FMOD_RESULT	result = m_pSystem->createStream(songPath, FMOD_DEFAULT, 0, &m_pSound);

	if (result != FMOD_OK)
	{
		gEngfuncs.Con_Printf("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", pszSong.c_str(), result);
		return false;
	}

	result = m_pSystem->playSound(m_pSound, m_pChannelGroup , false, &m_pChannel);

	if (result != FMOD_OK)
	{
		gEngfuncs.Con_Printf("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", pszSong.c_str(), result);
		return false;
	}

	//never be louder than what the player set
	m_pChannel->setVolume(m_fVolume);
	if (fadeIn)
	{
		m_pChannel->setVolume( 0.0 );
		m_bFadeIn = true;
	}

	return true;
}

//Reset flags and stop music. For stopsound cmd and map swaps
void CMusicSystem::Reload(bool fadeOut)
{
	m_bCombat = false;
	m_bSystem = false;
	m_AreaMusic = "";

	if (fadeOut)
	{
		m_pChannel->setVolume(m_fVolume);
		m_bFadeOut = true;
	}
	else
		m_pChannel->stop();
}

// Abruptly stops playing all ambient sounds
void CMusicSystem::StopMusic(bool fadeOut)
{
	m_bCombat = false;
	m_AreaMusic = "";

	if (!m_bSystem)
	{
		if (fadeOut)
		{
			m_pChannel->setVolume(m_fVolume);
			m_bFadeOut = true;
		}
		else
			m_pChannel->stop();
	}
}

// Transitions between two ambient sounds if necessary
// If a sound isn't already playing when this is called, don't worry about it
void CMusicSystem::TransitionMusic(std::string pszSong, int mode)
{
	m_pChannel->setVolume(m_fVolume);

	if (IsPlaying() && pszSong == m_CurSound) return; //If song attempting to play is same as currently playing track
	if (m_TranSound == pszSong) return; //If music is already transitioning to this track

	bool bSwapMusic = false; //Flag to see if music will swap after checks

	switch(mode)
	{
		case MUSIC_SYSTEM:
		{
			m_bSystem = true;
			bSwapMusic = true;
		}
		case MUSIC_COMBAT: // combat music
		{
			if (!m_bSystem)
			{
				m_bCombat = true;
				bSwapMusic = true;
			}
			break;
		}
		case MUSIC_AREA: // area music
		{
			m_AreaMusic = pszSong; //Always switch queued area music

			if (!m_bCombat && !m_bSystem)
			{
				bSwapMusic = true;
			}
			break;
		}
	}

	if (bSwapMusic)
	{
		if (IsPlaying())
		{
			m_bFadeOut = true;
			m_bShouldTransition = true;
			m_TranSound = pszSong;
		}
		else
		{
			m_bFadeOut = false;
			m_bShouldTransition = false;
			m_TranSound = "";
			PlayMusic(pszSong, true);
		}
	}
}

// Abruptly stops playing all ambient sounds
void CMusicSystem::StopCombat()
{
	if (m_bCombat)
	{
		m_bCombat = false;
		TransitionMusic(m_AreaMusic,0);
	}
}