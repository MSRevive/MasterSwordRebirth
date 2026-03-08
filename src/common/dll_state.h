//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

//DLL State Flags

enum {
	DLL_INACTIVE = 0, // no dll
	DLL_ACTIVE = 1,   // dll is running
	DLL_PAUSED = 2,   // dll is paused
	DLL_CLOSE = 3,    // closing down dll
	DLL_TRANS = 4,    // Level Transition
	// DLL Pause reasons
	DLL_NORMAL = 0,  // User hit Esc or something.
	DLL_QUIT = 4,    // Quit now
	DLL_RESTART = 6 // Switch to launcher for linux, does a quit but returns 1
};
// DLL Substate info ( not relevant )

enum {
	ENG_NORMAL = (1 << 0);
};