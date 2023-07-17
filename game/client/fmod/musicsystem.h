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

	bool Think();

	bool IsPlaying();

	bool PlayMusic(std::string pszSong, bool fadeIn = false);
	void Reload(bool fadeOut = false);
	void StopMusic(bool fadeOut = false);
	void StopCombat();
	void TransitionMusic(std::string pszSong, int mode);

private:
	std::string GetCurrentSoundName(void);

	std::string m_TranSound = ""; //Transition to this music when fade out is done
	std::string m_CurSound = ""; //Current music playing
	std::string m_AreaMusic = ""; //Current area music assigned

	bool m_bLoop = false;
	bool m_bSystem = false; //Using system music
	bool m_bCombat = false; //Using combat music

	bool m_bShouldTransition = false;
	bool m_bFadeIn = false;
	bool m_bFadeOut = false;
	float m_fFadeDelay = 0.0;
	float m_fVolume = 0.0;

	FMOD::System* m_pSystem;
	FMOD::Sound* m_pSound;
	FMOD::Channel* m_pChannel = 0;
	FMOD::ChannelGroup* m_pChannelGroup;
};