//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

// director_cmds.h
// sub commands for svc_director

constexpr int DRC_ACTIVE =  0; // tells client that he's an spectator and will get director command
constexpr int DRC_STATUS =  1; // send status infos about proxy
constexpr int DRC_CAMERA =  2; // set the actual director camera position
constexpr int DRC_EVENT = 3; // informs the dircetor about ann important game event

constexpr int DRC_FLAG_PRIO_MASK = 0x0F; //	priorities between 0 and 15 (15 most important)
constexpr int DRC_FLAG_SIDE = (1 << 4);
constexpr int DRC_FLAG_DRAMATIC = (1 << 5);

// commands of the director API function CallDirectorProc(...)

constexpr int DRCAPI_NOP = 0;               // no operation
constexpr int DRCAPI_ACTIVE = 1;            // de/acivates director mode in engine
constexpr int DRCAPI_STATUS = 2;            // request proxy information
constexpr int DRCAPI_SETCAM = 3;            // set camera n to given position and angle
constexpr int DRCAPI_GETCAM = 4;            // request camera n position and angle
constexpr int DRCAPI_DIRPLAY = 5;           // set director time and play with normal speed
constexpr int DRCAPI_DIRFREEZE = 6;         // freeze directo at this time
constexpr int DRCAPI_SETVIEWMODE = 7;       // overview or 4 cameras
constexpr int DRCAPI_SETOVERVIEWPARAMS = 8; // sets parameter for overview mode
constexpr int DRCAPI_SETFOCUS = 9;          // set the camera which has the input focus
constexpr int DRCAPI_GETTARGETS = 10;       // queries engine for player list
constexpr int DRCAPI_SETVIEWPOINTS = 11;    // gives engine all waypoints
