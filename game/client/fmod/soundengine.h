#pragma once

#include <fmod/fmod.hpp>

enum eChannelGroupType
{
	CHANNELGROUP_MUSIC = 0,
	NUM_CHANNELGROUPS = 2
};

class CSoundEngine
{
public:
	CSoundEngine() = default;
	~CSoundEngine() = default;

	bool InitFMOD();
	bool ExitFMOD();
	bool Update();
	FMOD::System *GetSystem() { return m_pSystem; }

	FMOD::ChannelGroup *GetChannelGroup(eChannelGroupType channelgroupType) { return *m_pChannelGroups[channelgroupType]; }

private:
	FMOD::System *m_pSystem;
	FMOD::ChannelGroup** m_pChannelGroups[NUM_CHANNELGROUPS] = { nullptr };
};

extern CSoundEngine gSoundEngine;