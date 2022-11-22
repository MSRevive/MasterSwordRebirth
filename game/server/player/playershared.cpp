#include "msdllheaders.h"
#include "msitemdefs.h"
#include "player.h"
#include "monsters/corpse.h"
#include "stats/stats.h"
#include "stats/statdefs.h"
#include "syntax/syntax.h"
#include "weapons/weapons.h"
#include "weapons/genericitem.h"
#include "titles.h"
#include "magic.h"
#include "scriptedeffects.h"
#include "mscharacter.h"
#include "script.h"
#include "modeldefs.h"
#include "logger.h"

#ifndef VALVE_DLL
void ContainerWindowUpdate();
void ShowWeaponDesc(CGenericItem *pItem);
#include "ms/vgui_hud.h"
#include "hud.h"
#include "ms/hudscript.h"
#include "ms/clglobal.h"
#else
#include "global.h"
#include "fndatahandler.h"
#endif

//NOTENOTE: remove this when char corruption bug is fixed - Solokiller 5/10/2017
#include "game.h"
//END NOTE

char* ModelListHumanMale[HUMAN_BODYPARTS] =
{
	MODEL_HUMAN_LEGS,
	MODEL_HUMAN_HEAD,
	MODEL_HUMAN_CHEST,
	MODEL_HUMAN_ARMS,	
};

char* ModelListHumanFemale[HUMAN_BODYPARTS] =
{
	MODEL_HUMAN_FEM_LEGS,
	MODEL_HUMAN_FEM_HEAD,
	MODEL_HUMAN_FEM_CHEST,
	MODEL_HUMAN_FEM_ARMS,	
};

keysnapshot KeyHistory[MAX_KEYHISTORY];

const char *GetPlayerTitle(int Title)
{
	return CTitleManager::Titles[Title].Name;
}
int GetPlayerTitleIdx(const char *pszTitle)
{
	for (int i = 0; i < CTitleManager::Titles.size(); i++)
		if (FStrEq(CTitleManager::Titles[i].Name, pszTitle))
			return i;
	return 0;
}

mslist<title_t> CTitleManager::Titles;
title_t CTitleManager::DefaultTitle;

void CTitleManager::AddTitle(title_t &Title)
{
	if (!Title.SkillsReq.size()) //Setting the default title
		DefaultTitle = Title;
	else
		Titles.add(Title); //Setting a normal title
}
void CTitleManager::DeleteAllTitles()
{
	Titles.clear();
}

title_t *CTitleManager::GetPlayerTitle(CBasePlayer *pPlayer)
{
	//if( !pPlayer->m_SkillStats.size() ) return NULL;

	//Cache player skills, sorted by value
	skillcache_t SortedSkills[SKILL_MAX_STATS];
	skillcache_t Skills[SKILL_MAX_STATS];
	skillcache_t Temp;
	skillcache_t SwapTemp;

	for (int i = 0; i < SKILL_MAX_STATS; i++)
	{
		Temp.Skill = i;
		Temp.Value = pPlayer->GetSkillStat(SKILL_FIRSTSKILL + i);
		Skills[i] = Temp;

		for (int n = 0; n < i; n++)
		{
			if (Temp.Value > SortedSkills[n].Value)
			{
				SwapTemp = SortedSkills[n];
				SortedSkills[n] = Temp;
				Temp = SwapTemp;
			}
		}
		SortedSkills[i] = Temp;
	}

	/*#ifndef VALVE_DLL
	 for (int i = 0; i < SKILL_MAX_STATS; i++)
		Print( "Skill %i, [%i]%s (%i)\n", i, SortedSkills[i].Skill, SkillStatList[SortedSkills[i].Skill].Name, GetSkillName( SKILL_FIRSTSKILL + SortedSkills[i].Skill ) );
#endif*/

	//Find a title
	title_t *pBestTitle = &DefaultTitle;

	for (int i = 0; i < Titles.size(); i++)
	{
		title_t &Title = Titles[i];
		bool SkillsAreValid = true;
		for (int s = 0; s < Title.SkillsReq.size(); s++)
		{
			int Skill = Title.SkillsReq[s] - SKILL_FIRSTSKILL;

			//Is this skill high enough to match this title?
			if (Skills[Skill].Value < Title.MinLevel)
			{
				SkillsAreValid = false;
				break;
			} //Skill not high enough

			/*#ifndef VALVE_DLL
		Print( "Title: %s Skill #%i, [%i]%s (%i/%i)\n", STRING(Title.Name), s, Title.SkillsReq[s], GetSkillName( SKILL_FIRSTSKILL + Skills[Skill].Skill ), Skills[Skill].Value, Title.MinLevel );
#endif*/

			//Is this skill the highest skill? (If x skills are required, then is it one of the top x skills?)
			bool IsHighestSkill = false;
			for (int h = 0; h < Title.SkillsReq.size(); h++)
				if (SortedSkills[h].Skill == Skill)
				{
					IsHighestSkill = true;
					break;
				}

			if (!IsHighestSkill)
			{
				SkillsAreValid = false;
				break;
			} //Wasn't a top skill
		}

		if (!SkillsAreValid)
			continue; //At lesat one skill required by this title wasn't a top skill

		//Is this the first valid title, or is this title better than the previous best title?
		if (!pBestTitle ||												 //First valid title
			(pBestTitle &&												 //Previous title exists
			 ((Title.SkillsReq.size() > pBestTitle->SkillsReq.size()) || //This title is better because it requires more skills
			  (Title.MinLevel > pBestTitle->MinLevel))					 //This title is better because it requires a higher level
			 ))
			pBestTitle = &Title;
	}

	return pBestTitle;
}

const char *CBasePlayer::GetTitle()
{
	//Lark DEC2017_16: Only replace old titles if CustomTitle is set.
	if( !CustomTitle.contains("NONESET") )
		return CustomTitle;
	else
	{
		//Old Way
		title_t *pTitle = CTitleManager::GetPlayerTitle( this );
		if( pTitle ) return pTitle->Name;
		return "Unknown";
	}
}

const char *CBasePlayer::GetFullTitle()
{
	//Lark DEC2017_16: Only replace old titles if CustomTitle is set.
	if( !CustomTitle.contains("NONESET") )
		return CustomTitle;
	else
	{
		//Old Way
		title_t *pTitle = CTitleManager::GetPlayerTitle( this );
		if( pTitle )
		{
			static msstring Title;   //Format: "<skilllevel> Title"

			Title = "";

			int SkillLevel = 0;
			int SkillsReq = pTitle->SkillsReq.size();
			if( SkillsReq )   //Only add the skill level if this title requires skills
			{
				for( int s = 0; s < SkillsReq; s++ )
				{
					SkillLevel += GetSkillStat( pTitle->SkillsReq[s] );
				}
				SkillLevel /= SkillsReq;

				Title += SkillLevel;
				Title += " ";
			}

			Title += pTitle->Name;
			return Title.c_str();
		}
	}

	return "Unknown";
}
bool CBasePlayer::CreateStats()
{
	if (m_CharacterState == CHARSTATE_LOADED || m_StatsCreated)
		return false;

	CMSMonster::CreateStats();

	//#ifdef VALVE_DLL
	//	CStat::InitStatList( m_ClStats );
	//#endif

	m_CharacterState = CHARSTATE_UNLOADED; //1 == Stats Created but character not loaded

	return true;
}
void CBasePlayer::DeleteStats()
{
	//#ifdef VALVE_DLL
	//The server dll doesn't call entity destructors, so i have to do it all here
	//	m_ClStats.clear( );
	//#endif

	CMSMonster::DeleteStats();
}

//Called once on startup on both client & server
void CBasePlayer::InitialSpawn(void)
{
	startdbg;

	if (m_Initialized)
		return;

		//MiB JUN2010_17 - Reset the chosen arrow for the client
#ifndef VALVE_DLL
	player.m_ChosenArrow = NULL;
#endif

	m_AnimSpeedAdj = 1;

	// Reset char info
	for (int i = 0; i < MAX_CHARSLOTS; i++)
		m_CharInfo[i].Index = i;

	dbg("Call CreateStats");
	CreateStats();

	dbg("Call Script Spawn");

//Load the script file and precache all models/sounds it uses
#ifdef VALVE_DLL
	bool fScriptSpawned = Script_Add(PLAYER_SCRIPT, this) ? true : false;

	if (!fScriptSpawned)
		MSErrorConsoleText("CBasePlayer::InitialSpawn()", msstring("Missing ") + PLAYER_SCRIPT);

	//Add all the player-initiated effects.  Such as sit, lay down, emotes, etc.
	dbg("Add default effects to player");

	/*
		//Thothie MAR2012 debuggary
		globalscripteffect_t ManualEffect;
		ManualEffect.m_Name = "player_sitstand";
		ManualEffect.m_ScriptName = "player/emote_sit&stand";
		SetBits( ManualEffect.m_Flags, SCRIPTEFFECT_PLAYERACTION );
		CGlobalScriptedEffects::RegisterEffect( ManualEffect );
			*/

	for (int i = 0; i < CGlobalScriptedEffects::Effects.size(); i++)
	{
		globalscripteffect_t &Effect = CGlobalScriptedEffects::Effects[i];
		if (!FBitSet(Effect.m_Flags, SCRIPTEFFECT_PLAYERACTION))
			continue;

		CScript *Script = CGlobalScriptedEffects::ApplyEffect(Effect.m_ScriptName, this, this);
		if (!Script)
			continue;

		Script->SetVar("game.effect.updateplayer", 1); //Make sure the player gets an initial update
	}
#else
	msstringlist strParams = msstringlist();
	gHUD.m_HUDScript->CreateScript(PLAYER_SCRIPT, strParams, false, PLAYER_SCRIPT_ID);
#endif

	CallScriptEvent("game_reset_wear_positions"); //Initialize the wearable positions, in case the player makes a new char
	m_Initialized = true;

	enddbg;
}
/*
  PlaySound - Save yourself a couple parameters by using this instead of EMIT_SOUND.
  ���������   It also allows translation to the female versions of the sound
*/

void CBasePlayer ::PlaySound(int channel, const char *sample, float volume, bool fGenderSpecific, float attenuation)
{
	char SoundName[128];
	 strncpy(SoundName,  sample, sizeof(SoundName) );

	if (fGenderSpecific && m_Gender == GENDER_FEMALE)
	{
		const char *pszStart = strstr(sample, "player/");
		if (pszStart)
		{
			pszStart += 7;
			int iLeftLen = pszStart - sample;
			char cTemp1[128];
			 strncpy(cTemp1,  sample, sizeof(cTemp1) );
			cTemp1[iLeftLen] = 0;
			 _snprintf(SoundName, sizeof(SoundName),  "%sfemale%s",  cTemp1,  pszStart );
		}
	}

	EMIT_SOUND(edict(), channel, SoundName, volume, attenuation);
}

//
// Add an item to the player (Item == Weapon == Selectable Object)
//
bool CBasePlayer::AddItem(CGenericItem *pItem, bool ToHand, bool CheckWeight, int ForceHand)
{
	if (CMSMonster::AddItem(pItem, ToHand, CheckWeight, ForceHand))
	{
		if (pItem->m_Hand == HAND_PLAYERHANDS)
			PlayerHands = pItem;

#ifndef VALVE_DLL
		else
			ShowWeaponDesc(pItem);
#endif

		return true;
	}

	return false;
}

//Find a hand for an Item
//Returns:
//0 or 1 Item should be held in this hand
//2      Item should be held in this hand and item is undroppable
//-1 if no item specified (pItem==NULL)
//-2 if no hand is availiable
//-3 if item is too big
//-4 if a hand is available, but a hand swap is needed first
//If
int CBasePlayer::NewItemHand(CGenericItem *pItem, bool CheckWeight, bool bVerbose, bool FreeHands, char *pszErrorString)
{
	//Returns the hand (or -1) of where the new item can be held
	//bNonVerbose == TRUE means don't say anything just return the value
	int iAddHand = -1;
	char cErrorString[128];

	if (!pItem)
		return -1;

	for (int i = 0; i < MAX_PLAYER_HANDS; i++) //Am I already holding it?
		if (Hand(i) && Hand(i) == pItem)
			return 0;

	if (pItem->m_PrefHand == HAND_PLAYERHANDS)
		return HAND_PLAYERHANDS;

	bool HoldingTwoHandedItem = false;

	for (int i = 0; i < MAX_PLAYER_HANDS; i++) //If either hand is holding a two-handed items, my hands are full
		if (Hand(i) && Hand(i)->m_PrefHand == BOTH_HANDS)
		{
			HoldingTwoHandedItem = true;
			break;
		}

	if (!HoldingTwoHandedItem) //If I'm holding a two-handed weapon, my hands are already full
	{
		if (pItem->m_PrefHand != BOTH_HANDS)
		{
			//If the item has left or right set as the desired hand, only put the item
			//into that hand.  If no desired hand, use iPrefHand.  If needs both hands,
			//check both, and use iPrefHand if both are empty
			bool fHandAllowed[MAX_PLAYER_HANDS] = { false };
			if (pItem->m_PrefHand == ANY_HAND)
				for (int i = 0; i < MAX_PLAYER_HANDS; i++)
					fHandAllowed[i] = true;
			else
				fHandAllowed[pItem->m_PrefHand] = true;

			//Check left hand first, so items set to ANY_HAND will go into it first
			for (int i = 0; i < MAX_PLAYER_HANDS; i++)
				if (fHandAllowed[i] && !Hand(i))
				{
					iAddHand = i;
					break;
				}
		}
		else
		{
			//Item requires both hands
			iAddHand = m_PrefHand;

			for (int i = 0; i < MAX_PLAYER_HANDS; i++) //If either hand is full, the item can't be held
				if (Hand(i))
				{
					iAddHand = -1;
					break;
				}
		}
	}

	if (CheckWeight && !CanHold(pItem, bVerbose, cErrorString)) //If CheckWeight is not set, skip the weight/volume check since I'm already carrying the item
		iAddHand = -3;

	else if (iAddHand < 0)
	{
		//Preference hand was full.
		//Return the non-preference hand, or error if that is full too
		char cHandStr[32];
		bool Success = false;
		if (pItem->m_PrefHand <= ANY_HAND)
		{
			if (!HoldingTwoHandedItem && pItem->m_PrefHand < ANY_HAND && !Hand(!pItem->m_PrefHand))
				return !pItem->m_PrefHand; //Can hold item, but in the non-preferred hand
			else
			{
				if (FreeHands) //Try to free the desired hand
				{
					int PrefHand = (pItem->m_PrefHand < ANY_HAND) ? pItem->m_PrefHand : m_PrefHand, OtherHand = !PrefHand;
					if (!Hand(PrefHand) || Hand(PrefHand)->PutAway(false))
					{
						//Check if the other hand is holding a two-handed item
						CGenericItem *pOtherHand = Hand(OtherHand);
						if (pOtherHand && pOtherHand->m_PrefHand == BOTH_HANDS)
						{
							if (pOtherHand->PutAway(false)) //Try to get rid of the two-handed item
								Success = true;
						}
						else
							Success = true; //If I'm not holding a two-handed item then its fine
					}
				}

				 strncpy(cHandStr,  "your hands are full", sizeof(cHandStr) );
			}
		}
		else if (pItem->m_PrefHand == BOTH_HANDS)
		{
			if (FreeHands) //Try to free both hands
			{
				for (int i = 0; i < MAX_PLAYER_HANDS; i++)
				{
					if (Hand(i))
						if (!Hand(i)->PutAway(false))
							break;
					if (i == MAX_PLAYER_HANDS - 1)
						Success = true;
				}
			}

			 strncpy(cHandStr,  "you need both hands available", sizeof(cHandStr) );
		}

		if (Success)
			iAddHand = 0; //FreeHands was specified and I was able to put away an item or both items to make space
		else
		{
			if (bVerbose)
				 _snprintf(cErrorString, sizeof(cErrorString),  "You can't get %s because %s!",  SPEECH_GetItemName(pItem),  cHandStr );
			iAddHand = -2;
		}
	}
	if (iAddHand < 0)
		if (!pszErrorString && bVerbose)
			SendEventMsg(HUDEVENT_UNABLE, cErrorString);
		else
			strncpy(pszErrorString, cErrorString, sizeof(cErrorString));

	return iAddHand;
}

//#item Return Thothie SEP2011_07
int CBasePlayer::NumItems(void)
{
	int TotalItems = Gear.size() - 1;
	for (int i = 0; i < Gear.size(); i++)
	{
		CGenericItem *pPack = Gear[i];
		for (int n = 0; n < pPack->Container_ItemCount(); n++)
		{
			++TotalItems;
		}
	}
	return TotalItems;
}

//Weight check
bool CBasePlayer::CanHold(CGenericItem *pItem, bool bVerbose, char *pszErrorString)
{
	bool Success = true;

#ifdef VALVE_DLL
	char cErrorString[128];

	//Thothie attempting to limit item count (haven't figured what max count is yet)
	//Gear.size() returns # packs, not #items

	int TotalItems = NumItems(); //Gear.size() - 1;
	int MaxItems = THOTH_MAX_ITEMS;
	int WarnItems = THOTH_MAX_ITEMS - 15;
	/*
	 for (int i = 0; i < Gear.size(); i++)
	{
		CGenericItem *pPack = Gear[i];
		 for (int n = 0; n < pPack->Container_ItemCount(); n++)
		{
			++TotalItems;
		}
	}
	*/
	int outWeight = Weight();
	int outMaxWeight = Volume();
	ClientPrint(this->pev, at_console, UTIL_VarArgs("Item_Count: %i/%i Weight: %ilbs/%i\n", TotalItems, MaxItems, outWeight, outMaxWeight));
	if (TotalItems < MaxItems)
	{
		if (TotalItems > WarnItems)
			SendEventMsg(HUDEVENT_UNABLE, UTIL_VarArgs("Warning: you are carrying too many items! (%i/%i)\nToo many items can result in character corruption!", TotalItems, MaxItems));
	}
	if (TotalItems >= MaxItems)
	{
		if (bVerbose)
			 strncpy(cErrorString,  "You are carrying too many items.", sizeof(cErrorString) );
		pItem->pev->origin = pev->origin;
		Success = false;
	}
	//[Thothie]

	//Thothie - Size doesn't register proper, so we're removing this for now
	/*if( pItem->Volume( ) > Volume( ) )
	{
		if( bVerbose ) sprintf( cErrorString, "The %s is too big for you to carry.", pItem->DisplayName() );
		Success = false;
	}*/
	if (pItem->Weight() + Weight() > Volume())
	{
		if (bVerbose)
			 _snprintf(cErrorString, sizeof(cErrorString),  "The %s would make your equipment too heavy!",  pItem->DisplayName() );
		pItem->pev->origin = pev->origin; //Thothie - attempting to stop items that are too heavy from going to oblivion
		Success = false;
	}

	if (!Success)
	{
		if (!pszErrorString && bVerbose)
			SendEventMsg(HUDEVENT_UNABLE, cErrorString);
		else
			strncpy(pszErrorString, cErrorString, sizeof(cErrorString));
	}
#endif

	return Success;
}

/*bool CBasePlayer :: SwapHands( bool bVerbose )
{
	if( Hand(LEFT_HAND) && Hand(RIGHT_HAND) )
		return false;

	CGenericItem *pLefthand = (CGenericItem *)Hand(LEFT_HAND);
	CGenericItem *pRighthand = (CGenericItem *)Hand(RIGHT_HAND);

	Hand[LEFT_HAND] = Hand[RIGHT_HAND] = NULL;
	//#ifdef VALVE_DLL
	//	m_ClientHandID[LEFT_HAND] = m_ClientHandID[RIGHT_HAND] = -1;
	//#endif
	if( pLefthand )
	{
		SwitchItem( pLefthand, RIGHT_HAND, bVerbose );
	}
	if( pRighthand )
	{
		SwitchItem( pRighthand, LEFT_HAND, bVerbose );
	}
	if( bVerbose )
		SendInfoMsg( "You swap hands.\n" );

	return true;
}*/

//
// SwitchItem - Specify an Item and this function finds a free hand,
// ����������   places it in that hand, and uses it

/*int CBasePlayer :: SwitchItem( CGenericItem *pItem, int iHand, bool bVerbose )
{
	int iReturn = CMSMonster :: SwitchItem( pItem, iHand );

	if( iReturn == 5 && bVerbose ) SendInfoMsg( "No Hands free!\n" );

	if( iReturn == 1 )
	{
		#ifndef VALVE_DLL
			ContainerWindowUpdate( );
		#endif
	}

	return iReturn;
}*/
/*
	RemovePlayerItem - Removes an item from the player's hands or packlist.
	����������������   Set bCallItemDropFunc to TRUE if you want to call
					   pItem->Drop which completely disassociates the item
					   from its owner.  Set to FALSE if you're just removing
					   it from the player's hands (to wear it or something)
*/
bool CBasePlayer::RemoveItem(CGenericItem *pItem)
{
	if (PlayerHands == pItem)
		PlayerHands = NULL;

	CMSMonster::RemoveItem(pItem);

	return true;
}

//=========================================================
// PutInPack - puts the item in the specified hand into the
// specified pack.
//=========================================================
bool CBasePlayer::PutInPack(int iHand, CGenericItem *pContainer, bool bVerbose)
{
#ifdef VALVE_DLL
	bVerbose = false;
#endif

	if (!pContainer || Hand(iHand) == pContainer)
		return false;

	if (!Hand(iHand))
	{
		if (bVerbose)
			SendInfoMsg("There is nothing in your %s hand.\n", SPEECH_IntToHand(iHand));
		return false;
	}

	return PutInPack(Hand(iHand), pContainer, bVerbose);
}
bool CBasePlayer::PutInPack(CGenericItem *pItem, CGenericItem *pContainer, bool bVerbose)
{
	if (!pItem->PutInPack(pContainer))
	{
		if (bVerbose)
		{
			if (!pItem->SpellData)
			{
				if (!RANDOM_LONG(0, 1))
					SendInfoMsg("Your %s can't fit that!\n", pContainer->DisplayName());
				else
					SendInfoMsg("You try to stuff %s into your %s, but to no avail.\n", SPEECH_GetItemName(pItem), pContainer->DisplayName());
			}
		}
		return false;
	}

	//RemovePlayerItem() gets called from the Item's PutInPack( ) function
	char sz[32];
	 strncpy(sz,  SPEECH_GetItemName(pItem), sizeof(sz) );
	if (bVerbose)
		SendInfoMsg("You put %s in %s\n", sz, SPEECH_GetItemName(pContainer));
#ifndef VALVE_DLL
	ContainerWindowUpdate();
#endif
	return true;
}
CGenericItem *CBasePlayer::CanPutInAnyPack(CGenericItem *pItem, bool bVerbose)
{
	return pItem->FindPackForItem(this, bVerbose);
}
bool CBasePlayer::PutInAnyPack(CGenericItem *pItem, bool bVerbose)
{
	int PackCount = 0;
	CGenericItem *pFirstPack = NULL;
	for (int i = 0; i < Gear.size(); i++)
		if (FBitSet(Gear[i]->MSProperties(), ITEM_CONTAINER) &&
			Gear[i]->m_Location != ITEMPOS_HANDS)
		{
			pFirstPack = Gear[i];
			PackCount++;
		}

	if (PackCount == 1) //Only one pack found, use it
		return PutInPack(pItem, pFirstPack, bVerbose);

	CGenericItem *pPack = CanPutInAnyPack(pItem, bVerbose);

	if (pPack)
		return PutInPack(pItem, pPack, bVerbose);
	else if (bVerbose)
	{
		//No packs or all packs full
		if (!pItem->SpellData) //Spells don't give error messages
			//Give a generic error message
			SendEventMsg(HUDEVENT_UNABLE, msstring(SPEECH_GetItemName(pItem)) + " won't fit into any of your packs");
	}
	return false;
}

bool CBasePlayer::UseItem(int iHand, bool bVerbose)
{
	startdbg;

	//bVerbose == true print all failure messages
	int iUseHand = 0;

	if (iHand > -1 && iHand < MAX_NPC_HANDS)
		iUseHand = iHand;
	else
	{
		//No Hand Specified, Assume the current hand
		iUseHand = m_CurrentHand;
	}

	bool DrawWeapon = !Hand(iUseHand) ? true : false;
	if (!DrawWeapon && Hand(iUseHand)->m_PrefHand == HAND_PLAYERHANDS) //Dont call useitem on playerhands
	{																   //Use pref hand (should be empty) to draw a weapon
		DrawWeapon = true;											   //or give a proper error message
		iUseHand = m_PrefHand;
	}

	/*dbg( "Remove weapon from sheath" );
	if( DrawWeapon )
	{
		//Try to find a weapon to pull out from a sheath

		CGenericItem *pItem = NULL;
		 for (int i = 0; i < Gear.size(); i++)
		{
			CGenericItem *pPack = Gear[i];
			if( !FBitSet(pPack->MSProperties(),ITEM_CONTAINER) || pPack->Container_Type() != CGenericItem::PACK_SHEATH || !pPack->Container_ItemCount() )
				continue;

			pItem = pPack->Container_GetItem( 0 );
			pPack = Gear[i];
			break;
		}

		if( pItem )
		{
			pItem->GiveTo( this, true, false );
			//if( AddItem( pItem, TRUE ) )
			//	pPack->Container_RemoveItem( pItem );

			//#ifdef VALVE_DLL
			//	//Sync client, so a server update is not sent
			//	m_ClientHandID[pItem->m_Hand] = Hand[pItem->m_Hand] ? Hand[pItem->m_Hand]->m_iId : 0;
			//	m_ClientCurrentHand = iCurrentHand;
			//#endif

			return true;
		}
		else if( bVerbose ) SendEventMsg( HUDEVENT_UNABLE, msstring("There is nothing in your ") + SPEECH_IntToHand(iUseHand) + " hand" );

		return false;
	}*/

	CGenericItem *pUse = Hand(iUseHand);

	dbg("Call UseItem");
	if (pUse && !pUse->UseItem(bVerbose))
	{
		//if( bVerbose ) SendInfoMsg( "You cannot use %s\n", SPEECH_GetItemName( Hand[iUseHand] ) );
		return false;
	}

	enddbg;

	return true;
}

void CBasePlayer::RemoveAllItems(bool fDead, bool fDeleteItems)
{
	int i = 0;
	for (i = 0; i < MAX_NPC_HANDS; i++)
	{
		CGenericItem *pItem = Hand(i);
		if (!pItem)
			continue;

		//Drop what's in your hands
		DropItem(pItem, true, false);

#ifdef VALVE_DLL
		//Sever only - client deletes items in CGenericItem::Drop()
		if (fDeleteItems)
			//Delete the item
			pItem->SUB_Remove();
#endif
	}

	//I have to make a copy of the current Gear list because it gets
	//modified while I go through it calling DropItem()
	int count = Gear.size();
	if (count)
	{
		CGenericItem **Gearlist = msnew(CGenericItem *[count]);
		for (i = 0; i < count; i++)
			Gearlist[i] = Gear[i];

		for (i = 0; i < count; i++)
		{
			if (!Gearlist[i])
				continue;

			if (fDeleteItems)
			{
				//Delete the item
				DropItem(Gearlist[i], true, false);
#ifdef VALVE_DLL
				//Sever only - client deletes items in CGenericItem::Drop()
				Gearlist[i]->SUB_Remove();
#endif
			}

#ifdef VALVE_DLL
			else if (fDead && m_Corpse)
			{
				//The Corpse MUST have space for everything the player is holding
				Gearlist[i]->RemoveFromOwner();
			}
#endif
			//If I'm not dead, I don't have a corpse, or my corpse doesn't
			//have space for this item, drop the item
			else
			{
				DropItem(Gearlist[i], true, false);
				Gearlist[i]->pev->velocity = pev->velocity * RANDOM_FLOAT(1, 1.6);
			}
		}

		//Items get deleted from the player's gear list in RemoveFromOwner
		//(which is also called from DropItem)
		//	Gear.RemoveAllItems( );
		delete[] Gearlist;
	}

	m_Corpse = NULL;
	//SetViewModel( NULL );
}

//=========================================================
// DropItem - drop the item in the specified hand, or if not given,
// the active hand.
//=========================================================
/*bool CBasePlayer::DropItem( int iHand, bool ForceDrop, Vector &vDropDir )
{
	int DropHand;

	if( iHand > -1 && iHand < MAX_NPC_HANDS ) DropHand = iHand;
	if ( iHand == -1 )
		//No Hand Specified, Assume the Current hand
		DropHand = m_CurrentHand;

	if( !Hand(DropHand) ) {
		#ifndef VALVE_DLL
			if( !ForceDrop )
				SendEventMsg( HUDEVENT_UNABLE, msstring("There is nothing in your ") + SPEECH_IntToHand(m_CurrentHand) + " hand to drop." );
		#endif
		return false;
	}

	return DropItem( Hand(DropHand), ForceDrop, ForceDrop, vDropDir );
}*/

//Items could be anywhere on the player (hands, worn, inside pack etc)
bool CBasePlayer::DropItem(CGenericItem *pDropItem, bool ForceDrop, bool Verbose)
{
	if (!pDropItem)
		return false;

	if (ForceDrop)
		pDropItem->Drop(/*1, vDropDir*/);

#ifdef VALVE_DLL
	else
	{
		if (pDropItem->CanDrop())
		{
			if (Verbose)
			{
				if (FBitSet(pDropItem->MSProperties(), ITEM_SPELL))
					SendEventMsg(HUDEVENT_NORMAL, msstring("The ") + SPEECH_GetItemName(pDropItem) + " spell is canceled");
				else
					SendEventMsg(HUDEVENT_NORMAL, msstring("You drop ") + SPEECH_GetItemName(pDropItem));
			}

			UTIL_MakeVectorsPrivate(pev->angles, gpGlobals->v_forward, NULL, NULL);
			pDropItem->Drop();
			pDropItem->pev->velocity = pev->velocity + gpGlobals->v_forward * 175 + Vector(0, 0, 60);
		}
		else
		{
			if (Verbose && !FBitSet(pDropItem->MSProperties(), ITEM_SPELL) && pDropItem->m_PrefHand != HAND_PLAYERHANDS)
				SendEventMsg(HUDEVENT_UNABLE, msstring("You cannot drop ") + SPEECH_GetItemName(pDropItem) + " right now");
			return false;
		}
	}
#else
	else
	{
		//Should never get here
		MSErrorConsoleText("CBasePlayer::DropItem()", "Called Dropitem on client without ForceDrop!");
		RemoveItem(pDropItem);
		pDropItem->SUB_Remove();
	}
#endif

	return true;
}

float CBasePlayer::CurrentSpeed(bool bParseSpeed)
{
	//The speed you're SUPPOSED to be able to go right now

	//	if( RoundTime > gpGlobals->time ) return fSpeed;
	if (!FBitSet(m_StatusFlags, PLAYER_MOVE_NORUN) && FBitSet(m_StatusFlags, PLAYER_MOVE_RUNNING))
		return RunSpeed(bParseSpeed);
	else
		return WalkSpeed(bParseSpeed);
}
float CBasePlayer::WalkSpeed(bool fParseSpeed)
{
	//75 dex adds 100 to speed
	float fSpeed;
	float Dex = GetNatStat(NATURAL_DEX);

#define BASE_SPEED 160
#define WALKSPEED_MAX_WEIGHT_SLOWDOWN 70
#define WALKSPEED_MAX_DEX 75.0f

	float StatEnhancement = (min(Dex, WALKSPEED_MAX_DEX) / WALKSPEED_MAX_DEX) * 100;

	//Speed detriment - When weight over 50% the volume, speed reduces
	float VolumeHalf = Volume() / 2.0f;

	float SpeedDetriment = Weight() - VolumeHalf;
	SpeedDetriment = (max(SpeedDetriment, 0) / VolumeHalf) * WALKSPEED_MAX_WEIGHT_SLOWDOWN;

	SpeedDetriment = min(SpeedDetriment, WALKSPEED_MAX_WEIGHT_SLOWDOWN);
	SpeedDetriment = max(SpeedDetriment, 0);

	fSpeed = BASE_SPEED + StatEnhancement - SpeedDetriment;

	return fParseSpeed ? ParseSpeed(fSpeed) : fSpeed;
}
float CBasePlayer::RunSpeed(bool fParseSpeed)
{
	float fSpeed;
	fSpeed = WalkSpeed(fParseSpeed); //1.6
	float flCappedStamina = max(Stamina, 0.001f);
	float flMaxStamina = MaxStamina();
	float flCappedMaxStamina = max(flMaxStamina, 0.001f);

	float StaminaRatio = flCappedStamina / flCappedMaxStamina;
	fSpeed *= 1 + min(StaminaRatio, 1.0f);
	return fParseSpeed ? ParseSpeed(fSpeed) : fSpeed;
	//if( !bParseSpeed ) return fSpeed;
	//return ParseSpeed(fSpeed);
}
float CBasePlayer::ParseSpeed(float flSpeed)
{
	float Speed = flSpeed;
	if (ArrowsStuckInMe > 0)
		Speed -= 60;

	Speed *= FBitSet(m_StatusFlags, PLAYER_MOVE_ATTACKING) ? 0.5f : 1.0f; //half speed while attacking
	//Print( "Speed: %f\n", fsp );
	return Speed;
}

float CBasePlayer::Volume()
{
	//Thothie - fixed the max weight issue here
	//- only after capping the item limit
	int MyVolume = GetNatStat(NATURAL_STR) * 25;
	MyVolume = MyVolume + 25; //max(min(MyVolume,600),0);
	MyVolume = min(MyVolume, 2000);
	return MyVolume;
}
float CBasePlayer::MaxHP()
{
	//Thothie adding 3hp per Wisdom point
	//AUG2010_23 - dropped Str from 8 to 7, raised fitness from 6 to 7 and wisdom from 2 to 3
	int Str = GetNatStat(NATURAL_STR) - 1, Fit = GetNatStat(NATURAL_FIT) - 1, Wis = GetNatStat(NATURAL_WIS) - 1;
	Str = max(Str, 0);
	Fit = max(Fit, 0);
	Wis = max(Wis, 0);
	return 5 + Str * 7 + Fit * 7 + Wis * 3;
}
float CBasePlayer::MaxMP()
{
	return GetNatStat(NATURAL_WIS) * 10;
}
float CBasePlayer::MaxStamina()
{
	return 3 + GetNatStat(NATURAL_FIT) * 2.5 + GetNatStat(NATURAL_STR) * 1.0;
}
int CBasePlayer::SkillAvg()
{
	//Get the average of all skills
	int iTemp = 0;
	for (int i = 0; i < SKILL_MAX_STATS; i++)
		iTemp += GetSkillStat(i);
	iTemp /= SKILL_MAX_STATS;

	iTemp = max(iTemp, 0);

	return iTemp;
}

void CBasePlayer::SendInfoMsg(char *msg, ...)
{
	if (!IsNetClient() || !m_fGameHUDInitialized)
		return;

	static char string[1024];
	va_list argptr;	
	va_start(argptr, msg);
	vsnprintf(string, sizeof(string), msg, argptr);
	va_end(argptr);

	SendEventMsg(string);
}
void CBasePlayer::SendHUDMsg(msstring_ref Title, msstring_ref Text)
{
#ifdef VALVE_DLL
	MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_HUDMSG], NULL, pev);
	WRITE_BYTE(0);		 //This is an InfoMsg
	WRITE_STRING_LIMIT(Title, 90); //Title
	WRITE_STRING_LIMIT(Text, 90);	 //Text
	MESSAGE_END();
#else
	HUD_ShowInfoWin(Title, Text);
#endif
}
void CBasePlayer::SendHelpMsg(msstring_ref Tipname, msstring_ref Title, msstring_ref Text)
{
	if (m_CharacterState != CHARSTATE_LOADED)
		return;

	//MAR2008a - allow use of helptip repeatedly for multi-line function
	msstring mstipname = Tipname;
	bool generic_tip = false;
	if (mstipname.contains("generic"))
		generic_tip = true;

	for (int i = 0; i < m_ViewedHelpTips.size(); i++)
		if (m_ViewedHelpTips[i] == Tipname && !generic_tip)
			return;

	//Thothie - MAY2007a - Centralize Help Tips scriptside
	if (!generic_tip)
	{
		msstringlist Parameters;
		Parameters.add(Title);
		Parameters.add(Tipname);
		CallScriptEvent("game_helptip", &Parameters);
	}

#ifdef VALVE_DLL
//Send the help msg in parts.  First send all the parts.  Then send a message that shows the parts, all put together
//This gets around the 192 message size limit in HL
#define MAX_PARTLEN 189 //192 - (byte: null terminator) - (byte: msg id) - (byte: part index) = 189
	int iParts = (strlen(Text) / MAX_PARTLEN) + 1;
	msstringlist Parts;
	for (int p = 0; p < iParts; p++)
	{
		char cTemp[MAX_PARTLEN + 1];
		strncpy(cTemp, &Text[p * MAX_PARTLEN], MAX_PARTLEN);
		cTemp[MAX_PARTLEN] = 0;
		Parts.add(cTemp);
		MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_HUDMSG], NULL, pev);
		WRITE_BYTE(2);		 //This is a HelpMsg part
		WRITE_BYTE(p);		 //Part Index
		WRITE_STRING(cTemp); //Part Text
		MESSAGE_END();
	}
	MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_HUDMSG], NULL, pev);
	WRITE_BYTE(3);		 //This is an HelpMsg
	WRITE_STRING_LIMIT(Title, WRITE_STRING_MAX); //Title
	MESSAGE_END();
#else
	HUD_ShowHelpWin(Title, Text);
#endif

	//MAR2008a - allow use of helptip repeatedly for multi-line function
	if (!generic_tip)
		m_ViewedHelpTips.add(Tipname); //MAR2008a - Thothie - allow sending of repeatable help tips to make use of the multi-line function
}

static COLOR HUDEventColor[] =
	{
		COLOR(220, 220, 220, 0),		   //HUDEVENT_NORMAL
		COLOR(160, 160, 160, 0),		   //HUDEVENT_UNABLE
		COLOR(255 * 0.7, 170 * 0.7, 0, 0), //HUDEVENT_ATTACK
		COLOR(240, 0, 0, 0),			   //HUDEVENT_ATTACKED
		COLOR(0, 240, 0, 0),			   //HUDEVENT_GREEN
		COLOR(0, 0, 240, 0),			   //HUDEVENT_BLUE
};

void CBasePlayer::SendEventMsg(msstring_ref Text)
{
	SendEventMsg((hudevent_e)HUDEVENT_NORMAL, Text);
}
void CBasePlayer::SendEventMsg(hudevent_e HudEvent, msstring_ref Text)
{
	SendEventMsg(HUDEventColor[HudEvent], Text);
}

void CBasePlayer::SendEventMsg(COLOR &color, msstring_ref Text)
{
#ifdef VALVE_DLL
	MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_HUDMSG], NULL, pev);
	WRITE_BYTE(1);			  //Print to the event console
	WRITE_LONG((ulong)color); //Color
	WRITE_STRING_LIMIT(Text, WRITE_STRING_MAX);		  //Text
	MESSAGE_END();
#else
	vgui::Color vguicolor(color.r, color.g, color.b, color.a);
	HUD_PrintEvent(vguicolor, Text);
#endif
}

CGenericItem *CBasePlayer::ActiveItem()
{
	return Hand(m_CurrentHand);
}
int CBasePlayer::ActiveItemHand()
{
	CGenericItem *pActiveItem = ActiveItem();
	if (!pActiveItem)
		return -1;

	return pActiveItem->m_Hand;
}
//Switchs to a held item as the current item
bool CBasePlayer ::SwitchHands(int iHand, bool bVerbose)
{
	CGenericItem *pPrevActiveItem = ActiveItem();
	CGenericItem *pNewItem = Hand(iHand);
#ifdef VALVE_DLL
	if (!pNewItem)
	{
		if (bVerbose)
			SendEventMsg(HUDEVENT_UNABLE, msstring("You aren't holding anything in your ") + SPEECH_IntToHand(iHand) + " hand.");
		return false;
	}

	if (!pNewItem->CanDeploy())
	{
		if (bVerbose)
			SendEventMsg(HUDEVENT_UNABLE, msstring("Can't switch to your ") + SPEECH_IntToHand(iHand) + " hand.");
		return false;
	}

	if (pPrevActiveItem)
	{
		if (pNewItem->m_Hand == HAND_PLAYERHANDS && pPrevActiveItem != pNewItem)
			//Don't allow switch to the player hands if something else is already active
			return false;

		//if (!pPrevActiveItem->CanHolster()) //I dont think this is needed, as the item is already in the players hands. Ends up introducing a bug
		//{
			//if( bVerbose ) SendInfoMsg( "Can't switch to your %s hand.\n", SPEECH_IntToHand(iHand) );
		//	return false;
		//}
		//pPrevActiveItem->Holster();
	}
#endif

	m_CurrentHand = iHand;

	CGenericItem *pActiveItem = ActiveItem(); //This item is the new active item
	//Hand[iCurrentHand]->iGetMethod = METHOD_SWITCHEDHANDS;
	if (pActiveItem)
	{
		pActiveItem->Deploy();
		if (pPrevActiveItem && pActiveItem != pPrevActiveItem && pPrevActiveItem->m_Hand != HAND_PLAYERHANDS) // Swithed from another weapon (not player hands)
			pActiveItem->CallScriptEvent("game_switchhands");
	}
	return true;
}
//Switchs to the best hand
bool CBasePlayer::SwitchToBestHand()
{
#ifdef VALVE_DLL
	if (SwitchHands(m_PrefHand, false))
		return true;
	if (SwitchHands(!m_PrefHand, false))
		return true;

	if (SwitchHands(HAND_PLAYERHANDS, false))
		return true;
#endif

	return false;
}

void CBasePlayer::AttackSound()
{
}
void CBasePlayer::Deactivate()
{
#ifdef VALVE_DLL
	SetTeam(NULL); // Remove me from any party
#endif

	RemoveAllItems(false, true); // Destroy all of the player's weapons and items

	//Must manually deallocate any player memory that was dynamically allocated
	for (int i = 0; i < MAX_CHARSLOTS; i++)
		m_CharInfo[i].Destroy();

	m_EntInfo.clear();
	m_Storages.clear();
	m_ViewedHelpTips.clear();
	m_Quests.clear();
	m_Maps.clear();
	m_WearPositions.clear();
	m_Companions.clear();
#ifdef VALVE_DLL
	m_ClientItems.clear();

	if (CamEntity) // Delete this, if present. (fix mem leak if in death cam + disconnects)
	{
		CamEntity->SUB_Remove();
		CamEntity = NULL;
	}
#endif

	CMSMonster::Deactivate();
}

void CBasePlayer::SetKeys()
{
	//Sets up which buttons Mastersword considers pressed and released
	//Some buttons may be blocked
	SetBits(pbs.ButtonsDown, m_afButtonPressed);
	ClearBits(pbs.ButtonsDown, m_afButtonReleased);

	//Block buttons
	ClearBits(pbs.BlockButtons, (~pev->button));  //Buttons that aren't down anymore get removed from blocking
	ClearBits(pbs.ButtonsDown, pbs.BlockButtons); //Block the buttons
}
void CBasePlayer::BlockButton(int Button)
{
	//Setting a blocked button causes the bit for that button to stay false
	//until the button is released

	SetBits(pbs.BlockButtons, Button);
	ClearBits(pbs.ButtonsDown, Button);
}

void CBasePlayer::LearnSpell(const char *pszSpellScript, bool fVerbose)
{
	spellgroup_v &SpellList = m_SpellList;

	for (int s = 0; s < SpellList.size(); s++) //Already know this spell?
		if (SpellList[s] == pszSpellScript)
			return;

	SpellList.push_back(pszSpellScript);

#ifdef VALVE_DLL
	//MiB Aug2008a (JAN2010_15) - 8 tome limit fix. New method sends one spell at a time.
	MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_SPELLS], NULL, pev);
	WRITE_BYTE(fVerbose ? 1 : 0);
	WRITE_STRING_LIMIT(pszSpellScript, WRITE_STRING_MAX);
	MESSAGE_END();

/*	MESSAGE_BEGIN( MSG_ONE, g_netmsg[NETMSG_SPELLS], NULL, pev );
			WRITE_SHORT( SpellList.size() );					//Number of spells
			WRITE_SHORT( fVerbose ? SpellList.size() : 0 );		//If Verbose: Which spell was learned (incremented by 1 | 0 = Non-verbose)
			 for (int s = 0; s < SpellList.size(); s++)
				WRITE_STRING( SpellList[s] );				//Spell scriptname
		MESSAGE_END();		*/
#else
	if (fVerbose)
	{
		CGenericItem *pTempSpell = CGenericItemMgr::GetGlobalGenericItemByName( pszSpellScript, true );
		if (pTempSpell)
		{
			SendEventMsg(HUDEVENT_NORMAL, msstring("You memorize the ") + pTempSpell->DisplayName() + " spell");
		}
	}
#endif
}

storage_t *CBasePlayer::Storage_GetStorage(msstring_ref pszStorageName)
{
	for (int s = 0; s < m_Storages.size(); s++)
		if (m_Storages[s].Name == pszStorageName)
			return &m_Storages[s];

	return NULL;
}
storage_t *CBasePlayer::Storage_CreateAccount(storage_t &Storage)
{
	storage_t *pExistingStorage = Storage_GetStorage(Storage.Name);
	if (pExistingStorage)
		return pExistingStorage;

	return &m_Storages.add(Storage);
}

bool CBasePlayer::Script_SetupEvent(CScript *Script, SCRIPT_EVENT &Event)
{
	//SetScriptVar( "game.monster.current_anim.uselegs", strlen(m_szAnimLegs) > 0 );

	return CMSMonster::Script_SetupEvent(Script, Event);
}

bool CBasePlayer::IsLocalHost() //This is a listen server and this is the host
{
#ifdef VALVE_DLL
	return !IS_DEDICATED_SERVER() && (entindex() == 1);
#else
	return MSCLGlobals::OnMyOwnListenServer;
#endif
}
msstring CBasePlayer::AuthID()
{
#ifdef VALVE_DLL
	return GETPLAYERAUTHID(this->edict());
#else
	return MSCLGlobals::AuthID;
#endif
}

void CBasePlayer::PreLoadChars(int CharIdx)
{
//Reload the Character list, for players entering server
#ifdef VALVE_DLL
	if (!MSGlobals::ServerSideChar)
		return;

	if (FnDataHandler::IsEnabled())
	{
		// Send a request to retrieve the player's info from a central server.
		// Once the player file is downloaded, m_CharInfo will be updated with the info
		if (CharIdx == -1)
			FnDataHandler::LoadCharacter(this);
		else
			FnDataHandler::LoadCharacter(this, CharIdx);
	}
	else
	{
		charloc_e Location = LOC_SERVER;
#else
		charloc_e Location = LOC_CLIENT;
#endif

		//Load all characters from file, locally

		for (int i = 0; i < MAX_CHARSLOTS; i++)
		{
			charinfo_t &Char = m_CharInfo[i];
			CPlayer_DataBuffer gFile;
			if (gFile.ReadFromFile(GetSaveFileName(i, this), "rb", true))
				Char.AssignChar(i, Location, (char *)gFile.m_Buffer, gFile.GetFileSize(), this);
			else
				Char.Status = CDS_NOTFOUND;
		}

#ifdef VALVE_DLL
	}

	//Start checking m_CharInfo for new char data
	m_TimeSendCharInfo = gpGlobals->time;
#endif
}

void charinfo_t::Destroy()
{
	if (Data)	
		delete Data;
	Data = NULL;
	GearInfo.clear();
	memset(this, 0, sizeof(*this));
}

void charinfo_t::AssignChar(int CharIndex, charloc_e eLocation, const char *pData, int iDataLen, CBasePlayer *pPlayer)
{
	Destroy();

	Index = CharIndex;
	DataLen = iDataLen;
	Data = msnew char[DataLen];
	memcpy(Data, pData, DataLen);
	Location = eLocation;

	chardata_t CharData;
	if (MSChar_Interface::ReadCharData(Data, DataLen, &CharData))
	{
		m_CachedStatus = CDS_UNLOADED;
		Status = CDS_LOADED;
		IsElite = CharData.IsElite ? true : false;
		Gender = (gender_e)CharData.Gender;
		JoinType = MSChar_Interface::CanJoinThisMap(CharData, CharData.m_VisitedMaps);
		//ALERT( at_aiconsole, "PLAYER_HAS_JOINED:%s\n", CharData.m_VisitedMaps ); //thothie: seeing if can pull map log
		Name = CharData.Name;
		MapName = CharData.MapName;
		OldTrans = CharData.OldTrans;
		NextMap = CharData.NextMap;
		NewTrans = CharData.NewTrans;

		//MiB JAN2010_27 - Char Selection Fix
		//Find last body used
		for (int i = 0; i < CharData.m_Quests.size(); i++)
		{
			if (CharData.m_Quests[i].Name == "BODY")
			{
				body = atoi(CharData.m_Quests[i].Data.c_str());
				break;
			}
		}

		for (int i = 0; i < CharData.m_Items.size(); i++) //Determine what model/body my gear is using
		{
			genericitem_full_t &Item = CharData.m_Items[i];
			CGenericItem *pItem = (CGenericItem *)Item;
			if (!pItem)
				continue;

			pItem->CallScriptEvent("game_spawn");
			pItem->CallScriptEvent("game_deploy");
			//pItem->CallScriptEvent( "game_wear" );

			//Thothie FEB2010_01 Pass gender/race with wear (iffy) //fail
			/*
			//Can't do this way, don't have data for m_pOwner
			static msstringlist Params;
			Params.clearitems();
			Params.add( pItem->m_pOwner->m_Race );
			Params.add( (pItem->m_pOwner->m_Gender == 0) ? "male" : "female" );
			Params.add( "char_menu" );
			pItem->CallScriptEvent( "game_wear", &Params );
			*/

			//MiB - But we do have CharData :)
			static msstringlist Params;
			Params.clearitems();
			Params.add((CharData.Gender == GENDER_MALE) ? "male" : "female");
			Params.add("char_menu");
			pItem->CallScriptEvent("game_wear", &Params);

			gearinfo_t Info;
			Info.Flags = 0;
			for (int w = 0; w < pItem->m_WearModelPositions.size(); w++)
				SetBits(Info.Flags, (1 << pItem->m_WearModelPositions[w]));
			if (Item.Location != ITEMPOS_HANDS)
				SetBits(Info.Flags, GEARFL_WEARING);

			Info.Body = pItem->pev->body;
			Info.Model = pItem->pev->modelindex;
			Info.Anim = pItem->pev->sequence;

			GearInfo.add(Info);
			pItem->SUB_Remove();
		}
		if (pPlayer)
			if (JoinType != JN_NOTALLOWED)
				pPlayer->m_CanJoin = true; //The player has at least one character that can join the map
	}
	else
		Status = CDS_NOTFOUND;
}

charinfo_t::~charinfo_t()
{
	Destroy();
}

char *GetOtherPlayerTransition(CBasePlayer *pPlayer)
{
#ifdef VALVE_DLL
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pOtherPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (!pOtherPlayer || pOtherPlayer == pPlayer)
			continue;

		//If one person has a character that can join the map, then don't switch the map
		if (pOtherPlayer->m_CharacterState == CHARSTATE_LOADED && pOtherPlayer->m_SpawnTransition)
			return pOtherPlayer->m_SpawnTransition;
	}

	return 0;
#else
	return MSCLGlobals::OtherPlayers ? "true" : 0;
#endif
}
