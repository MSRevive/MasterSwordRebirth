//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

//DLL State Flags

constexpr int DLL_INACTIVE = 0; // no dll
constexpr int DLL_ACTIVE = 1;   // dll is running
constexpr int DLL_PAUSED = 2;   // dll is paused
constexpr int DLL_CLOSE = 3;    // closing down dll
constexpr int DLL_TRANS = 4;    // Level Transition

// DLL Pause reasons

constexpr int DLL_NORMAL = 0;  // User hit Esc or something.
constexpr int DLL_QUIT = 4;    // Quit now
constexpr int DLL_RESTART = 6; // Switch to launcher for linux, does a quit but returns 1

// DLL Substate info ( not relevant )
constexpr int ENG_NORMAL = (1 << 0);
