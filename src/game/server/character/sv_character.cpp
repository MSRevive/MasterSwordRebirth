//
//  Character functions for the Server
//

#include "inc_weapondefs.h"
#include "stats/stats.h"
#include "global.h"
#include "mscharacter.h"
#include "magic.h"
#include "script.h"
#include "fn/FNSharedDefs.h"
#include "mslogger.h"

#include <msgpack.hpp>

#ifndef _WIN32
#include "sys/io.h"
#endif

//NOTENOTE: remove this when char corruption bug is fixed - Solokiller 5/10/2017
#include "game.h"
//END NOTE

void CBasePlayer::CreateChar(createchar_t &CharData)
{
	//Create a new character, using the options the user specified.
	//A custom savedata_t struct can be created for the new
	//character and passed to SaveAll, but any items the character
	//starts with need to be added to Gear

	savedata_t Data;
	memset(&Data, 0, sizeof(savedata_t));

	strncpy(Data.Name, CharData.Name, sizeof(Data.Name));
	strncpy(Data.Race, "Human", sizeof(Data.Race)); // LEGACY
	strncpy(Data.MapName, MSGlobals::MapName, sizeof(Data.MapName));
	Data.Gender = CharData.Gender;
	Data.Gold = MSGlobals::DefaultGold;

#ifndef VALVE_DLL
	MSCLGlobals::RemoveAllEntities();
#else
	RemoveAllItems(false, true);
#endif
	//Give 1 skill to each stat
	for (int i = 0; i < m_Stats.size(); i++)
	{
		CStat &Stat = m_Stats[i];
		if (Stat.m_SubStats.size() == 1) //Parry - Only give 1 to proficiency
			Stat.m_SubStats[0].Value = 1;
		else if (Stat.m_SubStats.size() <= STATPROP_TOTAL) //Weapon Skills - Give 1 to power
			Stat.m_SubStats[STATPROP_POWER].Value = 1;
		else if (Stat.m_SubStats.size() > STATPROP_TOTAL) //Spellcasting - Give 1 to each spell category
			for (int r = 0; r < Stat.m_SubStats.size(); r++)
				Stat.m_SubStats[r].Value = 1;
	}

	//Give free items.  Wear any packs
	for (int i = 0; i < MSGlobals::DefaultFreeItems.size(); i++)
	{
		CGenericItem *pStartingItem = NewGenericItem(MSGlobals::DefaultFreeItems[i]);
		if (!pStartingItem)
			continue;

		if (FBitSet(pStartingItem->MSProperties(), ITEM_WEARABLE))
		{
			//Wear packs
			//m_fClientInitiated = true;				//Dont send a client update
			if (!AddItem(pStartingItem, false, true) || !pStartingItem->WearItem())
				pStartingItem->SUB_Remove();
			//m_fClientInitiated = false;
		}
		else if (FBitSet(pStartingItem->MSProperties(), ITEM_SPELL))
		{
			LearnSpell(pStartingItem->ItemName, true);
			pStartingItem->SUB_Remove();
		}
		else
		{
			//Normal item
			if (!AddItem(pStartingItem, true, true))
				pStartingItem->SUB_Remove();
		}
	}

	//Put the chosen weapon in the player's right hand
	CGenericItem *pStartingItem = NewGenericItem(CharData.Weapon);

	if (FBitSet(pStartingItem->MSProperties(), ITEM_SPELL))
		//It's a spell
	{
		LearnSpell(pStartingItem->ItemName, false);
		pStartingItem->SUB_Remove();
	}
	else
	{
		if (!AddItem(pStartingItem, true, true))
			pStartingItem->SUB_Remove();
	}

	m_CharacterNum = CharData.iChar;
	m_CharacterState = CHARSTATE_LOADED;	 //Temporary, so the save will succeed
	MSChar_Interface::SaveChar(this, &Data); //Save the new character

	m_CharacterState = CHARSTATE_UNLOADED; //Created a new character... this one is no longer valid

	//Clean up
	RemoveAllItems(false, true);

#ifdef VALVE_DLL
	//Update player's character list, so the new char is sent down to client
	if (!FNShared::IsEnabled())
		PreLoadChars();
#endif
}

bool DeleteChar(CBasePlayer *pPlayer, int iCharacter)
{
#ifdef VALVE_DLL
	if (FNShared::IsEnabled())
	{
		FNShared::DeleteCharacter(pPlayer, iCharacter);
		return true;
	}
#endif

	const char *pszCharFileName = GetSaveFileName(iCharacter, pPlayer);
	int ret = remove(pszCharFileName);	  //Delete savefile
	remove(BACKUP_NAME(pszCharFileName)); //Delete backup

	//Update player's char list
	if (iCharacter < MAX_CHARSLOTS)
		pPlayer->m_CharInfo[iCharacter].Status = CDS_NOTFOUND;

	return (!ret) ? true : false;
}

void MSChar_Interface::AutoSave(CBasePlayer* pPlayer)
{
	if (gpGlobals->time <= pPlayer->m_TimeNextSave) return;

	SaveChar(pPlayer, NULL); // Don't auto save too often when using FN.
	pPlayer->m_TimeNextSave = gpGlobals->time + RANDOM_FLOAT(5.0f, 10.0f);
}

//
//  msgpack character format.  Field keys and versioning rules are in mscharacterheader.h
//  A save is CHARPACK_MAGIC followed by one msgpack map.
//

#define CHARPACK_MAGIC "MSRC" //Legacy saves always start with 0 (CHARDATA_HEADER1), so this can't collide
#define CHARPACK_MAGIC_LEN 4

//Non-throwing readers.  A value of the wrong type reads as the default, so a malformed
//or changed field can never take down the whole load.

static long long PackReadInt(const msgpack::object &Obj, long long Default = 0)
{
	switch (Obj.type)
	{
	case msgpack::type::POSITIVE_INTEGER: return (long long)Obj.via.u64;
	case msgpack::type::NEGATIVE_INTEGER: return Obj.via.i64;
	case msgpack::type::FLOAT32:
	case msgpack::type::FLOAT64: return (long long)Obj.via.f64;
	case msgpack::type::BOOLEAN: return Obj.via.boolean ? 1 : 0;
	default: return Default;
	}
}

static float PackReadFloat(const msgpack::object &Obj, float Default = 0.0f)
{
	switch (Obj.type)
	{
	case msgpack::type::FLOAT32:
	case msgpack::type::FLOAT64: return (float)Obj.via.f64;
	case msgpack::type::POSITIVE_INTEGER: return (float)Obj.via.u64;
	case msgpack::type::NEGATIVE_INTEGER: return (float)Obj.via.i64;
	default: return Default;
	}
}

//Copies a string into a fixed buffer, truncating if needed.  Always null-terminates.
static void PackReadStr(const msgpack::object &Obj, char *pOut, size_t OutSize)
{
	if (!OutSize)
		return;

	size_t Len = 0;
	if (Obj.type == msgpack::type::STR)
	{
		Len = Obj.via.str.size;
		if (Len > OutSize - 1)
			Len = OutSize - 1;
		memcpy(pOut, Obj.via.str.ptr, Len);
	}
	pOut[Len] = 0;
}

static msstring PackReadMsStr(const msgpack::object &Obj)
{
	char cTemp[MSSTRING_SIZE];
	PackReadStr(Obj, cTemp, sizeof(cTemp));
	return msstring(cTemp);
}

//Returns the array/map, or an empty one if the object is some other type
static msgpack::object_array PackArray(const msgpack::object &Obj)
{
	if (Obj.type == msgpack::type::ARRAY)
		return Obj.via.array;

	msgpack::object_array Empty;
	Empty.size = 0;
	Empty.ptr = NULL;
	return Empty;
}

static msgpack::object_map PackMap(const msgpack::object &Obj)
{
	if (Obj.type == msgpack::type::MAP)
		return Obj.via.map;

	msgpack::object_map Empty;
	Empty.size = 0;
	Empty.ptr = NULL;
	return Empty;
}

//
//  Helpers shared by the msgpack and legacy readers
//

// MiB JUL2010_02 - Hacky, but if we add more stats and we load a character that doesn't have said stat in the file
//		we set it to the default new stat value (level 0 in Prof and Balance, 1 in Power)
static void InitUnsavedStats(statlist &Stats, int SavedStats)
{
	for (int i = SavedStats; i < Stats.size(); i++)
	{
		CStat &Stat = Stats[i];
		Stat.m_SubStats[Stat.m_SubStats.size() - 1].Value = 1;
	}
}

static void ApplyItemAlias(msstring &ItemName)
{
	msstringstringhash::iterator iAlias = CGenericItemMgr::mItemAlias.find(ItemName);
	if (iAlias != CGenericItemMgr::mItemAlias.end())
		ItemName = iAlias->second;
}

//
//  Read msgpack sections
//

static void ReadItemListPack(const msgpack::object &Obj, mslist<genericitem_full_t> &outItems);

static bool ReadItemPack(const msgpack::object &Obj, genericitem_full_t &outItem)
{
	clrmem(outItem);

	msgpack::object_map Fields = PackMap(Obj);
	for (uint32_t f = 0; f < Fields.size; f++)
	{
		const msgpack::object &Value = Fields.ptr[f].val;
		switch (PackReadInt(Fields.ptr[f].key, -1))
		{
		case CI_NAME: outItem.Name = PackReadMsStr(Value); break;
		case CI_PROPERTIES: outItem.Properties = (ulong)PackReadInt(Value); break;
		case CI_LOCATION: outItem.Location = (unsigned short)PackReadInt(Value); break;
		case CI_HAND: outItem.Hand = (byte)PackReadInt(Value); break;
		case CI_ID: outItem.ID = (ulong)PackReadInt(Value); break;
		case CI_QUALITY: outItem.Quality = (unsigned short)PackReadInt(Value); break;
		case CI_MAXQUALITY: outItem.MaxQuality = (unsigned short)PackReadInt(Value); break;
		case CI_QUANTITY: outItem.Quantity = (unsigned short)PackReadInt(Value); break;
		case CI_CONTENTS: ReadItemListPack(Value, outItem.ContainerItems); break;
		}
	}

	if (!outItem.Name.len())
		return false;

	ApplyItemAlias(outItem.Name);
	return true;
}

static void ReadItemListPack(const msgpack::object &Obj, mslist<genericitem_full_t> &outItems)
{
	msgpack::object_array Items = PackArray(Obj);
	for (uint32_t i = 0; i < Items.size; i++)
	{
		genericitem_full_t Item;
		if (!ReadItemPack(Items.ptr[i], Item))
		{
			MS_ERROR("ReadCharData: Bad item, skipping...");
			continue;
		}
		outItems.add(Item);
	}
}

static void ReadStringListPack(const msgpack::object &Obj, std::vector<std::string> &outList)
{
	msgpack::object_array Strings = PackArray(Obj);
	outList.clear();
	outList.reserve(Strings.size);
	char cTemp[MSSTRING_SIZE];
	for (uint32_t i = 0; i < Strings.size; i++)
	{
		PackReadStr(Strings.ptr[i], cTemp, sizeof(cTemp));
		outList.emplace_back(cTemp);
	}
}

bool MSChar_Interface::ReadCharData(void *pData, ulong Size, chardata_t *CharData)
{
	return CharData->ReadData(pData, Size);
}

//Reads both msgpack and legacy saves
bool chardata_t::ReadData(void *pData, ulong Size)
{
	if (Size >= CHARPACK_MAGIC_LEN && !memcmp(pData, CHARPACK_MAGIC, CHARPACK_MAGIC_LEN))
		return ReadDataPack((const char *)pData + CHARPACK_MAGIC_LEN, Size - CHARPACK_MAGIC_LEN);

	return ReadDataLegacy(pData, Size);
}

bool chardata_t::ReadDataPack(const char *pData, size_t Size)
{
	memset(static_cast<savedata_t *>(this), 0, sizeof(savedata_t));
	Version = SAVECHAR_VERSION;
	strncpy(Race, "Human", sizeof(Race)); // LEGACY

	try
	{
		//Every element takes at least a byte, so no count can legitimately exceed Size.  Stops a corrupt
		//save from requesting a huge allocation or nesting deep enough to blow the stack.
		const msgpack::unpack_limit Limit(Size, Size, Size, Size, Size, 64);
		msgpack::object_handle Handle = msgpack::unpack(pData, Size, MSGPACK_NULLPTR, MSGPACK_NULLPTR, Limit);
		msgpack::object_map Root = PackMap(Handle.get());

		//Check the format before reading anything
		int Format = -1;
		for (uint32_t i = 0; i < Root.size; i++)
			if (PackReadInt(Root.ptr[i].key, -1) == CF_FORMAT)
				Format = (int)PackReadInt(Root.ptr[i].val, -1);

		if (Format < 1 || Format > CHARFMT_VERSION)
		{
			MS_ERROR("ReadCharData: unsupported character format %i (this build supports up to %i)", Format, CHARFMT_VERSION);
			return false;
		}

		//Keys not handled here were written by a newer build, or are retired, and are skipped
		for (uint32_t i = 0; i < Root.size; i++)
		{
			const msgpack::object &Value = Root.ptr[i].val;

			switch (PackReadInt(Root.ptr[i].key, -1))
			{
			//Header
			case CF_NAME: PackReadStr(Value, Name, sizeof(Name)); break;
			case CF_MAPNAME: PackReadStr(Value, MapName, sizeof(MapName)); break;
			case CF_NEXTMAP: PackReadStr(Value, NextMap, sizeof(NextMap)); break;
			case CF_OLDTRANS: PackReadStr(Value, OldTrans, sizeof(OldTrans)); break;
			case CF_NEWTRANS: PackReadStr(Value, NewTrans, sizeof(NewTrans)); break;
			case CF_STEAMID: PackReadStr(Value, SteamID, sizeof(SteamID)); break;
			case CF_PARTY: PackReadStr(Value, Party, sizeof(Party)); break;
			case CF_PARTYID: PartyID = (ulong)PackReadInt(Value); break;
			case CF_ISELITE: IsElite = (byte)PackReadInt(Value); break;
			case CF_GOLD: Gold = (int)PackReadInt(Value); break;
			case CF_MAXHP: MaxHP = (short)PackReadInt(Value); break;
			case CF_MAXMP: MaxMP = (short)PackReadInt(Value); break;
			case CF_HP: HP = (short)PackReadInt(Value); break;
			case CF_MP: MP = (short)PackReadInt(Value); break;
			case CF_GENDER: Gender = (byte)PackReadInt(Value); break;
			case CF_PLAYERKILLS: PlayerKills = (short)PackReadInt(Value); break;
			case CF_TIMEFORGETKILL: TimeWaitedToForgetKill = PackReadFloat(Value); break;

			//Sections
			case CF_VISITEDMAPS: ReadStringListPack(Value, m_VisitedMaps); break;
			case CF_SPELLS: ReadStringListPack(Value, m_Spells); break;
			case CF_ITEMS: ReadItemListPack(Value, m_Items); break;

			case CF_SKILLS:
			{
				CStat::InitStatList(m_Stats);

				msgpack::object_array Stats = PackArray(Value);
				for (uint32_t s = 0; s < Stats.size; s++)
				{
					CStat *pStat = GetStat(s);
					if (!pStat)
						continue;

					msgpack::object_array SubStats = PackArray(Stats.ptr[s]);
					for (uint32_t r = 0; r < SubStats.size; r++)
					{
						CSubStat *pSubStat = pStat->GetSubStat(r);
						if (!pSubStat)
							continue;

						msgpack::object_array SubStat = PackArray(SubStats.ptr[r]); //[value, exp]
						if (SubStat.size > 0)
							pSubStat->Value = (int)PackReadInt(SubStat.ptr[0]);
						if (SubStat.size > 1)
							pSubStat->Exp = (ulong)PackReadInt(SubStat.ptr[1]);
					}
				}

				InitUnsavedStats(m_Stats, Stats.size);
				break;
			}

			case CF_STORAGES:
			{
				msgpack::object_array Storages = PackArray(Value);
				for (uint32_t s = 0; s < Storages.size; s++)
				{
					storage_t Storage;
					msgpack::object_map Fields = PackMap(Storages.ptr[s]);
					for (uint32_t f = 0; f < Fields.size; f++)
					{
						const msgpack::object &FieldValue = Fields.ptr[f].val;
						switch (PackReadInt(Fields.ptr[f].key, -1))
						{
						case CS_NAME: Storage.Name = PackReadMsStr(FieldValue); break;
						case CS_ITEMS: ReadItemListPack(FieldValue, Storage.Items); break;
						}
					}
					m_Storages.add(Storage);
				}
				break;
			}

			case CF_COMPANIONS:
			{
				msgpack::object_array Companions = PackArray(Value);
				for (uint32_t c = 0; c < Companions.size; c++)
				{
					companion_t &Companion = m_Companions.add(companion_t());
					Companion.Active = false;

					msgpack::object_map Fields = PackMap(Companions.ptr[c]);
					for (uint32_t f = 0; f < Fields.size; f++)
					{
						const msgpack::object &FieldValue = Fields.ptr[f].val;
						switch (PackReadInt(Fields.ptr[f].key, -1))
						{
						case CC_SCRIPT: Companion.ScriptName = PackReadMsStr(FieldValue); break;
						case CC_VARS:
						{
							msgpack::object_array Vars = PackArray(FieldValue);
							for (uint32_t v = 0; v < Vars.size; v++)
							{
								msgpack::object_array Var = PackArray(Vars.ptr[v]); //[name, value]
								if (Var.size < 2)
									continue;
								Companion.SaveVarName.add(PackReadMsStr(Var.ptr[0]));
								Companion.SaveVarValue.add(PackReadMsStr(Var.ptr[1]));
							}
							break;
						}
						}
					}
				}
				break;
			}

			case CF_QUESTS:
			{
				msgpack::object_array Quests = PackArray(Value);
				m_Quests.clear();
				for (uint32_t q = 0; q < Quests.size; q++)
				{
					msgpack::object_array QuestData = PackArray(Quests.ptr[q]); //[name, data]
					if (QuestData.size < 2)
						continue;

					quest_t Quest;
					Quest.Name = PackReadMsStr(QuestData.ptr[0]);
					Quest.Data = PackReadMsStr(QuestData.ptr[1]);
					m_Quests.add(Quest);
				}
				break;
			}

			case CF_QUICKSLOTS:
			{
				msgpack::object_array QuickSlots = PackArray(Value);
				m_QuickSlots.clear();
				for (uint32_t q = 0; q < QuickSlots.size; q++)
				{
					quickslot_t QuickSlot;
					msgpack::object_array SlotData = PackArray(QuickSlots.ptr[q]); //nil or [type, id]
					if (SlotData.size >= 2)
					{
						QuickSlot.Active = true;
						QuickSlot.Type = (quickslottype_e)PackReadInt(SlotData.ptr[0]);
						QuickSlot.ID = (uint)PackReadInt(SlotData.ptr[1]);
					}
					else
						QuickSlot.Active = false;

					m_QuickSlots.push_back(QuickSlot); //Drops anything past MAX_QUICKSLOTS
				}
				break;
			}
			}
		}
	}
	catch (const std::exception &e)
	{
		MS_ERROR("ReadCharData: corrupt character data (%s)", e.what());
		return false;
	}

	return true;
}

//
//  LEGACY: Read pre-msgpack saves
//

static bool IsValidCharVersion(int Version)
{
	return (Version == SAVECHAR_VERSION_MSC) || (Version == SAVECHAR_VERSION_MSR);
}

bool chardata_t::ReadDataLegacy(void *pData, ulong Size)
{
	bool ValidVersion = false;
	memset(static_cast<savedata_t *>(this), 0, sizeof(savedata_t)); //Header fields added after the legacy format start at 0

	CPlayer_DataBuffer m_File(Size);
	m_File.Write(pData, Size);
	byte DataID = CHARDATA_UNKNOWN;

	do
	{
		m_File.ReadByte(DataID);
		if (DataID >= CHARDATA_UNKNOWN)
		{
			ValidVersion = false;
			break;
		}

		if (ReadHeader1(DataID, m_File))
			ValidVersion = true;

		ReadMaps1(DataID, m_File);
		ReadSkills1(DataID, m_File);
		ReadSpells1(DataID, m_File);
		ReadItems1(DataID, m_File);
		ReadStorageItems1(DataID, m_File);
		ReadCompanions1(DataID, m_File);
		SkipHelpTips1(DataID, m_File);
		ReadQuests1(DataID, m_File);
		ReadQuickSlots1(DataID, m_File);
	} while (!m_File.Eof());

	m_File.Close();

	return ValidVersion;
}

bool chardata_t::ReadHeader1(byte DataID, CPlayer_DataBuffer &m_File)
{
	if (DataID == CHARDATA_HEADER1)
	{
		m_File.Read(static_cast<savedata_legacy_t *>(this), sizeof(savedata_legacy_t)); //[HEADER}

		if (!IsValidCharVersion(Version))
			return false;

		return true;
	}
	return false;
}

//
//  LEGACY: Read chunks from raw char data
//

static char cTemp[MSSTRING_SIZE];

void chardata_t::ReadMaps1(byte DataID, CPlayer_DataBuffer &m_File)
{
	if (DataID == CHARDATA_MAPSVISITED1)
	{
		//Read Maps Visited
		//Must come DIRECTLY after reading the savedata_t Data
		int Maps = 0;
		m_File.ReadInt(Maps); //[INT]
		m_VisitedMaps.clear();
		if (Maps > 0)
			m_VisitedMaps.reserve(Maps);
		for (int m = 0; m < Maps; m++)
		{
			m_File.ReadString(cTemp, MSSTRING_SIZE); //[STRING]
			m_VisitedMaps.emplace_back(cTemp);
		}
	}
}

void chardata_t::ReadSkills1(byte DataID, CPlayer_DataBuffer &m_File)
{
	if (DataID == CHARDATA_SKILLS1)
	{
		// Read skills
		CStat::InitStatList(m_Stats);

		CStat* pStat = NULL;
		CSubStat* pSubStat = NULL;

		byte Stats = 0;
		m_File.ReadByte(Stats);

		for (int i = 0; i < Stats; i++)
		{
			pStat = GetStat(i);
			byte SubStats = 0;
			m_File.ReadByte(SubStats);

			for (int r = 0; r < SubStats; r++)
			{
				short Value = 0;
				int Exp = 0;
				m_File.ReadShort(Value); //[SHORT]
				m_File.ReadInt(Exp);	 //[INT]

				pSubStat = (pStat ? pStat->GetSubStat(r) : NULL);

				// Handle legacy loading, hacky & messy:
				if ((Version == SAVECHAR_VERSION_MSC) && (i == SKILL_SPELLCASTING))
				{
					switch (r)
					{
					case 5: // Used to be DIVINATION
						pSubStat = (pStat ? pStat->GetSubStat(STAT_MAGIC_DIVINATION) : NULL);
						break;

					case 6: // Used to be AFFLICTION
						pSubStat = (pStat ? pStat->GetSubStat(STAT_MAGIC_AFFLICTION) : NULL);
						break;
					}
				}

				if (pStat && pSubStat)
				{
					pSubStat->Value = Value;
					pSubStat->Exp = Exp;
				}
			}
		}

		InitUnsavedStats(m_Stats, Stats);
	}
}

void chardata_t::ReadSpells1(byte DataID, CPlayer_DataBuffer &m_File)
{
	if (DataID == CHARDATA_SPELLS1)
	{
		//Read Magic spells
		byte Spells = 0;
		m_File.ReadByte(Spells); //[BYTE]
		m_Spells.reserve(Spells);
		for (int s = 0; s < Spells; s++)
		{
			m_File.ReadString(cTemp, MSSTRING_SIZE); //[STRING]
			m_Spells.emplace_back(cTemp);
		}
	}
}

void chardata_t::ReadItems1(byte DataID, CPlayer_DataBuffer &m_File)
{
	if (DataID == CHARDATA_ITEMS1 || DataID == CHARDATA_ITEMS2)
	{
		//Read Items
		byte GearItems = 0;
		m_File.ReadByte(GearItems); //[SHORT]
		for (int i = 0; i < GearItems; i++)
		{
			genericitem_full_t Item;
			if (!ReadItem1(DataID, m_File, Item)) //[X ITEMS]
				continue;

			m_Items.add(Item);
		}
	}
}

void chardata_t::ReadStorageItems1(byte DataID, CPlayer_DataBuffer &m_File)
{
	if (DataID == CHARDATA_STORAGE1)
	{
		//Read storage items

		short Storages = 0;
		m_File.ReadShort(Storages); //[SHORT]
		for (int i = 0; i < Storages; i++)
		{
			storage_t Storage;
			m_File.ReadString(Storage.Name, MSSTRING_SIZE); //[X STRINGS]

			short Items = 0;
			m_File.ReadShort(Items); //[X SHORTS]
			for (int i = 0; i < Items; i++)
			{
				genericitem_full_t Item;
				if (!ReadItem1(DataID, m_File, Item)) //[Y ITEMS]
					continue;

				Storage.Items.add(Item);
			}

			m_Storages.add(Storage);
		}
	}
}

void chardata_t::ReadCompanions1(byte DataID, CPlayer_DataBuffer &m_File)
{
	if (DataID == CHARDATA_COMPANIONS1)
	{
		//Read Companions
		short Companions = 0;
		m_File.ReadShort(Companions); //[SHORT]
		for (int c = 0; c < Companions; c++)
		{
			companion_t &Companion = m_Companions.add(companion_t());
			m_File.ReadString(cTemp, MSSTRING_SIZE); //[STRING]
			Companion.ScriptName = cTemp;
			Companion.Active = false;

			//Read the saved variables
			short Vars = 0;
			m_File.ReadShort(Vars); //[SHORT]
			for (int var = 0; var < Vars; var++)
			{
				m_File.ReadString(cTemp, MSSTRING_SIZE); //[STRING]
				Companion.SaveVarName.add(cTemp);
				m_File.ReadString(cTemp, MSSTRING_SIZE); //[STRING]
				Companion.SaveVarValue.add(cTemp);
			}
		}
	}
}

void chardata_t::SkipHelpTips1(byte DataID, CPlayer_DataBuffer &m_File)
{
	if (DataID == CHARDATA_HELPTIPS1)
	{
		//Help tips are no longer saved, but the chunk still has to be read past
		short HelpTips = 0;
		m_File.ReadShort(HelpTips); //[SHORT]
		for (int t = 0; t < HelpTips; t++)
			m_File.ReadString(cTemp, MSSTRING_SIZE); //[STRING]
	}
}

void chardata_t::ReadQuests1(byte DataID, CPlayer_DataBuffer &m_File)
{
	if (DataID == CHARDATA_QUESTS1)
	{
		//Read Quests
		int Quests = 0;
		m_File.ReadInt(Quests); //[INT]
		m_Quests.clear();
		for (int q = 0; q < Quests; q++)
		{
			quest_t Quest;
			m_File.ReadString(cTemp, MSSTRING_SIZE); //[STRING]
			Quest.Name = cTemp;
			m_File.ReadString(cTemp, MSSTRING_SIZE); //[STRING]
			Quest.Data = cTemp;
			m_Quests.add(Quest);
		}
	}
}

void chardata_t::ReadQuickSlots1(byte DataID, CPlayer_DataBuffer &m_File)
{
	if (DataID == CHARDATA_QUICKSLOTS1)
	{
		//Read Quickslots
		byte QuickSlots = 0;
		m_File.ReadByte(QuickSlots); //[BYTE]
		m_QuickSlots.clear();
		for (int q = 0; q < QuickSlots; q++)
		{
			quickslot_t QuickSlot;
			byte Type = 0;
			m_File.ReadByte(Type); //[BYTE]

			if (Type) //Something in this slot
			{
				int OldID = -1;
				m_File.ReadInt(OldID); //[INT]

				QuickSlot.Active = true;
				QuickSlot.Type = (quickslottype_e)(Type - 1); //Slot type is offset by 1
				QuickSlot.ID = OldID;
			}
			else //Nothing in this slot
			{
				QuickSlot.Active = false;
			}

			m_QuickSlots.push_back(QuickSlot); //Drops anything past MAX_QUICKSLOTS, but still reads it so the next chunk lines up
		}
	}
}

bool chardata_t::ReadItem1(byte DataID, CPlayer_DataBuffer &Data, genericitem_full_t &outItem)
{
	clrmem(outItem);

	char cTemp[128];
	Data.ReadString(cTemp, sizeof(cTemp)); //[STRING]
	if (!cTemp || !cTemp[0])
		return false;

	outItem.Name = cTemp;
	ApplyItemAlias(outItem.Name);

	//It is now possible to read an item from file correctly, but not be able to spawn that item.
	//Be sure that, even if the item can't be created, all of the item's data is read from file properly

	short Properties = 0;
	Data.ReadShort(Properties); //[SHORT]

	outItem.Properties = Properties;

	short Location = 0;
	Data.ReadShort(Location); //[SHORT]
	outItem.Location = Location;

	byte Hand = 0;
	Data.ReadByte(Hand); //[BYTE]
	outItem.Hand = Hand;

	if (DataID == CHARDATA_ITEMS2)
	{
		int LastID = 0;
		Data.ReadInt(LastID); //[INT]
		outItem.ID = LastID;
	}

	if (FBitSet(Properties, ITEM_PERISHABLE) ||
		FBitSet(Properties, ITEM_DRINKABLE))
	{
		short Quality = 0, MaxQuality = 0;
		Data.ReadShort(Quality);	//[SHORT]
		Data.ReadShort(MaxQuality); //[SHORT]

		outItem.Quality = Quality;
		outItem.MaxQuality = MaxQuality;
	}

	if (FBitSet(Properties, ITEM_GROUPABLE))
	{
		short GroupAmt = 0;
		Data.ReadShort(GroupAmt); //[SHORT]

		outItem.Quantity = GroupAmt;
	}

	if (FBitSet(Properties, ITEM_CONTAINER))
	{
		short iItemCount = 0;
		Data.ReadShort(iItemCount); //[SHORT]

		genericitem_full_t PackItem;
		for (int i = 0; i < iItemCount; i++)
		{
			bool Success = ReadItem1(DataID, Data, PackItem);
			if (!Success)
			{
				MS_ERROR("ReadItem() Bad item in container, skipping...");
				continue;
			}
			outItem.ContainerItems.add(PackItem);

			/*if( pItem )
				pPackItem->PutInPack( pItem );
				else
				//If I couldn't spawn this pack, drop the item that's supposed to be inside it
				pPackItem->Drop( 3, (Vector &)g_vecZero, (Vector &)g_vecZero, (Vector &)g_vecZero );*/
		}
	}

	return true;
}

//
// Save Character
// ==============

struct charpack_t
{
	msgpack::sbuffer Buf;
	msgpack::packer<msgpack::sbuffer> Pk;

	charpack_t() : Pk(Buf) {}
};

static void PackStr(charpack_t &Out, const char *pszValue, size_t MaxLen = MSSTRING_SIZE)
{
	uint32_t Len = pszValue ? (uint32_t)strnlen(pszValue, MaxLen) : 0;
	Out.Pk.pack_str(Len);
	if (Len)
		Out.Pk.pack_str_body(pszValue, Len);
}

static void PackStr(charpack_t &Out, msstring &Value)
{
	PackStr(Out, Value.c_str(), Value.len());
}

//A map or array whose size isn't known up front.  Reserves a 16-bit count and patches in the
//real one when the scope ends, so fields can be added or written conditionally without
//keeping a separate count in sync.  Nested scopes must end before their parent (just use C++ scoping).
class CPackScope
{
public:
	CPackScope(charpack_t &Out, bool IsMap) : m_Out(Out), m_Offset(Out.Buf.size()), m_Count(0)
	{
		const char Header[3] = {(char)(IsMap ? 0xde : 0xdc), 0, 0}; //map16 / array16
		m_Out.Buf.write(Header, sizeof(Header));
	}

	~CPackScope()
	{
		if (m_Count > 0xFFFF)
			MS_ERROR("SaveChar: too many entries (%u) in one map/array, save is corrupt!", m_Count);

		//Buffer may have been reallocated since the header was written, so go through the offset
		char *pHeader = m_Out.Buf.data() + m_Offset;
		pHeader[1] = (char)((m_Count >> 8) & 0xFF);
		pHeader[2] = (char)(m_Count & 0xFF);
	}

	//Map: write a key and value
	template <typename T>
	void Field(int FieldKey, const T &Value)
	{
		m_Out.Pk.pack(FieldKey);
		m_Out.Pk.pack(Value);
		m_Count++;
	}

	void FieldStr(int FieldKey, const char *pszValue, size_t MaxLen)
	{
		m_Out.Pk.pack(FieldKey);
		PackStr(m_Out, pszValue, MaxLen);
		m_Count++;
	}

	void FieldStr(int FieldKey, msstring &Value)
	{
		m_Out.Pk.pack(FieldKey);
		PackStr(m_Out, Value);
		m_Count++;
	}

	//Map: write a key, the caller packs exactly one value (or nested scope) after it
	void Key(int FieldKey)
	{
		m_Out.Pk.pack(FieldKey);
		m_Count++;
	}

	//Array: the caller packed one element
	void Added() { m_Count++; }

private:
	charpack_t &m_Out;
	size_t m_Offset;
	uint32_t m_Count;
};

//Returns false if the item isn't saved (spells)
static bool PackItem(charpack_t &Out, genericitem_full_t &Item)
{
	if (FBitSet(Item.Properties, ITEM_SPELL))
		return false;

	CPackScope Fields(Out, true);
	Fields.FieldStr(CI_NAME, Item.Name);
	Fields.Field(CI_PROPERTIES, Item.Properties);
	Fields.Field(CI_LOCATION, Item.Location);
	Fields.Field(CI_HAND, Item.Hand);
	Fields.Field(CI_ID, Item.ID); //Item ID at last save (used by quickslots to identify this item)

	if (FBitSet(Item.Properties, ITEM_PERISHABLE) ||
		FBitSet(Item.Properties, ITEM_DRINKABLE))
	{
		Fields.Field(CI_QUALITY, Item.Quality);
		Fields.Field(CI_MAXQUALITY, Item.MaxQuality);
	}

	if (FBitSet(Item.Properties, ITEM_GROUPABLE))
		Fields.Field(CI_QUANTITY, Item.Quantity);

	if (FBitSet(Item.Properties, ITEM_CONTAINER))
	{
		Fields.Key(CI_CONTENTS);
		CPackScope Contents(Out, false);
		for (int i = 0; i < Item.ContainerItems.size(); i++)
			if (PackItem(Out, Item.ContainerItems[i]))
				Contents.Added();
	}

	return true;
}

static CScript *GetCompanionScript(companion_t &Companion)
{
	CBaseEntity *pEntity = Companion.Entity.Entity();
	if (!pEntity)
		return NULL;
	IScripted *pScripted = pEntity->GetScripted();
	if (!pScripted || !pScripted->m_Scripts.size())
		return NULL;
	return pScripted->m_Scripts[0];
}

//Packs the whole character into Out.Buf.  Keys and versioning rules: mscharacterheader.h
static void PackChar(charpack_t &Out, CBasePlayer *pPlayer, savedata_t &Data)
{
	Out.Buf.clear(); //Keeps its allocation
	Out.Buf.write(CHARPACK_MAGIC, CHARPACK_MAGIC_LEN);

	CPackScope Root(Out, true);

	//Header
	Root.Field(CF_FORMAT, CHARFMT_VERSION);
	Root.FieldStr(CF_NAME, Data.Name, sizeof(Data.Name));
	Root.FieldStr(CF_MAPNAME, Data.MapName, sizeof(Data.MapName));
	Root.FieldStr(CF_NEXTMAP, Data.NextMap, sizeof(Data.NextMap));
	Root.FieldStr(CF_OLDTRANS, Data.OldTrans, sizeof(Data.OldTrans));
	Root.FieldStr(CF_NEWTRANS, Data.NewTrans, sizeof(Data.NewTrans));
	Root.FieldStr(CF_STEAMID, Data.SteamID, sizeof(Data.SteamID));
	Root.FieldStr(CF_PARTY, Data.Party, sizeof(Data.Party));
	Root.Field(CF_PARTYID, Data.PartyID);
	Root.Field(CF_ISELITE, Data.IsElite);
	Root.Field(CF_GOLD, Data.Gold);
	Root.Field(CF_MAXHP, Data.MaxHP);
	Root.Field(CF_MAXMP, Data.MaxMP);
	Root.Field(CF_HP, Data.HP);
	Root.Field(CF_MP, Data.MP);
	Root.Field(CF_GENDER, Data.Gender);
	Root.Field(CF_PLAYERKILLS, Data.PlayerKills);
	Root.Field(CF_TIMEFORGETKILL, Data.TimeWaitedToForgetKill);

	//Maps visited
	Root.Key(CF_VISITEDMAPS);
	Out.Pk.pack_array(pPlayer->m_Maps.size());
	for (const std::string &Map : pPlayer->m_Maps)
		PackStr(Out, Map.c_str(), MSSTRING_MAXLEN);

	//Skills
	Root.Key(CF_SKILLS);
	statlist &StatList = pPlayer->m_Stats;
	Out.Pk.pack_array(StatList.size());
	for (int i = 0; i < StatList.size(); i++)
	{
		CStat &Stat = StatList[i];
		Out.Pk.pack_array(Stat.m_SubStats.size());
		for (int r = 0; r < Stat.m_SubStats.size(); r++)
		{
			CSubStat &SubStat = Stat.m_SubStats[r];
			Out.Pk.pack_array(2);
			Out.Pk.pack(SubStat.Value);
			Out.Pk.pack(SubStat.Exp);
		}
	}

	//Magic spells
	Root.Key(CF_SPELLS);
	spellgroup_v &SpellList = pPlayer->m_SpellList;
	Out.Pk.pack_array(SpellList.size());
	for (int s = 0; s < SpellList.size(); s++)
		PackStr(Out, SpellList[s]);

	//Items
	Root.Key(CF_ITEMS);
	{
		CPackScope Items(Out, false);
		for (int i = 0; i < pPlayer->Gear.size(); i++)
		{
			if (pPlayer->Gear[i] == pPlayer->PlayerHands) //Skip player hands
				continue;

			genericitem_full_t Item = genericitem_full_t(pPlayer->Gear[i]);
			if (PackItem(Out, Item))
				Items.Added();
		}
	}

	//Storage items
	Root.Key(CF_STORAGES);
	Out.Pk.pack_array(pPlayer->m_Storages.size());
	for (int s = 0; s < pPlayer->m_Storages.size(); s++)
	{
		storage_t &Storage = pPlayer->m_Storages[s];

		CPackScope StorageFields(Out, true);
		StorageFields.FieldStr(CS_NAME, Storage.Name);
		StorageFields.Key(CS_ITEMS);
		CPackScope Items(Out, false);
		for (int i = 0; i < Storage.Items.size(); i++)
			if (PackItem(Out, Storage.Items[i]))
				Items.Added();
	}

	//Companions - save any variables that start with "companion.save."
	Root.Key(CF_COMPANIONS);
	Out.Pk.pack_array(pPlayer->m_Companions.size());
	for (int c = 0; c < pPlayer->m_Companions.size(); c++)
	{
		companion_t &Companion = pPlayer->m_Companions[c];

		CPackScope CompanionFields(Out, true);
		CompanionFields.FieldStr(CC_SCRIPT, Companion.ScriptName);
		CompanionFields.Key(CC_VARS);
		CPackScope Vars(Out, false);

		CScript *Script = GetCompanionScript(Companion);
		if (!Script)
			continue;

		for (int v = 0; v < Script->m_Variables.size(); v++)
		{
			scriptvar_t &Var = Script->m_Variables[v];
			if (!Var.Name.starts_with("companion.save."))
				continue;

			Out.Pk.pack_array(2);
			PackStr(Out, Var.Name);
			PackStr(Out, Var.Value);
			Vars.Added();
		}
	}

	//Quests
	Root.Key(CF_QUESTS);
	Out.Pk.pack_array(pPlayer->m_Quests.size());
	for (int q = 0; q < pPlayer->m_Quests.size(); q++)
	{
		Out.Pk.pack_array(2);
		PackStr(Out, pPlayer->m_Quests[q].Name);
		PackStr(Out, pPlayer->m_Quests[q].Data);
	}

	//Quickslots
	Root.Key(CF_QUICKSLOTS);
	Out.Pk.pack_array(MAX_QUICKSLOTS);
	for (int q = 0; q < MAX_QUICKSLOTS; q++)
	{
		quickslot_t &QuickSlot = pPlayer->m_QuickSlots[q];
		if (QuickSlot.Active)
		{
			Out.Pk.pack_array(2);
			Out.Pk.pack((int)QuickSlot.Type);
			Out.Pk.pack(QuickSlot.ID);
		}
		else
			Out.Pk.pack_nil();
	}
}

//If pData != NULL, then this is a new char
void MSChar_Interface::SaveChar(CBasePlayer *pPlayer, savedata_t *pData)
{
	//Can I save right now?
	if (pPlayer->m_CharacterState == CHARSTATE_UNLOADED /*||	//Can't save if no character is created
		MSGlobals::GameType != GAMETYPE_ADVENTURE*/
		)												//If the gametype isn't adventure, we can't save no matter what.
	{
		return;
	}

	//MiB JUL2010_13 - Don't save in dev mode
	if (MSGlobals::DevModeEnabled)
		return;

	//FEB2015_25 Thothie - don't save if <15 hp (char delete bug workaround)
	if (pPlayer->MaxHP() < 15)
		return;

	//Add this map to the list of maps visited
	if (!HasVisited(MSGlobals::MapName, pPlayer->m_Maps))
		pPlayer->m_Maps.emplace_back(MSGlobals::MapName.c_str());

	const char *pszFileName;
	//#ifdef VALVE_DLL
	pszFileName = GetSaveFileName(pPlayer->m_CharacterNum, pPlayer);
	//#else
	//	pszFileName = GetSaveFileName( pPlayer->m_CharacterNum );
	//#endif

	//if( fVerbose ) Print( "Saving to file: %s\n", pszFileName );

	//Initialize
	savedata_t Data;
	memset(&Data, 0, sizeof(savedata_t));
	if (pData)
		memcpy(&Data, pData, sizeof(savedata_t));

	//Copy the data
	Data.Version = SAVECHAR_VERSION;
	//strcpy( Data.MapName, (pPlayer->iHP > 0) ? g_MapName : "" );

	if (!pData)
	{
		strncpy(Data.Name, pPlayer->m_DisplayName, sizeof(Data.Name)); // Store actual character name (DisplayName() is servername, and will have a (#) at the end if there are duplicates on the server)
		strncpy(Data.Race, "Human", sizeof(Data.Race)); // LEGACY
		strncpy(Data.Party, pPlayer->GetPartyName(), sizeof(Data.Party) - 1);
		Data.PartyID = pPlayer->GetPartyID();

		strncpy(Data.MapName, MSGlobals::MapName, sizeof(Data.MapName));

		strncpy(Data.OldTrans, pPlayer->m_OldTransition, 32);
		strncpy(Data.NextMap, pPlayer->m_NextMap, 32);
		strncpy(Data.NewTrans, pPlayer->m_NextTransition, 32);
		
		//MS_INFO("SaveChar: Saving transitions - MapName='%s', OldTrans='%s', NextMap='%s', NewTrans='%s'",
		//        Data.MapName, Data.OldTrans, Data.NextMap, Data.NewTrans);

		Data.IsElite = pPlayer->m_fIsElite;
		Data.Gold = pPlayer->m_Gold;
		Data.MaxHP = pPlayer->m_MaxHP;
		Data.MaxMP = pPlayer->m_MaxMP;
		if (pPlayer->pev->deadflag == DEAD_NO)
		{
			Data.HP = pPlayer->m_HP;
			Data.MP = pPlayer->m_MP;
		}
		else
		{
			//Tried to save while dead.
			//Set Health/Mana to 0 to indicate this
			Data.HP = Data.MP = 0.0;
			//fDead = true;
			//pPlayer->iHP = 1.0;
		}

		//Data.Origin = LastGoodPos;
		//Data.Angles = LastGoodAng;
		//Print( "Save: Data.Origin: %f %f %f\n", Data.Origin.x, Data.Origin.y, Data.Origin.z );
		//Data.SayType = pPlayer->m_SayType;
		Data.Gender = pPlayer->m_Gender;
		Data.PlayerKills = pPlayer->m_PlayersKilled;
		Data.TimeWaitedToForgetKill = pPlayer->m_TimeWaitedToForgetKill;
		//#ifdef VALVE_DLL
		strncpy(Data.SteamID, GETPLAYERAUTHID(pPlayer->edict()), 32);
		//#else
		//	strncpy( Data.SteamID, MSCLGlobals::AuthID, 32 );
		//#endif
	}

	//Let companions update their saved vars.  Done before packing so no script runs mid-pack.
	for (int c = 0; c < pPlayer->m_Companions.size(); c++)
	{
		CBaseEntity *pEntity = pPlayer->m_Companions[c].Entity.Entity();
		IScripted *pScripted = pEntity ? pEntity->GetScripted() : NULL;
		if (pScripted && pScripted->m_Scripts.size())
			pScripted->CallScriptEvent("game_companion_save");
	}

	//The pack buffer is reused between saves, so steady-state saves don't allocate.
	//Everything below copies the data before returning.
	static charpack_t s_CharPack;
	PackChar(s_CharPack, pPlayer, Data);

	const char *pPackData = s_CharPack.Buf.data();
	const size_t PackSize = s_CharPack.Buf.size();

	if (FNShared::IsEnabled())
	{
		// If Central Server is enabled, save to the Central Server instead of locally
		FNShared::CreateOrUpdateCharacter(pPlayer, pPlayer->m_CharacterNum, pPackData, PackSize, (pData == NULL));
		return;
	}

	CPlayer_DataBuffer gFile;
	gFile.SetBuffer((byte *)pPackData, PackSize);
	gFile.WriteToFile(pszFileName, "wb", true);
	gFile.Close();
}
