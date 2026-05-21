/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef SCRIPTEVENT_H
#define SCRIPTEVENT_H

enum script_event_e {

	SCRIPT_EVENT_DEAD = 1000,          // character is now dead
	SCRIPT_EVENT_NOINTERRUPT = 1001,   // does not allow interrupt
	SCRIPT_EVENT_CANINTERRUPT = 1002,  // will allow interrupt
	SCRIPT_EVENT_FIREEVENT = 1003,     // event now fires
	SCRIPT_EVENT_SOUND = 1004,         // Play named wave file (on CHAN_BODY)
	SCRIPT_EVENT_SENTENCE = 1005,      // Play named sentence
	SCRIPT_EVENT_INAIR = 1006,         // Leave the character in air at the end of the sequence (don't find the floor)
	SCRIPT_EVENT_ENDANIMATION = 1007,  // Set the animation by name after the sequence completes
	SCRIPT_EVENT_SOUND_VOICE = 1008,   // Play named wave file (on CHAN_VOICE)
	SCRIPT_EVENT_SENTENCE_RND1 = 1009, // Play sentence group 25% of the time
	SCRIPT_EVENT_NOT_DEAD = 1010      // Bring back to life (for life/death sequences)
};
#endif                                  //SCRIPTEVENT_H
