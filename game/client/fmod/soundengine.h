#pragma once

#include <fmod/fmod.hpp>

enum eChannelGroupType
{
	CHANNELGROUP_MUSIC = 0,
	NUM_CHANNELGROUPS
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

	//No longer needed just pass NULL channel since we are not doing any submixing
	//FMOD::ChannelGroup *GetChannelGroup(eChannelGroupType channelgroupType) { return m_pChannelGroups[channelgroupType]; }

private:
	FMOD::System *m_pSystem;
	//No longer needed just pass NULL channel since we are not doing any submixing
	//FMOD::ChannelGroup* m_pChannelGroups[NUM_CHANNELGROUPS];
};

extern CSoundEngine gSoundEngine;