/*

	MSMonster.cpp - Extended functionality for Master Sword monsters.

	*included by both dlls*

*/

#include "MSDLLHeaders.h"
#include "Half-life/monsters.h"
#include "animation.h"
#include "soundent.h"

#include "Script.h"

#include "Stats/Stats.h"
#include "Stats/Races.h"
//#include "Store.h"
#include "MSItemDefs.h"
//#include "Monsters/Bodyparts/Bodyparts.h"
#include "Stats/statdefs.h"
#include "Magic.h"
#include "Weapons/Weapons.h"
#include "Syntax/Syntax.h"
#include "Weapons/GenericItem.h"
#include "../cl_dll/MasterSword/vgui_HUD.h"
#include "logfile.h"

void CMSMonster::Deactivate()
{
	//Clean up memory
	DeleteStats();

	if (Body)
	{
		Body->Delete();
		Body = NULL;
	}
	Gear.clear();

	CScriptedEnt::Deactivate();
}

CMSMonster::~CMSMonster()
{
	Deactivate();
}

CMSMonster::CMSMonster() {}
int CMSMonster::MSProperties() { return MS_NPC; }

bool CMSMonster::IsActing()
{
	bool Charging = false;
	for (int i = 0; i < Gear.size(); i++)
		if (Gear[i]->Attack_IsCharging())
		{
			Charging = true;
			break;
		}

	return FBitSet(m_StatusFlags, PLAYER_MOVE_ATTACKING) || Charging;
}

//MiB JUN2010_19 - Check if the player is holding a shield up
bool CMSMonster::IsShielding()
{
	for (int i = 0; i < Gear.size(); i++)
		if (Gear[i]->CurrentAttack && msstring(Gear[i]->m_Name).starts_with("shields_"))
			return true;
	return false;
}

float CMSMonster::WalkSpeed(bool bParseSpeed) { return 1.0; }
float CMSMonster::RunSpeed(bool bParseSpeed) { return 1.0; }

void CMSMonster::CancelAttack()
{
	for (int i = 0; i < MAX_NPC_HANDS; i++)
		if (Hand(i))
			Hand(i)->CancelAttack();

#ifdef VALVE_DLL
	if (m_Activity == ACT_SPECIAL_ATTACK1)
	{
		StopWalking();
	}
#endif
}

CGenericItem *CMSMonster::GetContainer(const char *pNameSubstring)
{
	for (int i = 0; i < Gear.size(); i++)
		if (FBitSet(Gear[i]->MSProperties(), ITEM_CONTAINER) && strstr(Gear[i]->ItemName, pNameSubstring))
			return Gear[i];

	return NULL;
}
CGenericItem *CMSMonster::GetContainer(ulong ID)
{
	for (int i = 0; i < Gear.size(); i++)
		if (FBitSet(Gear[i]->MSProperties(), ITEM_CONTAINER) && Gear[i]->m_iId == ID)
			return Gear[i];

	return NULL;
}
CGenericItem *CMSMonster::GetGearItem(ulong ID)
{
	return Gear.GetItem(ID);
}
CGenericItem *CMSMonster::FindItem(ulong ID, bool CheckHands, bool CheckWorn, bool CheckPacks)
{
	for (int i = 0; i < Gear.size(); i++)
	{
		CGenericItem *pPack = Gear[i];

		if ((pPack->m_Location == ITEMPOS_HANDS && CheckHands) ||
			(pPack->m_Location > ITEMPOS_HANDS && CheckWorn))
		{
			if (pPack->m_iId == ID)
				return pPack;
		}

		if (!CheckPacks || !FBitSet(pPack->MSProperties(), ITEM_CONTAINER))
			continue;

		for (int n = 0; n < pPack->Container_ItemCount(); n++)
		{
			CGenericItem *pPackItem = pPack->Container_GetItem(n);
			if (pPackItem->m_iId == ID)
				return pPackItem;
		}
	}

	return NULL;
}
//
// GetItem - Finds an item in a hand, worn, or inside a pack.
// �������
bool CMSMonster::GetItem(getitem_t &ItemDesc)
{
	ItemDesc.retFound = false;
	ItemDesc.retItem = NULL;
	ItemDesc.retContainer = NULL;

	//Loop twice.  First time, only check the hands.  Second time, check everything else
	for (int t = 0; t < 2; t++)
	{
		bool Loop_CheckHands = (t == 0);

		if ((ItemDesc.IgnoreHands && Loop_CheckHands) || (ItemDesc.IgnoreWornItems && !Loop_CheckHands))
			continue;

		for (int i = 0; i < Gear.size(); i++)
		{
			CGenericItem *pItem = Gear[i];

			if ((Loop_CheckHands && pItem->m_Location != ITEMPOS_HANDS)		 //On hand loop and item is not in hand
				|| (!Loop_CheckHands && pItem->m_Location == ITEMPOS_HANDS)) //On inv loop and item is in hand
				continue;

			if ((ItemDesc.CheckPartialName && pItem->ItemName.m_string.contains(ItemDesc.Name)) || (!ItemDesc.CheckPartialName && pItem->ItemName == ItemDesc.Name))
			{
				ItemDesc.retFound = true;
				ItemDesc.retItem = pItem;
				return true;
			}

			if (ItemDesc.IgnoreInsideContainers || !FBitSet(pItem->MSProperties(), ITEM_CONTAINER))
				continue;

			for (int n = 0; n < pItem->Container_ItemCount(); n++)
			{
				CGenericItem *pItemInPack = pItem->Container_GetItem(n);

				if ((ItemDesc.CheckPartialName && pItem->ItemName.m_string.contains(ItemDesc.Name)) || (!ItemDesc.CheckPartialName && pItemInPack->ItemName == ItemDesc.Name))
				{
					ItemDesc.retFound = true;
					ItemDesc.retItem = pItemInPack;
					ItemDesc.retContainer = pItem;
					return true;
				}
			}
		}
	}

	return NULL;
}

//
// GetItem - Finds an item in a hand or pack.  Won't return a worn item
// �������   If an item is returned and pPack is NULL, its in a hand
CGenericItem *CMSMonster::GetItem(const char *pItemName, CGenericItem **rpPack)
{
	if (rpPack)
		*rpPack = NULL;

	//Check hands first
	for (int i = 0; i < MAX_NPC_HANDS; i++)
		if (Hand(i) && strstr(Hand(i)->ItemName, pItemName))
			return Hand(i);

	//Check inside each pack
	for (int i = 0; i < Gear.size(); i++)
	{
		CGenericItem *pPack = Gear[i];

		if (!FBitSet(pPack->MSProperties(), ITEM_CONTAINER))
			continue;

		for (int n = 0; n < pPack->Container_ItemCount(); n++)
		{
			CGenericItem *pProjInPack = pPack->Container_GetItem(n);
			if (!strstr(pProjInPack->ItemName, pItemName))
				continue;

			if (rpPack)
				*rpPack = pPack;
			return pProjInPack;
		}
	}

	return NULL;
}

//
// GetItemInInventory - Finds the first weapon in inventory after the specified ID.
// ������������������   If no start item is found, then the first valid item is returned
//						If an item is returned and pPack is NULL, the item is being worn (bows)
//						MiB JUN2010_16 - Added a SearchName property. Defaults to "", uses .starts_with(SearchName) to determine if it should return or not
CGenericItem *CMSMonster::GetItemInInventory(uint StartID, bool WeaponOnly, bool CheckHands, bool CheckWorn, msstring SearchName)
{
	static mslist<CGenericItem *> Items;
	Items.clearitems();

	//Check hands first
	if (CheckHands)
		for (int i = 0; i < MAX_NPC_HANDS; i++)
		{
			CGenericItem *pItem = Hand(i);
			if (pItem)
				if (!SearchName.len() || msstring(pItem->m_Name).starts_with(SearchName))
					Items.add(pItem);
		}

	//Check inside each pack
	for (int i = 0; i < Gear.size(); i++)
	{
		CGenericItem *pPack = Gear[i];

		if (pPack->m_Location == ITEMPOS_HANDS && !CheckHands)
			continue;

		if (pPack->m_Location > ITEMPOS_HANDS && !CheckWorn)
			continue;

		if (!SearchName.len() || msstring(pPack->m_Name).starts_with(SearchName))
			Items.add(pPack);

		if (!FBitSet(pPack->MSProperties(), ITEM_CONTAINER))
			continue;

		for (int n = 0; n < pPack->Container_ItemCount(); n++)
		{
			CGenericItem *pItem = pPack->Container_GetItem(n);
			if (!SearchName.len() || msstring(pItem->m_Name).starts_with(SearchName))
				Items.add(pItem);
		}
	}

	//Remove items that aren't weapons
	if (WeaponOnly)
		for (int i = 0; i < Items.size(); i++)
		{
			CGenericItem *pItem = Items[i];

			if (!pItem->m_Attacks.size())
				Items.erase(i--);
		}

	bool UseNextItem = false;
	for (int i = 0; i < Items.size(); i++)
	{
		CGenericItem *pItem = Items[i];

		if (!UseNextItem)
		{								 //Need Braces for logic
			if (pItem->m_iId == StartID) //Found start item, return the next valid item
				UseNextItem = true;
		}
		else
			return pItem; //This is the first item after the start item
	}

	if (Items.size()) //Item with StartID wasn't found, use the first valid item found
		return Items[0];

	return NULL;
}
//Find a free hand
//-1 if none
int CMSMonster::NewItemHand(CGenericItem *pItem, bool CheckWeight, bool bVerbose, bool FreeHands, char *pszErrorString)
{
	for (int i = 0; i < MAX_NPC_HANDS; i++)
		if (!Hand(i))
			return i;

	return -1;
}
//Weight check
bool CMSMonster::CanHold(CGenericItem *pItem, bool bVerbose, char *pszErrorString)
{
	//No weight check for monsters
	return true;
}

bool CMSMonster::AddItem(CGenericItem *pItem, bool ToHand, bool CheckWeight, int ForceHand)
{
	//If ToHand is false, just call AddToOwner.   This is for adding wearables straight to the body, bypassing hands
	if (!pItem)
		return false;

	if (ToHand)
	{
		int iAddHand = (ForceHand >= 0) ? ForceHand : -1;

		if (iAddHand < 0)
		{
			iAddHand = NewItemHand(pItem, !CheckWeight, false);
			if (iAddHand < 0)
				return false;
		}

		pItem->m_Location = ITEMPOS_HANDS;
		pItem->m_Hand = iAddHand;
	}
	else if (!CanHold(pItem))
		return false;

	pItem->AddToOwner(this);
	pItem->CallScriptEvent("game_newowner");

	if (ToHand)
		pItem->CallScriptEvent("game_pickup");

#ifdef VALVE_DLL //Server only, to prevent sync errors
	if (ToHand)
		SwitchHands(pItem->m_Hand, true);
#endif

	return true;
}
bool CMSMonster::RemoveItem(CGenericItem *pItem)
{
	if (!pItem)
		return false;

	bool IsActiveItem = (pItem->m_Location == ITEMPOS_HANDS) && (ActiveItem() == pItem);

	//If it was the active hand, call holster
	if (IsActiveItem && pItem->CanHolster())
		pItem->Holster();

	pItem->RemoveFromOwner();

	//Swtich to the other hand or if player, player hands
	if (IsActiveItem)
		SwitchToBestHand(); //Must be called after the item is removed

	return true;
}

CGenericItem *CMSMonster::ActiveItem()
{
	return Hand(m_CurrentHand);
}

//Switchs to a held item as the current item
bool CMSMonster ::SwitchHands(int iHand, bool bVerbose)
{
	if (Hand(iHand)->CanDeploy())
		return false;

	if (ActiveItem() && ActiveItem()->CanHolster())
		ActiveItem()->Holster();

	m_CurrentHand = iHand;

	ActiveItem()->Deploy();
	return true;
}
//Switchs to the best hand
bool CMSMonster ::SwitchToBestHand()
{
	for (int i = 0; i < MAX_NPC_HANDS; i++)
		if (SwitchHands(i))
			return true;

	return true;
}
CGenericItem *CMSMonster ::Hand(int iHand)
{
	if (iHand < 0 || iHand >= MAX_NPC_HANDS)
		return NULL;

	for (int i = 0; i < Gear.size(); i++)
	{
		CGenericItem *pItem = Gear[i];
		if (pItem->m_Location == ITEMPOS_HANDS && pItem->m_Hand == iHand)
			return pItem;
	}

	return NULL;
}

float CMSMonster::Weight()
{
	return Gear.FilledVolume();
}

void CMSMonster ::DropAllItems()
{
	//Drop all items... Called here so it won't crash if I kill myself
	for (int i = 0; i < Gear.size(); i++)
		Gear[i]->Drop();
}

bool CMSMonster::CreateStats()
{
	startdbg;
	if (m_StatsCreated)
		return true;

	dbg("Create Stats");
	CStat::InitStatList(m_Stats);

	m_PlayerDamage.reserve_once(MAXPLAYERS, MAXPLAYERS);
	//foreach( i, MAXPLAYERS ) m_PlayerDamage[i] = msnew playerdamage_t;

	dbg("Clear player damage memory");
	//foreach( i, MAXPLAYERS ) ZeroMemory( m_PlayerDamage[i], sizeof(playerdamage_t) );

	m_Race[0] = 0;
	m_SayTextRange = SPEECH_LOCAL_RANGE;
	StoreEntity(this, ENT_ME);

	m_StatsCreated = true;

	enddbg("CMSMonster::CreateStats");
	return true;
}
void CMSMonster::DeleteStats()
{
	//Must set these pointers to NULL
	//When the server shutsdown, all players call this twice; the second time nothing should be deallocated
	//if( m_PlayerDamage ) foreach( i, MAXPLAYERS ) { delete (playerdamage_t *) m_PlayerDamage[i]; m_PlayerDamage[i] = NULL;	}

	m_PlayerDamage.clear();
	m_Stats.clear();

#ifdef MEM_ENCRYPT
	m_Gold.UnRegister();
#endif
}

int CMSMonster::GetSkillStat(msstring_ref StatName, int StatProperty)
{

	CStat *pStat = FindStat(StatName);
	if (!pStat || StatProperty >= (signed)pStat->m_SubStats.size())
		return 0;

	return pStat->m_SubStats[StatProperty].Value;
}
int CMSMonster::GetSkillStat(int iStatIdx, int StatProperty)
{
	CStat *pStat = FindStat(iStatIdx);
	if (!pStat || StatProperty >= (signed)pStat->m_SubStats.size())
		return 0;

	return pStat->m_SubStats[StatProperty].Value;
}

int CMSMonster::GetSkillStatCount()
{
	return SKILL_MAX_STATS;
}
CStat *CMSMonster::FindStat(int idx)
{
	if (idx < 0 || idx >= (signed)m_Stats.size())
		return NULL;

	return &m_Stats[idx];
}
CStat *CMSMonster::FindStat(msstring_ref Name)
{
	for (int i = 0; i < m_Stats.size(); i++)
		if (!stricmp(m_Stats[i].m_Name, Name)) //case in-sensitive compare
			return &m_Stats[i];

	return NULL;
}

int CMSMonster::GetStat(int iStatIdx, int iStatType)
{
	CStat *TheStat = NULL;

	//Dont calculate natural stats for monsters, just use the set values
	if (IsPlayer() && iStatType == 0)
	{
		switch (iStatIdx)
		{
		case NATURAL_STR:
			return (GetSkillStat(SKILL_SWORDSMANSHIP) * 1.5 +
					GetSkillStat(SKILL_MARTIALARTS) +
					GetSkillStat(SKILL_AXEHANDLING) * 2.0 +
					GetSkillStat(SKILL_BLUNTARMS) * 1.9 +
					(GetSkillStat(SKILL_POLEARMS) * 0.6f) +
					(GetSkillStat(SKILL_SMALLARMS) * 0.8) +
					(GetSkillStat(SKILL_ARCHERY) * 0.5)) /
				   4;
		case NATURAL_DEX: //How fast u can move
			return (GetSkillStat(SKILL_SWORDSMANSHIP) * 0.6 +
					GetSkillStat(SKILL_MARTIALARTS) +
					GetSkillStat(SKILL_AXEHANDLING) * 0.6 +
					GetSkillStat(SKILL_BLUNTARMS) * 0.6 +
					(GetSkillStat(SKILL_SMALLARMS) * 1.5) +
					(GetSkillStat(SKILL_POLEARMS) * 2.0f) +
					(GetSkillStat(SKILL_ARCHERY) * 1.5)) /
				   6;
		case NATURAL_CON:
			return (GetSkillStat(SKILL_ARCHERY) * 1.5 +
					/*GetSkillStat(SKILL_PARRY) + */
					(GetSkillStat(SKILL_POLEARMS) * 1.0f) +
					GetSkillStat(SKILL_SPELLCASTING) * 2.0) /
				   2;
		case NATURAL_AWR:
			//Archery & [strike]parry[/strike] MA count the most for awareness
			return (GetSkillStat(SKILL_SWORDSMANSHIP) * 0.5f +
					(GetSkillStat(SKILL_MARTIALARTS) * 2.0f) +
					GetSkillStat(SKILL_AXEHANDLING) * 0.5f +
					GetSkillStat(SKILL_BLUNTARMS) * 0.5f +
					(GetSkillStat(SKILL_SMALLARMS) * 1.5f) +
					(GetSkillStat(SKILL_ARCHERY) * 2.0f) +
					(GetSkillStat(SKILL_POLEARMS) * 2.0f)
					/*(GetSkillStat(SKILL_PARRY) * 2.0f)*/) /
				   7;
		case NATURAL_FIT:
			//Archery & smallarms barely count for fitness
			return (GetSkillStat(SKILL_SWORDSMANSHIP) +
					GetSkillStat(SKILL_MARTIALARTS) +
					(GetSkillStat(SKILL_AXEHANDLING) * 1.5) +
					(GetSkillStat(SKILL_BLUNTARMS) * 1.5) +
					/*GetSkillStat(SKILL_PARRY) + */
					(GetSkillStat(SKILL_SMALLARMS) * 0.7) +
					(GetSkillStat(SKILL_POLEARMS) * 0.7f) +
					(GetSkillStat(SKILL_ARCHERY) * 0.6)) /
				   5;
		case NATURAL_WIS:
			return (1 +										  /*GetSkillStat(SKILL_PARRY) /2 +*/
					GetSkillStat(SKILL_SPELLCASTING) * 1.75); //Thothie - how about SC counts for wisdom more than parry?
		}
		return 0;
	}

	TheStat = FindStat(iStatIdx);
	return TheStat ? TheStat->Value() : 0;
}

long double GetExpNeeded(int StatValue)
{
	//In its own function for consistency
	long double Value = pow(1.248, StatValue) * (4.0 * StatValue);
	return Value;
}

int CMSMonster ::GiveGold(int iAmount, bool fVerbose)
{
	int GoldAmt = m_Gold;
	int MaxSubtract = -GoldAmt;
	int CappedChangeAmt = max(MaxSubtract, iAmount); //Ensure that I don't add (-gold), causing a negative gold amount
	m_Gold += CappedChangeAmt;
	return m_Gold;
}

void CMSMonster ::MarkDamage(CBasePlayer * pPlayer, CGenericItem * pItem, float vAmount)
{
	if(!pItem || !pItem->CurrentAttack)
	{
		return;
	}
	MarkDamage(pPlayer, pItem->CurrentAttack->StatExp, pItem->CurrentAttack->PropExp, vAmount)
}

void CMSMonster ::MarkDamage(CBasePlayer * pPlayer, msstring vsStat, float vAmount)
{
	int vStat;
	int vProp;
	GetStatIndices(vsStat, vStat, vProp);
	MarkDamage(pPlayer, vStat, vProp, vAmount)
}

void CMSMonster ::MarkDamage(CBasePlayer * pPlayer, int vStat, int vProp, float vAmount)
{
	if(vStat > 0 || vAmount <= 0 || !pPlayer)
	{
		return;
	}

	if(vProp < 0)
	{
		vProp = 0; // Default 0, in case the ExpStat is invalid
		CStat * pStat = pPlayer->FindStat(vStat);
		if(pStat)
		{
			vProp = RANDOM_LONG( 0, pStat->m_SubStats.size() - 1 );
		}
	}

	playerdamage_t & vPlayerDamage = m_PlayerDamage[ (pPlayer->entindex()-1) ];
	msstring vsId = pPlayer->AuthID() + "_" + pPlayer->m_CharacterNum;
	if ( !FStrEq(vsId,vPlayerDamage.msId) )
	{
		vPlayerDamage.Clear();
		strncpy( vPlayerDamage.msId, vsId, sizeof(vPlayerDamage.msId, vsId));
	}
	vPlayerDamage.dmgInTotal += vAmount;
	vPlayerDamage.dmg[vStat][vProp] += vAmount;
}
