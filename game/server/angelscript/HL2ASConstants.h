//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SHARED_ZPS_ANGELSCRIPT_HL2ASCONSTANTS_H
#define GAME_SHARED_ZPS_ANGELSCRIPT_HL2ASCONSTANTS_H

namespace ModuleAccessMask
{
/**
*	Access masks for modules.
*/
enum ModuleAccessMask
{
	/**
	*	No access.
	*/
	NONE = 0,

	/**
	*	Shared API.
	*/
	SHARED = 1 << 0,

	/**
	*	Map script specific.
	*/
	MAPSCRIPT = SHARED | 1 << 1,

	/**
	*	Base script specific.
	*/
	BASE = SHARED | 1 << 2,

	/**
	*	All scripts.
	*/
	ALL = SHARED | MAPSCRIPT | BASE
};
}

const size_t AS_MAX_VARARGS = 16;

#endif // GAME_SHARED_ZPS_ANGELSCRIPT_HL2ASCONSTANTS_H
