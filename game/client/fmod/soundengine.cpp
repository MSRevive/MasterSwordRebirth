#include <fmod/fmod_errors.h>
#include "hud.h"
#include "cl_util.h"
#include "clglobal.h"

#include "soundengine.h"

// Starts FMOD
bool CSoundEngine::InitFMOD(void)
{
	FMOD_RESULT	result = FMOD::System_Create(&m_pSystem); // Create the main system object.
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

	/*result = m_pSystem->createChannelGroup("MusicChannel", m_pChannelGroups[CHANNELGROUP_MUSIC]); //initialize channelgroup for music;
	if (result != FMOD_OK)
	{
		gEngfuncs.Con_Printf("FMOD ERROR: Failed to create music channel group!\n");
		return false;
	}
	else
		gEngfuncs.Con_Printf("FMOD music channel group created successfully.\n");
		*/
	return true;
}

// Stops FMOD
bool CSoundEngine::ExitFMOD(void)
{
	FMOD_RESULT	result = m_pSystem->release();

	if (result != FMOD_OK)
	{
		gEngfuncs.Con_Printf("FMOD ERROR: System did not terminate properly!\n");
		return false;
	}
	else
		gEngfuncs.Con_Printf("FMOD system terminated successfully.\n");

	return true;
}

// Update FMOD System
bool CSoundEngine::Update(void) {

	if (m_pSystem) {

		FMOD_RESULT result = m_pSystem->update();

		if (result != FMOD_OK) {
			gEngfuncs.Con_Printf("FMOD failed to update!.\n");
			return false;
		}

		FMOD::ChannelGroup* debug_music_channel = GetChannelGroup(CHANNELGROUP_MUSIC);

		if (debug_music_channel) {
			bool* debug = new bool;
			int* debug_channelcount = new int;

			debug_music_channel->isPlaying(debug);
			debug_music_channel->getNumChannels(debug_channelcount);

			if (*debug == true) {
				gEngfuncs.Con_Printf("FMOD Music Channel is playing.\n");
			}
			else {
				gEngfuncs.Con_Printf("FMOD Music Channel is not playing.\n");
			}

			gEngfuncs.Con_Printf("FMOD Music Channel count: %i.\n", debug_channelcount);

			delete debug;
			delete debug_music_channel;
		}
	}
	else {
		gEngfuncs.Con_Printf("FMOD System not found by update!.\n");
		return false;
	}

	return true;
}
