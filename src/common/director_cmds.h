//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

// director_cmds.h
// sub commands for svc_director

enum {
	DRC_ACTIVE = 0, // tells client that he's an spectator and will get director command
	DRC_STATUS = 1, // send status infos about proxy
	DRC_CAMERA = 2, // set the actual director camera position
	DRC_EVENT = 3   // informs the dircetor about ann important game event
};

enum {
	DRC_FLAG_PRIO_MASK = 0x0F, //	priorities between 0 and 15 (15 most important)
	DRC_FLAG_SIDE = (1 << 4),
	DRC_FLAG_DRAMATIC = (1 << 5)
};
// commands of the director API function CallDirectorProc(...)

enum {
	DRCAPI_NOP = 0,               // no operation
	DRCAPI_ACTIVE = 1,            // de/acivates director mode in engine
	DRCAPI_STATUS = 2,            // request proxy information
	DRCAPI_SETCAM = 3,            // set camera n to given position and angle
	DRCAPI_GETCAM = 4,            // request camera n position and angle
	DRCAPI_DIRPLAY = 5,           // set director time and play with normal speed
	DRCAPI_DIRFREEZE = 6,         // freeze directo at this time
	DRCAPI_SETVIEWMODE = 7,       // overview or 4 cameras
	DRCAPI_SETOVERVIEWPARAMS = 8, // sets parameter for overview mode
	DRCAPI_SETFOCUS = 9,          // set the camera which has the input focus
	DRCAPI_GETTARGETS = 10,       // queries engine for player list
	DRCAPI_SETVIEWPOINTS = 11     // gives engine all waypoints
};