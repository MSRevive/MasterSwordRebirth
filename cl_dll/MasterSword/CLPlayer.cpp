/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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
#include "inc_weapondefs.h"

#include "usercmd.h"
#include "entity_state.h"
#include "demo_api.h"
#include "pm_defs.h"
#include "event_api.h"
#include "r_efx.h"

#include "../hud_iface.h"
#include "../com_weapons.h"
//#include "../demo.h"

//Master Sword
#include "inc_huditem.h"
#include "HUDMisc.h"
#include "HUDScript.h"
#include "Fatigue.h"
#include "CLGlobal.h"
#include "script.h"
#include "Menu.h"
#include "Stats\stats.h"
#include "logfile.h"
#include "MSCharacter.h"
#include "../vgui_ScorePanel.h"
#include "vgui_HUD.h"
#include "Action.h"

void ShowVGUIMenu(int iMenu);
extern int g_SwitchToHand;
extern int g_TempEntCount;
void CAM_ToFirstPerson();
void CAM_ToThirdPerson();
void ContainerWindowOpen(ulong ContainerID);

//------------------
// The entity we'll use to represent the local client
CBasePlayer player;
int playerBodyArray[16]; //MiB MAR2010_12 Armor Fix FINAL

//Master Sword player functionality
void ShowWeaponDesc(CGenericItem *pItem);
void Storage_Show(msstring_ref DisplayName, msstring_ref StorageName, float flFeeRatio);
void UpdateActiveMenus();

/*
=====================
CBaseEntity :: Killed

If weapons code "kills" an entity, just set its effects to EF_NODRAW
=====================
*/
void CBaseEntity ::Killed(entvars_t *pevAttacker, int iGib)
{
	pev->effects |= EF_NODRAW;
}

/*
=====================
CBasePlayer::Killed

=====================
*/
void CBasePlayer::Killed(entvars_t *pevAttacker, int iGib)
{
	//Referencing pev here causes crashes... no time to find out why
	if (m_CharacterState != CHARSTATE_LOADED)
		return;

	ShowVGUIMenu(0);
	pev->deadflag = DEAD_DEAD;
}

/*
=====================
CBasePlayer::Spawn

=====================
*/
void CBasePlayer::Spawn(void)
{
	Stamina = player.MaxStamina();
	m_PrefHand = RIGHT_HAND;
	m_CurrentHand = m_PrefHand;
	//m_flNextAttack = 0;
	mLastGlowColor[0] = 0;
	pev->deadflag = DEAD_NO;

	pev->nextthink = gpGlobals->time;
	BlockButton(IN_ATTACK); //Make it inconsequential if the player is still holding down the button

	//Always keep the actual view model at null.mdl
	//This is the 'real' HL viewmodel, which is never shown, but always needs a model or it won't hit the renderer
	//What's actually show on the screen are two different models based on what's in your hands
	gEngfuncs.CL_LoadModel("models/null.mdl", &player.pev->viewmodel);

	//Reset some variables
	pbs.fMaxForwardPressTime = 0;

	gHUD.m_HUDScript->CallScriptEvent("client_spawn");

	ShowVGUIMenu(0); //Remove the blank screen VGUI
}

int CL_IsDead(void)
{
	//Master Sword - Health is global now
	return !player.IsAlive();
}

void CBasePlayer::Think(void)
{
	startdbg;

	dbg("Begin");
	cl_entity_s *clplayer = gEngfuncs.GetLocalPlayer();
	pev->iuser1 = clplayer->index;
	pev->origin = clplayer->origin;
	pev->angles = clplayer->angles;
	pev->v_angle = clplayer->angles;

	msstring vsGlowColor = CVAR_GET_STRING("ms_glowcolor");
	if(!FStrEq(mLastGlowColor,vsGlowColor))
	{
		strncpy(mLastGlowColor, vsGlowColor, sizeof(mLastGlowColor));
		ServerCmd(UTIL_VarArgs("SetGlowColor \"%s\"\n", vsGlowColor.c_str()));
	}

	//g_flHealth = m_HP; //For code that doesn't include entity headers

	dbg("Call CheckSpeed");
	CheckSpeed();

	dbg("Call gHUD.m_Fatigue->DoThink");
	gHUD.m_Fatigue->DoThink();

	dbg("Call Script Event game_think");
	gHUD.m_HUDScript->CallScriptEvent("game_think");

	//If I auto-used an item, return to original item when the attack is done
	/*if( g_SwitchToHand > -1 )
	{
		if( player.m_CurrentHand == g_SwitchToHand || !player.ActiveItem() ) g_SwitchToHand = -1;
		else if( player.ActiveItem() && !player.ActiveItem()->pev->iuser1 )
		{
			if( player.SwitchHands( g_SwitchToHand, false ) )
			{
				char cCmd[16];
				 _snprintf(cCmd, sizeof(cCmd),  "hand %i\n",  g_SwitchToHand );
				gEngfuncs.pfnServerCmd( cCmd );
			}
			g_SwitchToHand = -1;
		}
	}*/

	//dbg( "Autosave" );
	//MSChar_Interface::AutoSave( this );

	//	Print( "Entities: %i\n", MS_CLGlobals->num_ents );
	pev->nextthink = gpGlobals->time;

	enddbg;
}

const char *CBasePlayer::TeamID(void) { return GetPartyName(); }
const char *CBasePlayer::GetPartyName(void)
{
	int PlayerIdx = MSCLGlobals::GetLocalPlayerIndex();
	return g_PlayerExtraInfo[PlayerIdx].teamname;
}
ulong CBasePlayer::GetPartyID(void)
{
	int PlayerIdx = MSCLGlobals::GetLocalPlayerIndex();
	return g_PlayerExtraInfo[PlayerIdx].TeamID;
}

/*
=====================
UTIL_TraceLine

Don't actually trace, but act like the trace didn't hit anything.
=====================
*/
void UTIL_TraceLine(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr)
{
	memset(ptr, 0, sizeof(*ptr));
	ptr->flFraction = 1.0;
}

/*
=====================
UTIL_ParticleBox

For debugging, draw a box around a player made out of particles
=====================
*/
void UTIL_ParticleBox(CBasePlayer *player, float *mins, float *maxs, float life, unsigned char r, unsigned char g, unsigned char b)
{
	int i;
	vec3_t mmin, mmax;

	for (i = 0; i < 3; i++)
	{
		mmin[i] = player->pev->origin[i] + mins[i];
		mmax[i] = player->pev->origin[i] + maxs[i];
	}

	gEngfuncs.pEfxAPI->R_ParticleBox((float *)&mmin, (float *)&mmax, 5.0, 0, 255, 0);
}

/*
=====================
UTIL_ParticleBoxes

For debugging, draw boxes for other collidable players
=====================
*/
void UTIL_ParticleBoxes(void)
{
	int idx;
	physent_t *pe;
	cl_entity_t *player;
	vec3_t mins, maxs;

	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	player = gEngfuncs.GetLocalPlayer();
	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(player->index - 1);

	for (idx = 1; idx < 100; idx++)
	{
		pe = gEngfuncs.pEventAPI->EV_GetPhysent(idx);
		if (!pe)
			break;

		if (pe->info >= 1 && pe->info <= gEngfuncs.GetMaxClients())
		{
			mins = pe->origin + pe->mins;
			maxs = pe->origin + pe->maxs;

			gEngfuncs.pEfxAPI->R_ParticleBox((float *)&mins, (float *)&maxs, 0, 0, 255, 2.0);
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

physent_t *MSUTIL_EntityByIndex(int playerindex)
{
	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);
	gEngfuncs.pEventAPI->EV_PushPMStates();
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(player.pev->iuser1 - 1);

	for (int idx = 1; idx < 256; idx++)
	{
		physent_t *pe = gEngfuncs.pEventAPI->EV_GetPhysent(idx);
		if (!pe)
			break;

		if (pe->info == playerindex)
		{
			gEngfuncs.pEventAPI->EV_PopPMStates();
			return pe;
		}
	}
	gEngfuncs.pEventAPI->EV_PopPMStates();
	return NULL;
}
/*
=====================
UTIL_ParticleLine

For debugging, draw a line made out of particles
=====================
*/
void UTIL_ParticleLine(CBasePlayer *player, float *start, float *end, float life, unsigned char r, unsigned char g, unsigned char b)
{
	gEngfuncs.pEfxAPI->R_ParticleLine(start, end, r, g, b, life);
}

/*
=====================
HUD_WeaponsPostThink

Run ItemPostFrame() as nessesary
=====================
*/
void HUD_WeaponsPostThink(local_state_s *from, local_state_s *to, usercmd_t *cmd, double time, unsigned int random_seed)
{
	startdbg;
	dbg("Begin");

	if (player.m_CharacterState == CHARSTATE_UNLOADED)
		return;

	int buttonsChanged;

	// Get current clock
	//gpGlobals->time = time;  //UNDONE -- Run time even when lag is happening

	//This button code must run every frame regaurdless of whether the weapon is client-side

	// Which buttons chave changed
	buttonsChanged = (player.m_afButtonLast ^ cmd->buttons); // These buttons have changed this frame

	// Debounced button codes for pressed/released
	// The changed ones still down are "pressed"
	player.m_afButtonPressed = buttonsChanged & cmd->buttons;
	// The ones not down are "released"
	player.m_afButtonReleased = buttonsChanged & (~cmd->buttons);

	player.pev->button = cmd->buttons;

	dbg("SetKeys");
	player.SetKeys();

	if (FBitSet(player.pbs.ButtonsDown, IN_ATTACK))
	{
		if (QuickSlotConfirm())			   //User pressed attack to confirm a quickslot selection
			player.BlockButton(IN_ATTACK); //Don't attack until he releases the button
	}

	//Takes blocked buttons into account
	bool fButtonsChanged = (player.pbs.ButtonsDown != player.m_afButtonLast);

	if (fButtonsChanged)
	{
		//memcpy( &KeyHistory[1], &KeyHistory[0], sizeof(KeyHistory) - sizeof(KeyHistory[0]) );
		for (int i = (MAX_KEYHISTORY - 2); i >= 0; i--)
			KeyHistory[i + 1] = KeyHistory[i];

		//int buttons = cmd->buttons;

		KeyHistory[0].Buttons = player.pbs.ButtonsDown;
		KeyHistory[0].Time = gpGlobals->time;
	}

	//Status info from server... only certain values are copied
	/*if( from->client.iuser3 & PLAYER_MOVE_SITTING )
	{
		//Sitting automatically switches to third person
		if( !FBitSet(player.m_StatusFlags,PLAYER_MOVE_SITTING) )
			CAM_ToThirdPerson( );
	}
	else
	{
		//Standing automatically switches to first person
		if( FBitSet(player.m_StatusFlags,PLAYER_MOVE_SITTING) )
			CAM_ToFirstPerson( );
		ClearBits( player.m_StatusFlags, PLAYER_MOVE_SITTING );
	}*/

	//Status info from server... only certain values are copied
	static int PreserveMask = PLAYER_MOVE_RUNNING | PLAYER_MOVE_ATTACKING; //These are client-side bits to be saved
	int PreserveBits = (player.m_StatusFlags & PreserveMask);			   //Save the client-side bits

	player.m_StatusFlags = ClearBits(from->client.iuser3, PreserveMask); //Copy all the flags except the client-side ones
	SetBits(player.m_StatusFlags, PreserveBits);						 //Copy the client-side bits back over

	to->playerstate.iuser3 = from->playerstate.iuser3;

	//if( !player.IsAlive() ) return;

	dbg("Set variables");

	// For random weapon events, use this seed to seed random # generator
	player.random_seed = random_seed;

	// Set player variables that weapons code might check/alter

	player.pev->velocity = from->client.velocity;
	player.pev->flags = from->client.flags;

	player.pev->waterlevel = from->client.waterlevel;

	//player.pev->maxspeed - Just a multiplier for the client speed
	//since the server cannot know the client's current speed
	//(It changes with stamina, running, etc. client-only events)
	player.pev->maxspeed = from->client.maxspeed;

	player.pev->fov = from->client.fov;
	//	player.pev->weaponanim = from->client.weaponanim;
	//	player.pev->viewmodel = from->client.viewmodel;
	//	player.m_flNextAttack = from->client.m_flNextAttack;

	// Store pointer to our destination entity_state_t so we can get our origin, etc. from it
	//  for setting up events on the client
	g_finalstate = to;

	//Catch a weapon switch
	//dbg( "CheckHandSwitch" );
	//player.CheckHandSwitch( );

	//dbg( "Player Think" );
	//player.Think( );

	/*
	UNDONE -- Think is now called normally on the hand items
	if( player.IsAlive() )
	{
		if( pItem ) ;//pItem->ItemPostFrame();
		else
		{
			//Use player hands..
			if( player.PlayerHands )
				player.PlayerHands->ItemPostFrame( );
		}
	}*/

	// Assume that we are not going to switch weapons
	dbg("Preserve states");
	to->client.m_iId = from->client.m_iId;

	// Copy in results of prediction code

	to->client.flags = player.pev->flags;
	to->client.viewmodel = player.pev->viewmodel;
	to->client.weaponanim = player.pev->weaponanim;
	//	to->client.viewmodel				= from->client.viewmodel;
	//	to->client.weaponanim				= from->client.weaponanim;
	to->client.fov = player.pev->fov;
	//	to->client.m_flNextAttack			= player.m_flNextAttack;

	//Must preserve maxspeed.  This is a percentage sent from the server indicating
	//how fast the client can move (used by sludge, stuns, etc.)
	to->client.maxspeed = from->client.maxspeed;
	player.m_afButtonLast = KeyHistory[0].Buttons; //player.pev->button;

	// Wipe it so we can't use it after this frame
	g_finalstate = NULL;

	enddbg;
}
bool IsPlayerActing()
{
	return player.IsActing() ? true : false;
}
void Player_UseStamina(float flAddAmt)
{
	player.Stamina -= flAddAmt;
	player.Stamina = max(player.Stamina, 0);
	float MaxStamina = player.MaxStamina();
	player.Stamina = min(player.Stamina, MaxStamina);
}
void Player_DoJump()
{
	//Use up stamina - called from the physics code
	//This function is here so other factor can play into the
	//loss for jumping ... TODO: Consider weight?
	int Weight = player.Weight();
	float FilledRatio = Weight / player.Volume();
	FilledRatio = min(FilledRatio, 1.0f);
	int JumpEnergy = FilledRatio * 4;
	Player_UseStamina(JumpEnergy);
}
/*
=====================
HUD_PostRunCmd

Client calls this during prediction, after it has moved the player and updated any info changed into to->
time is the current client clock based on prediction
cmd is the command that caused the movement, etc
runfuncs is 1 if this is the first time we've predicted this command.  If so, sounds and effects should play, otherwise, they should
be ignored
=====================
*/
void _DLLEXPORT HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed)
{
	DBG_INPUT;
	startdbg;
	g_runfuncs = runfuncs;

	// Only run post think on client-predicted frames, not the server
	// sync frames (which are usually late)
	if (runfuncs)
		HUD_WeaponsPostThink(from, to, cmd, time, random_seed);

	enddbg;
	//Print( "%f %f / %f %f\n", from->playerstate.origin.x, from->playerstate.origin.y, to->playerstate.origin.x, to->playerstate.origin.y );
}

void CBasePlayer::CheckSpeed()
{
	CheckRun();

	//Normal speed (factoring in stamina, stuck arrows, etc.)
	fSpeed = player.CurrentSpeed();

	//Server maxspeed == % of normal speed that player should go
	if (player.pev->maxspeed)
		fSpeed *= (player.pev->maxspeed / 100.0f);

	m_MaxSpeed = max(fSpeed, 0.001);

	gEngfuncs.Cvar_SetValue("cl_forwardspeed", fSpeed);
	//if( player.Class && player.Class->id == CLASS_ROGUE )
	//Rogues can move backwards almost as fast as forwards
	//	gEngfuncs.Cvar_SetValue( "cl_backspeed", fSpeed * 0.9 );
	//else
	gEngfuncs.Cvar_SetValue("cl_backspeed", fSpeed * 0.5);
	gEngfuncs.Cvar_SetValue("cl_sidespeed", fSpeed * 0.8);
}

void CBasePlayer::CheckRun()
{
	//Master Sword: Check if the player is trying to run.
	//You have to let go of forward before 0.1 secs and press it again by 0.4 secs
	if (pev->deadflag != DEAD_NO)
		return;

	if (FBitSet(pbs.ButtonsDown, IN_FORWARD))
	{
		//MIB MAR2012_15 - switching run from double tap to +shift
		if ((FBitSet(pbs.ButtonsDown, IN_RUN) || (pbs.fMaxForwardPressTime > 0 && gpGlobals->time < pbs.fMaxForwardPressTime)) &&
			!player.pev->maxspeed &&
			!FBitSet(m_StatusFlags, PLAYER_MOVE_RUNNING) &&
			!FBitSet(m_StatusFlags, PLAYER_MOVE_ATTACKING) &&
			!FBitSet(m_StatusFlags, PLAYER_MOVE_NORUN))
		{
			if (player.Stamina > 1)
			{
				SendEventMsg("You break into a jog.");
				SetBits(m_StatusFlags, PLAYER_MOVE_RUNNING);
			}
			else
			{
				SendEventMsg(HUDEVENT_UNABLE, "You are too exhausted to run.");
			}
			pbs.fMaxForwardPressTime = 0;
		}
		else if (FBitSet(m_StatusFlags, PLAYER_MOVE_RUNNING)) //Run until too tired or hit something
		{
			if (player.Stamina <= 0)
			{ // Too Tired to go on
				ClearBits(m_StatusFlags, PLAYER_MOVE_RUNNING);
				SendEventMsg(HUDEVENT_UNABLE, "You are too exhausted to continue running.");
			}
			else
			{
				Player_UseStamina(0.6 * gpGlobals->frametime);		  //Get tired from running
				if (player.pev->velocity.Length2D() < fLastSpeed - 50 //If you lose too much speed between this frame
																	  //and the last, you've hit something
					|| FBitSet(player.pev->flags, FL_DUCKING)		  //If you duck, stop
					|| FBitSet(m_StatusFlags, PLAYER_MOVE_ATTACKING)  //If you attack, stop
				)
				{

					ClearBits(m_StatusFlags, PLAYER_MOVE_RUNNING);
					SendEventMsg(HUDEVENT_UNABLE, "You lose your running speed.");
					//SendInfoMsg( "You lose your running speed.\n" );
				}
			}
		}
		else
		{ //You're just walking or maybe tapping the button the first time
			if (!pbs.fMaxForwardPressTime)
				pbs.fMaxForwardPressTime = -(gpGlobals->time + 0.4);
		}
	}
	else
	{
		if (m_StatusFlags & PLAYER_MOVE_RUNNING)
		{ //Stop running
			m_StatusFlags &= ~PLAYER_MOVE_RUNNING;
			SendEventMsg("You slow down and begin walking casually.");
			//SendInfoMsg( "You slow down and begin walking casually.\n" );
		}
		if (pbs.fMaxForwardPressTime)
		{
			if (gpGlobals->time > pbs.fMaxForwardPressTime * (pbs.fMaxForwardPressTime < 0 ? -1 : 1))
				pbs.fMaxForwardPressTime = 0;
			else if (pbs.fMaxForwardPressTime < 0)
			{
				if (gpGlobals->time < pbs.fMaxForwardPressTime * (pbs.fMaxForwardPressTime < 0 ? -1 : 1) - 0.2)
					pbs.fMaxForwardPressTime *= -1;
				else
					pbs.fMaxForwardPressTime = 0;
			}
		}
	}

	fLastSpeed = player.pev->velocity.Length2D();
}

bool CBasePlayer::CheckHandSwitch() //No longer called.  Client now sends 'hand' command directly
{
	if (!FBitSet(pbs.ButtonsDown, IN_ATTACK2))
		return false;

	//BlockButton( IN_ATTACK2 );	//Block the button regardless of success or failure to switch hands

	//if( !player.SwitchHands( !player.m_CurrentHand ) )
	//	return false;

	//I switched hands, update the server
	//gEngfuncs.pfnServerCmd( msstring("hand ") + int(!player.m_CurrentHand) + "\n" );

	return true;
}
void ShowWeaponDesc(CGenericItem *pItem)
{
	if (!pItem)
		return;

	static char cDescString[256];

	if (!pItem->DisplayDesc.len())
		 strncpy(cDescString,  "No Description", sizeof(cDescString) );
	else
	{
		char amt[16] = {""};
		if (FBitSet(pItem->MSProperties(), ITEM_GROUPABLE) && pItem->iQuantity > 1)
			 _snprintf(amt, sizeof(amt),  " (%i)",  pItem->iQuantity );

		 _snprintf(cDescString, sizeof(cDescString),  "%s%s",  pItem->DisplayDesc.c_str(),  amt );
	}
	static client_textmessage_t msg;
	msg.effect = 1;
	msg.r1 = msg.g1 = msg.b1 = 128;
	msg.r2 = msg.g2 = msg.b2 = 0;
	msg.x = MSCLGlobals::DefaultHUDCoords.ItemDesc_X;
	msg.y = MSCLGlobals::DefaultHUDCoords.ItemDesc_Y;
	msg.fadein = 2.0;
	msg.holdtime = 5.0;
	msg.fadeout = 3.0;
	msg.pName = NULL;
	msg.pMessage = cDescString;
	gHUD.m_Message.MessageAdd(msg);
}
void __CmdFunc_PlayerDesc(void)
{
	//Show weapon descriptions
	ShowWeaponDesc(player.ActiveItem());
}
//Handles all inventory messages
int __MsgFunc_Item(const char *pszName, int iSize, void *pbuf)
{
	startdbg;
	dbg("Begin");

	BEGIN_READ(pbuf, iSize);
	byte Operation = READ_BYTE();

	switch (Operation)
	{
	//New item
	case 0:
	{
		dbg("Add item");
		CGenericItem *pItem = ReadGenericItem(true);
		if (!pItem)
			MSErrorConsoleText("__MsgFunc_Item()", "Got 'add item' msg but client couldn't create item");

		//Add the item, without any checks (free hand, weight, etc.)
		if (player.AddItem(pItem, (pItem->m_Location == ITEMPOS_HANDS), false, pItem->m_Hand))
		{
			if (pItem->m_Location > ITEMPOS_HANDS)
			{
				//Thothie FEB2010_01 Pass gender/race with wear
				static msstringlist Params;
				Params.clearitems();
				Params.add(pItem->m_pOwner->m_Race);
				Params.add((pItem->m_pOwner->m_Gender == GENDER_MALE) ? "male" : "female"); //Thothie: This returns wrong here
				Params.add("__MsgFunc_Item_Add");
				pItem->CallScriptEvent("game_wear", &Params);
			}
		}
		else
		{
			pItem->SUB_Remove();
			MSErrorConsoleText("__MsgFunc_Item()", msstring("Got 'add item' msg but client didn't accept item ") + pItem->DisplayName());
		}
	}
	break;

	//Update existing item
	case 1:
	{
		dbg("Update item");
		ulong lID = READ_LONG();
		READ_REWIND_LONG(); //I read the ID, put the offset back so that ReadGenericItem() can read it
		CGenericItem *pItem = MSUtil_GetItemByID(lID);
		if (pItem)
		{
			int LastLoc = pItem->m_Location;
			pItem = ReadGenericItem(false);
			if (LastLoc == ITEMPOS_HANDS && pItem->m_Location > ITEMPOS_HANDS)
			{
				//Thothie FEB2010_01 Pass gender/race with wear
				static msstringlist Params;
				Params.clearitems();
				Params.add(pItem->m_pOwner->m_Race);
				Params.add((pItem->m_pOwner->m_Gender == GENDER_MALE) ? "male" : "female");
				Params.add("__MsgFunc_Item_Update");
				pItem->CallScriptEvent("game_wear", &Params);
			}
		}

		if (!pItem)
			MSErrorConsoleText("__MsgFunc_Item()", "Got 'update item' msg but client couldn't find item");
	}
	break;

	//Remove item
	case 2:
	{
		dbg("Remove item");
		ulong ItemID = READ_LONG();
		CGenericItem *pItem = MSUtil_GetItemByID(ItemID, &player);

		if (pItem)
		{
			//Must check if item is still on the player...
			//Adding the item to a container will send 'add to container' first, then send this message to remove the item from the gear list.
			//If the item has already been removed from the gear list, just do nothing
			if (player.Gear.ItemExists(pItem))
			{
				player.RemoveItem(pItem);
				pItem->SUB_Remove();
			}
		}
		else
			MSErrorConsoleText("__MsgFunc_Item()", msstring("Non-Fatal: Got 'remove item' msg but item ") + int(ItemID) + " not found.");
	}
	break;

	//Add to container
	case 3:
	{
		dbg("Add item to container");
		ulong ContainerID = READ_LONG();
		dbg("Read Long");
		CGenericItem *pContainer = MSUtil_GetItemByID(ContainerID, &player);
		dbg("get pContainer");
		if (!pContainer)
			MSErrorConsoleText("__MsgFunc_Item()", msstring("Got 'add to container' msg but client couldn't find container") + (int)ContainerID);

		dbg("get pItem");
		CGenericItem *pItem = ReadGenericItem(true);
		if (!pItem)
			MSErrorConsoleText("__MsgFunc_Item()", "Got 'add to container' msg but client couldn't find item");

		if (!pItem->PutInPack(pContainer))
			MSErrorConsoleText("__MsgFunc_Item()", msstring("Got 'add to container' msg but client couldn't put ") + pItem->DisplayName() + " in " + pContainer->DisplayName());
	}
	break;

	//Remove from container
	case 4:
	{
		dbg("Remove item from container");
		ulong ContainerID = READ_LONG();
		CGenericItem *pContainer = MSUtil_GetItemByID(ContainerID, &player);
		if (!pContainer)
			MSErrorConsoleText("__MsgFunc_Item()", msstring("Got 'remove from container' msg but client couldn't find container") + (int)ContainerID);

		ulong ItemID = READ_LONG();
		CGenericItem *pItem = MSUtil_GetItemByID(ItemID);
		if (!pItem)
			MSErrorConsoleText("__MsgFunc_Item()", "Got 'remove from container' msg but client couldn't find item");

		if (!pContainer->Container_RemoveItem(pItem))
			MSErrorConsoleText("__MsgFunc_Item()", "Non-Fatal: Got 'remove from container' msg but item was not found in container");
		pItem->SUB_Remove();
	}
	break;

	//Show storage
	case 5:
	{
		dbg("Show storage window");
		msstring DisplayName = READ_STRING();
		msstring StorageName = READ_STRING();
		float flFeeRatio = READ_FLOAT();
		Storage_Show(DisplayName, StorageName, flFeeRatio);
	}
	break;

	case 6:
	{
		//MiB DEC2007a - Redundant skool of redundancy
		// Params: ID, Attacknum, prop, value
		dbg("AttackProps");
		ulong lID = READ_LONG();
		CGenericItem *pItem = MSUtil_GetItemByID(lID);

		int attackNum = READ_BYTE();
		attackdata_t *AttData = &pItem->m_Attacks[attackNum];

		msstring PropName = READ_STRING();
		msstring PropValue = READ_STRING();

		int Int = atoi(PropValue);
		float Float = atof(PropValue);
		Vector Vec = StringToVec(PropValue);
		bool Bool = PropValue == "true" || PropValue == "1";

		if (PropName == "type")
			AttData->sDamageType = PropValue;
		else if (PropName == "range")
			AttData->flRange = Float;
		else if (PropName == "dmg")
			AttData->flDamage = Float;
		else if (PropName == "dmg.range")
			AttData->flDamageRange = Float;
		else if (PropName == "dmg.type")
			AttData->sDamageType = PropValue;
		else if (PropName == "dmg.multi")
			AttData->f1DmgMulti = Float;
		else if (PropName == "aoe.range")
			AttData->flDamageAOERange = Float;
		else if (PropName == "aoe.falloff")
			AttData->flDamageAOEAttn = Float;
		else if (PropName == "energydrain")
			AttData->flEnergy = Float;
		else if (PropName == "mpdrain")
			AttData->flMPDrain = Float;
		else if (PropName == "stat.prof")
			AttData->StatProf = Int;
		else if (PropName == "stat.balance")
			AttData->StatBalance = Int;
		else if (PropName == "stat.power")
			AttData->StatPower = Int;
		else if (PropName == "stat.exp")
			AttData->StatExp = Int;
		else if (PropName == "noise")
			AttData->flNoise = Float;
		else if (PropName == "callback")
			AttData->CallbackName = PropValue;
		else if (PropName == "ofs.startpos")
			AttData->StartOffset = Vec;
		else if (PropName == "ofs.aimang")
			AttData->AimOffset = Vec;
		else if (PropName == "priority")
			AttData->iPriority = Int;
		else if (PropName == "dmg.ignore")
			AttData->NoDamage = Bool;
		else if (PropName == "hitchance")
			AttData->flAccuracyDefault = Float;
		else if (PropName == "delay.end")
			AttData->tDuration = Float;
		else if (PropName == "delay.strike")
			AttData->tLandDelay = Float;
		else if (PropName == "dmg.noautoaim")
			AttData->NoAutoAim = Bool;
		else if (PropName == "ammodrain")
			AttData->iAmmoDrain = Int;
		else if (PropName == "hold_min")
			AttData->tProjMinHold = Float;
		else if (PropName == "hold_max")
			AttData->tMaxHold = Float;
		else if (PropName == "projectile")
			AttData->sProjectileType = PropValue;
		else if (PropName == "COF.l")
			AttData->flAccuracyDefault = Float;
		else if (PropName == "COF.r")
			AttData->flAccBest = Float;
		else if (PropName == "delay.end")
			AttData->tDuration = Float;
		else if (PropName == "reqskill")
			AttData->RequiredSkill = Int;
	}
	break;

	case 7:
		//Shuriken FEB2008a - setviewmodelprop
		//setviewmodelprop edits APR2008a MIB
		{
			dbg("setviewmodelprop");
			msstring Mode = READ_STRING();
			int iHand = READ_SHORT();
			int iParam1;
			float fParam1;
			msstring sParam1;
			if (Mode == "model")
			{
				sParam1 = READ_STRING();
			}
			else
			{
				if (Mode == "animspeed")
				{
					fParam1 = READ_FLOAT();
				}
				else
				{
					iParam1 = READ_BYTE();
				}
			}

			CGenericItem *pItem = player.Hand(iHand);

			if (Mode == "submodel")
			{

				pItem->m_ViewModelPart = iParam1;
				pItem->m_ViewModelSubmodel = READ_BYTE();
				//pItem->m_ViewModelBody = Param1;
			}
			else if (Mode == "skin")
				pItem->m_Skin = iParam1;
			else if (Mode == "rendermode")
				pItem->m_RenderMode = iParam1;
			else if (Mode == "renderfx")
				pItem->m_RenderFx = iParam1;
			else if (Mode == "renderamt")
				pItem->m_RenderAmt = iParam1;
			else if (Mode == "model")
			{
				//Print( "%s\n", sParam1.c_str() );
				pItem->m_ViewModel = sParam1;
			}
			else if (Mode == "animspeed")
			{
				//Thothie OCT2011_30 viewnim_speed
				Print("DEBUG: cl_got_animspeed %f\n", fParam1);
				pItem->m_ViewModelAnimSpeed = fParam1;
			}
		}
		break;

	//MIB FEB2010_03 - Clit event
	case 8:
	{
		ulong lID = READ_LONG();
		CGenericItem *pItem = MSUtil_GetItemByID(lID);

		msstring Event = READ_STRING();
		int numParams = READ_BYTE();

		msstringlist Params;
		Params.clear();
		for (int i = 0; i < numParams; i++)
			Params.add(READ_STRING());

		pItem->CallScriptEvent(Event, &Params);
	}
	break;
	}

	dbg("UpdateActiveMenus()");

	UpdateActiveMenus();

	enddbg;
	return 1;
}

//Modded by MiB JAN2010_15
int __MsgFunc_SetProp(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int Prop = READ_BYTE();
	//
	//	Please continue to add to this table as we add more props.
	//	Index: Prop     - Inputs						Explaination
	//	0: Title		- String						Change Title
	//  1: Name			- String						Change Name
	//  2: Spells(1)	- Int<index> String<spellname>	Replace a spell
	//	2: Spells(0)	- Int<index>					Remove a spell
	//	2: Spells(-1)	- None							Erase all spells

	switch (Prop)
	{
	case PROP_TITLE:
		player.CustomTitle = READ_STRING();
		break;
	case PROP_NAME:
		player.m_DisplayName = READ_STRING();
		break;
	case PROP_SPELL:
	{
		int type = READ_BYTE();
		if (type == 1)
		{
			int idx = READ_BYTE();
			player.m_SpellList[idx] = READ_STRING(); //Overwrite the spell with the spell provided
		}
		else if (type == 0)
		{
			//Memory dellocation errors when using '.erase()', so I'm now using this loop
			int idx = READ_BYTE();
			msstringlist newSpells;
			for (int i = 0; i < player.m_SpellList.size(); i++)
				if (i != idx)
					newSpells.add(player.m_SpellList[i]); //Add all spells to the new list except the one to erase
			player.m_SpellList = newSpells;				  //Overwrite the player spells with the new list
		}
		else if (type == -1)
		{
			player.m_SpellList.clearitems();
		}
		break;
	}
	case PROP_ANIMSPEEDADJ:
	{
		player.m_AnimSpeedAdj = READ_FLOAT();
		break;
	}
	}

	return 1;
}

void Player_OpenInventory()
{
	if (player.m_CharacterState == CHARSTATE_UNLOADED)
		return;
	if (gHUD.m_iHideHUDDisplay & HIDEHUD_ALL)
		return;

	CGenericItem *pWearable = NULL; //Fallback, in case a pack isn't found
	for (int i = 0; i < player.Gear.size(); i++)
	{
		CGenericItem *pPack = player.Gear[i];
		if (FBitSet(pPack->MSProperties(), ITEM_CONTAINER) && pPack->m_Location > ITEMPOS_HANDS)
		{
			pPack->Container_Open();
			pPack->ListContents();
			gEngfuncs.pfnServerCmd(UTIL_VarArgs("inv open %i\n", pPack->m_iId));
			break;
		}
		else if (FBitSet(pPack->MSProperties(), ITEM_WEARABLE) && !pWearable)
			pWearable = pPack; //Open this as a last resort
	}

	if (pWearable)
		pWearable->ListContents(); //No pack was found.  Open to any wearable
	else
		ContainerWindowOpen(0); //No wearable was found, Open to player hands
}
void __CmdFunc_Inv() { Player_OpenInventory(); }
/*void __CmdFunc_Slot1( ) { Player_SelectSlot( 1 ); }
void __CmdFunc_Slot2( ) { Player_SelectSlot( 2 ); }
void __CmdFunc_Slot3( ) { Player_SelectSlot( 3 ); }
void __CmdFunc_Slot4( ) { Player_SelectSlot( 4 ); }
void __CmdFunc_Slot5( ) { Player_SelectSlot( 5 ); }
void __CmdFunc_Slot6( ) { Player_SelectSlot( 6 ); }
void __CmdFunc_Slot7( ) { Player_SelectSlot( 7 ); }
void __CmdFunc_Slot8( ) { Player_SelectSlot( 8 ); }
void __CmdFunc_Slot9( ) { Player_SelectSlot( 9 ); }
void __CmdFunc_Slot10( ) { Player_SelectSlot( 10 ); }*/

int __MsgFunc_HideWeapon(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	gHUD.m_iHideHUDDisplay = READ_BYTE();

	/*if ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
	{
		static wrect_t nullrc;
		SetCrosshair( 0, nullrc, 0, 0, 0 );
	}
	else
	{
		if ( m_pWeapon )
			SetCrosshair( m_pWeapon->hCrosshair, m_pWeapon->rcCrosshair, 255, 255, 255 );
	}*/

	return 1;
}

void __CmdFunc_BindTeleport(void)
{
#ifdef DEV_BUILD
	if (gEngfuncs.Cmd_Argc() < 2)
		return;

	msstring Key = gEngfuncs.Cmd_Argv(1);
	msstring BindStr;
	cl_entity_t *clplayer = gEngfuncs.GetLocalPlayer();
	if (clplayer)
	{
		_snprintf(BindStr.c_str(), MSSTRING_SIZE, "bind %s \"teleport %.2f %.2f %.2f\"\n", Key.c_str(), clplayer->origin.x, clplayer->origin.y, clplayer->origin.z);
		gEngfuncs.pfnClientCmd(BindStr);
	}
#endif
}
void DynamicNPC_SelectMenuItem(int idx, TCallbackMenu *pcbMenu)
{
	msstringlist NPCList;
	TokenizeString(gEngfuncs.pfnGetCvarString("ms_dynamicnpc"), NPCList);
	if (idx < 0 || idx >= (signed)NPCList.size())
		return;
	gEngfuncs.pfnClientCmd(UTIL_VarArgs("npc %s\n", NPCList[idx].c_str()));
}
void __CmdFunc_DynamicNPC(void)
{
	//byte *pMem = player.m_Gold.DataLoc_Get( );
	//pMem[1] = 233;

	char MenuText[2048] = "Spawn NPC:\n\n";

	char *pszNPCList = gEngfuncs.pfnGetCvarString("ms_dynamicnpc");

	if (!pszNPCList)
	{
		Print("Error: ms_dynamicnpc CVAR not registered!\n");
		return;
	}

	if (gHUD.m_Menu->HideMyMenu(MENU_DYNAMICNPC))
		return;

	msstringlist NPCList;
	TokenizeString(pszNPCList, NPCList);

	if (!NPCList.size())
	{
		Print("No NPCs to spawn! (ms_dynamicnpc == '%s')", pszNPCList);
		return;
	}

	int iBitsValid = 0;
	for (int i = 0; i < NPCList.size(); i++)
	{
		const char *arg = UTIL_VarArgs("%i. %s\n", i + 1, NPCList[i].c_str());
		strncat(MenuText, arg, strlen(arg));
		iBitsValid |= (1 << i);
	}

	gHUD.m_Menu->ShowMenu(iBitsValid, MenuText, DynamicNPC_SelectMenuItem, MENU_DYNAMICNPC);
}
void __CmdFunc_DebugPrint(void)
{
	Print("Pos: %.2f %.2f %.2f\n", player.pev->origin.x, player.pev->origin.y, player.pev->origin.z);
	Print("Ang: %.2f %.2f %.2f\n", player.pev->angles.x, player.pev->angles.y, player.pev->angles.z);
	Print("Scripts attached to player (client): %i\n", gHUD.m_HUDScript->m_Scripts.size());
	Print("TempEnts: %i\n", g_TempEntCount);
}
bool ShowHUD()
{
	bool ShowHUD = true;
	if (MSCLGlobals::CharPanelActive)
		ShowHUD = false;
	if (!player.IsAlive())
		ShowHUD = false;
	if (FBitSet(gHUD.m_iHideHUDDisplay, HIDEHUD_ALL))
		ShowHUD = false;
	if (EngineFunc::CVAR_GetFloat("ms_hidehud"))
		ShowHUD = false;

	return ShowHUD;
}
bool ShowHealth() { return player.m_CharacterState == CHARSTATE_LOADED ? ShowHUD() : false; }
bool ShowChat() { return true; } //Always show chat

int __MsgFunc_Hands(const char *pszName, int iSize, void *pbuf)
{
	startdbg;
	dbg("Begin");

	BEGIN_READ(pbuf, iSize);
	player.SwitchHands(READ_BYTE());

	enddbg;
	return 1;
}

int g_SwitchToHand = -1;
void __CmdFunc_ToggleServerBrowser(void);
bool fBroswerVisible();
void ShowVGUIMenu(int iMenu);
void Player_UseStamina(float flAddAmt);
extern float g_fMenuLastClosed;

int __MsgFunc_CLDllFunc(const char *pszName, int iSize, void *pbuf)
{
	startdbg;
	dbg("Begin");

	BEGIN_READ(pbuf, iSize);

	byte Cmd = READ_BYTE();

	dbg(msstring("Cmd: ") + (int)Cmd);
	switch (Cmd)
	{
	case 0: //Spawned
		logfile << "Recieved SPAWN message..." << endl;
		player.m_CharacterState = CHARSTATE_LOADED;

		player.Spawn();
		logfile << "Player Successfully Spawned" << endl;
		g_fMenuLastClosed = 0.0f;
		break;
	case 1: //Killed
		if (player.m_CharacterState == CHARSTATE_UNLOADED)
			break;
		player.Killed(NULL, NULL);
		break;
	case 2: //[OPEN]
		//MSChar_Interface::SaveChar( &player );
		break;
	case 3: //Client  can now change levels (standing in a transition area)
	{
		int iMode = READ_BYTE();
		if (!iMode)
		{
			int flags = READ_BYTE();
			bool fShowBrowser = flags & (1 << 0),
				 fPlaySound = (flags & (1 << 1)) ? true : false;
			 strncpy(player.m_NextMap,  READ_STRING(), sizeof(player.m_NextMap) );
			 strncpy(player.m_OldTransition,  READ_STRING(), sizeof(player.m_OldTransition) );
			 strncpy(player.m_NextTransition,  READ_STRING(), sizeof(player.m_NextTransition) );

			//if( fShowBrowser && !fBroswerVisible() )
			//	__CmdFunc_ToggleServerBrowser( );
			if (fPlaySound)
				PlaySound("misc/transition.wav", 1.0);
			//Print("DEBUG_CLIENT_TRANS: %s", player.m_NextTransition); //MAR2010_08
		}
		else if (iMode == 1)
		{
			if (fBroswerVisible())
				__CmdFunc_ToggleServerBrowser();
			player.m_NextMap[0] = 0;
			player.m_NextTransition[0] = 0;
		}
		//MSChar_Interface::SaveChar( &player );
	}
	break;
	case 4: //Character Name
	{
		player.m_DisplayName = READ_STRING();
	}
	break;
	case 5: //Drain stamina
	{
		Player_UseStamina(READ_LONG());
	}
	break;
	case 6: //This server connected to a Central Server
	{
		int Type = READ_BYTE();
		if (!Type)
		{
			//Not using Central Server
			ChooseChar_Interface::CentralServer = false;
		}
		else if (Type == 1)
		{
			//Server name and online status
			ChooseChar_Interface::CentralServer = true;
			int OnlineStatus = READ_BYTE();
			ChooseChar_Interface::CentralOnline = OnlineStatus ? true : false;
			ChooseChar_Interface::CentralNetworkName = READ_STRING();

			if (OnlineStatus == 1) //Server was connected when player joined
				player.SendHUDMsg("#CENTRALSV_TITLE", "#CENTRALSV_ONLINE");
			else if (OnlineStatus == 2) //Server *just* reconnected after a lost connection
				player.SendHUDMsg("#CENTRALSV_TITLE", "#CENTRALSV_RECONNECTED");
			else
				player.SendHUDMsg("#CENTRALSV_TITLE", "#CENTRALSV_OFFLINE");
		}
		else if (Type == 2)
		{
			//Server online status
			ChooseChar_Interface::CentralServer = true;
			ChooseChar_Interface::CentralOnline = READ_BYTE() ? true : false;
			if (ChooseChar_Interface::CentralOnline)
				player.SendHUDMsg("#CENTRALSV_TITLE", "#CENTRALSV_RECONNECTED");
			else
				player.SendHUDMsg("#CENTRALSV_TITLE", "#CENTRALSV_DISCONNECTED");
		}

		ChooseChar_Interface::UpdateCharScreen();
	}
	break;
	case 7: //I am a god (dev)
	{
		player.m_fIsElite = READ_BYTE() ? true : false;
		//MSChar_Interface::SaveChar( &player );
	}
	break;
	case 8: //Recv number of people I've killed
	{
		player.m_PlayersKilled = READ_SHORT();
	}
	break;
	case 9: //Recv time I've been waiting to forget a kill
	{
		player.m_TimeWaitedToForgetKill = READ_COORD();
	}
	break;
	case 10: //Auto-use weapon (like auto-block with shield)
	{
		int iHand = READ_BYTE(), iAttackNum = READ_BYTE();
		if (player.ActiveItem() && player.ActiveItem()->CurrentAttack)
			player.ActiveItem()->CancelAttack();

		if (player.m_CurrentHand != iHand)
		{
			//Set up a variable to switch back
			if (g_SwitchToHand == -1)
				g_SwitchToHand = player.m_CurrentHand;
			if (player.SwitchHands(iHand, FALSE))
				player.BlockButton(IN_ATTACK);
			else
				g_SwitchToHand = -1;
		}

		CGenericItem *pItem = player.Hand(iHand);
		pItem->StartAttack(iAttackNum);
		if (pItem->CurrentAttack)
			pItem->CurrentAttack->tProjMinHold = 0.50;
	}
	break;
	case 11: //Recieve char data
	{
		MSChar_Interface::HL_CLReadCharData();
	}
	break;
	case 12: //Item gain/lose quality
	{
		CGenericItem *pItem = MSUtil_GetItemByID(READ_LONG());
		if (!pItem)
			break;
		pItem->Quality = READ_SHORT();
	}
	case 13: //Recv time I've been waiting to lose thief status
	{
		player.m_TimeWaitedToForgetSteal = READ_COORD();
	}
	break;
	case 14: //Cancel shield and change quality at the same time
	{
		CGenericItem *pItem = MSUtil_GetItemByID(READ_LONG());
		if (!pItem)
			break;
		pItem->Quality = READ_SHORT();
		pItem->CancelAttack();
		if (player.ActiveItem() == pItem)
			player.BlockButton(IN_ATTACK);
	}
	break;
	case 15: //Open
	{
	}
	break;
	case 16: //Add/Remove a player action
	{
		gHUD.m_Action->MsgFunc_Action(pszName, iSize, pbuf);
	}
	break;
	case 17: //Client-side script
	{
		gHUD.m_HUDScript->MsgFunc_ClientScript(pszName, iSize, pbuf);
	}
	break;
	case 18: //Start receiving char from server
	{
		int CharIdx = READ_BYTE();
		uint Size = READ_LONG();
		MSChar_Interface::HL_CLNewIncomingChar(CharIdx, Size);
	}
	break;
	case 20: //New quickslot was assigned
	{
		int Flags = READ_BYTE();
		int Slot = READ_BYTE();
		int Type = READ_BYTE();
		int ID = READ_LONG();

		quickslot_t &QuickSlot = player.m_QuickSlots[Slot];

		if (Flags & (1 << 0))
		{
			if (Flags & (1 << 1))
			{
				static client_textmessage_t msg;
				msg.effect = 2;
				msg.r1 = msg.g1 = msg.b1 = 128;
				//msg.r2 = msg.g2 = msg.b2 = 230;
				msg.r2 = 255;
				msg.g2 = 230;
				msg.b2 = 0;
				msg.x = 0.08;
				msg.y = 0.72;
				msg.fadein = 0.01;
				msg.holdtime = 3.0;
				msg.fadeout = 1.0;
				msg.pName = NULL;
				msg.fxtime = 4.0;

				static char msgtext[256];
				 _snprintf(msgtext, sizeof(msgtext),  Localized("#QUICKSLOT_CREATE"),  (Slot + 1) );

				msg.pMessage = msgtext;
				gHUD.m_Message.MessageAdd(msg);
			}

			QuickSlot.Active = true;
			QuickSlot.Type = (quickslottype_e)Type;
			QuickSlot.ID = ID;
		}
		else
		{
			QuickSlot.Active = false;
		}
	}
	break;
	case 21: //Retrieve all quickslots (Sent at spawn)
	{
		for (int i = 0; i < MAX_QUICKSLOTS; i++)
		{
			quickslot_t &QuickSlot = player.m_QuickSlots[i];
			QuickSlot.Active = READ_BYTE() ? true : false;
			if (QuickSlot.Active)
			{
				QuickSlot.Type = (quickslottype_e)READ_BYTE();
				QuickSlot.ID = READ_LONG();
			}
		}
	}
	break;

		//22-24: Unused

	case 25: //Open Interact Menu for the specified NPC
	{
		VGUI_ShowMenuInteract();
	}
	break;
	case 26: //A Menu option for the currently selected NPC
	{
		VGUI_AddMenuOption();
	}
	break;
	//MIB MAR2010_12 Armor Fix FINAL
	case 27:
	{
		int player = READ_BYTE();
		int body = READ_BYTE();

		if (player < 16 && player >= 0)
			playerBodyArray[player] = body;
		else
			Print("ERROR: Body index out of bounds (%i)\n", player);
	}
	break;
	}

	enddbg;
	return 1;
}

int __MsgFunc_CLXPlay(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	// int var = READ_BYTE();
	// Same order you did the writing with, same var types all the way
	Vector clx_origin;

	msstring clx_sound = READ_STRING();
	clx_origin.x = READ_COORD();
	clx_origin.y = READ_COORD();
	clx_origin.z = READ_COORD();
	int clx_channel = READ_BYTE();
	float clx_volume = READ_COORD();
	float clx_attn = READ_COORD();
	float clx_pitch = READ_COORD();

	//Print("DEBUG: snd: %s vec:(%f,%f,%f) chan %i vol:%f atn:%f pit:%f",clx_sound.c_str(),clx_origin.x,clx_origin.y,clx_origin.z,clx_channel,clx_volume,clx_attn,clx_pitch);

	gEngfuncs.pEventAPI->EV_PlaySound(0, clx_origin, clx_channel, clx_sound.c_str(), clx_volume, clx_attn, 0, clx_pitch);

	return 1;
}
