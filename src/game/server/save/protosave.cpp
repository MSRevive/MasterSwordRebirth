#include "protosave.h"
#include "proto/savedata.pb.h"

#include "inc_weapondefs.h"
#include "stats/stats.h"
#include "stats/statdefs.h"
#include "global.h"
#include "mscharacter.h"
#include "magic.h"
#include "script.h"
#include "fn/FNSharedDefs.h"
#include "mslogger.h"
#include "player/player.h"

#ifndef _WIN32
#include "sys/io.h"
#endif

//NOTENOTE: remove this when char corruption bug is fixed - Solokiller 5/10/2017
#include "game.h"
//END NOTE


#include <format>
#include <strstream>
#include <google/protobuf/util/json_util.h>

namespace msr
{

  namespace {
    // game -> proto
    const auto GetProtoStatId = [](int statIndex)
    {
      switch(statIndex)
      {
        // Stats
        case NATURAL_STR: return MSRProto::Stat::Strength;
        case NATURAL_DEX: return MSRProto::Stat::Agility;
        case NATURAL_CON: return MSRProto::Stat::Concentration;
        case NATURAL_AWR: return MSRProto::Stat::Awareness;
        case NATURAL_FIT: return MSRProto::Stat::Fitness;
        case NATURAL_WIS: return MSRProto::Stat::Wisdom;
        // Skills
        case SKILL_SWORDSMANSHIP: return MSRProto::Stat::Swordsmanship;
        case SKILL_MARTIALARTS: return MSRProto::Stat::MartialArts;
        case SKILL_SMALLARMS: return MSRProto::Stat::SmallArms;
        case SKILL_AXEHANDLING: return MSRProto::Stat::AxeHandling;
        case SKILL_BLUNTARMS: return MSRProto::Stat::BluntArms;
        case SKILL_ARCHERY: return MSRProto::Stat::Archery;
        case SKILL_SPELLCASTING: return MSRProto::Stat::SpellCasting;
        case SKILL_PARRY: return MSRProto::Stat::Parry;
        case SKILL_POLEARMS: return MSRProto::Stat::PoleArms;
        default:
          return MSRProto::Stat::StatId_UNKNOWN;
      }
    };
    const auto GetProtoSubStatId = [](int subStatIndex, bool spellcasting)
    {
      if(spellcasting)
      {
        switch(subStatIndex)
        {
          case STAT_MAGIC_FIRE: return MSRProto::Stat::SubStat::Fire;
          case STAT_MAGIC_ICE: return MSRProto::Stat::SubStat::Ice;
          case STAT_MAGIC_LIGHTNING: return MSRProto::Stat::SubStat::Lightning;
          case STAT_MAGIC_DIVINATION: return MSRProto::Stat::SubStat::Divination;
          case STAT_MAGIC_AFFLICTION: return MSRProto::Stat::SubStat::Affliction;
          default:
            return MSRProto::Stat::SubStat::SubStatId_UNKNOWN;
        }
      }
      else
      {
        switch(subStatIndex)
        {
          case STATPROP_SKILL: return MSRProto::Stat::SubStat::Proficiency;
          case STATPROP_BALANCE: return MSRProto::Stat::SubStat::Balance;
          case STATPROP_POWER: return MSRProto::Stat::SubStat::Power;
          default:
            return MSRProto::Stat::SubStat::SubStatId_UNKNOWN;
        }
      }
    };
    const auto GetProtoHandId = [](auto handId) -> MSRProto::GenericItemFull::HandId {
      switch(handId){
        case hand_e::LEFT_HAND: return MSRProto::GenericItemFull::LEFT_HAND;
        case hand_e::RIGHT_HAND: return MSRProto::GenericItemFull::RIGHT_HAND;
        case hand_e::HAND_PLAYERHANDS: return MSRProto::GenericItemFull::HAND_PLAYERHANDS;
        case hand_e::ANY_HAND: return MSRProto::GenericItemFull::ANY_HAND;
        case hand_e::BOTH_HANDS: return MSRProto::GenericItemFull::BOTH_HANDS;
        default:
          return MSRProto::GenericItemFull::HandId_UNKNOWN;
      }
    };
    const auto GetProtoQuickSlotId = [](quickslottype_e quickSlotType) {
      switch (quickSlotType) {
        case quickslottype_e::QS_ITEM: return MSRProto::CharData::QuickSlot::QS_ITEM;
        case quickslottype_e::QS_SPELL: return MSRProto::CharData::QuickSlot::QS_SPELL;
        case quickslottype_e::QS_ARROW: return MSRProto::CharData::QuickSlot::QS_ARROW;
        default: return MSRProto::CharData::QuickSlot::QuickSlotTypeId_UNKNOWN;
      }
    };
    const auto GetProtoGender = [](gender_e gender) -> MSRProto::CharData::SaveData::GenderId {
      switch(gender)
      {
        case GENDER_MALE: return MSRProto::CharData::SaveData::Male;
        case GENDER_FEMALE: return MSRProto::CharData::SaveData::Female;
        case GENDER_UNKNOWN: return MSRProto::CharData::SaveData::GenderId_UNKNOWN;
        default:
          return MSRProto::CharData::SaveData::GenderId_UNKNOWN;
      }
    };

    // proto -> game
    const auto GetGameStatId = [](MSRProto::Stat::StatId protoStatId) -> std::int32_t
    {
      switch(protoStatId)
      {
        // Stats
        case MSRProto::Stat::Strength: return NATURAL_STR;
        case MSRProto::Stat::Agility: return NATURAL_DEX;
        case MSRProto::Stat::Concentration: return NATURAL_CON;
        case MSRProto::Stat::Awareness: return NATURAL_AWR;
        case MSRProto::Stat::Fitness: return NATURAL_FIT;
        case MSRProto::Stat::Wisdom: return NATURAL_WIS;
        // Skills
        case MSRProto::Stat::Swordsmanship: return SKILL_SWORDSMANSHIP;
        case MSRProto::Stat::MartialArts: return SKILL_MARTIALARTS;
        case MSRProto::Stat::SmallArms: return SKILL_SMALLARMS;
        case MSRProto::Stat::AxeHandling: return SKILL_AXEHANDLING;
        case MSRProto::Stat::BluntArms: return SKILL_BLUNTARMS;
        case MSRProto::Stat::Archery: return SKILL_ARCHERY;
        case MSRProto::Stat::SpellCasting: return SKILL_SPELLCASTING;
        case MSRProto::Stat::Parry: return SKILL_PARRY;
        case MSRProto::Stat::PoleArms: return SKILL_POLEARMS;
        default:
          return MSRProto::Stat::StatId_UNKNOWN;
      }
    };
    const auto GetGameSubStatId = [](MSRProto::Stat::SubStat::SubStatId protoSubStatId, bool spellcasting) -> std::int32_t
    {
      if(spellcasting)
      {
        switch(protoSubStatId)
        {
          case MSRProto::Stat::SubStat::Fire: return STAT_MAGIC_FIRE;
          case MSRProto::Stat::SubStat::Ice: return STAT_MAGIC_ICE;
          case MSRProto::Stat::SubStat::Lightning: return STAT_MAGIC_LIGHTNING;
          case MSRProto::Stat::SubStat::Divination: return STAT_MAGIC_DIVINATION;
          case MSRProto::Stat::SubStat::Affliction: return STAT_MAGIC_AFFLICTION;
          default:
            return -1;
        }
      }
      else
      {
        switch(protoSubStatId)
        {
          case MSRProto::Stat::SubStat::Proficiency: return STATPROP_SKILL;
          case MSRProto::Stat::SubStat::Balance: return STATPROP_BALANCE;
          case MSRProto::Stat::SubStat::Power: return STATPROP_POWER;
          default:
            return -1;
        }
      }
    };
    const auto GetGameHandId = [](MSRProto::GenericItemFull::HandId handId) -> std::int32_t {
      switch(handId){
        case MSRProto::GenericItemFull::LEFT_HAND: return hand_e::LEFT_HAND;
        case MSRProto::GenericItemFull::RIGHT_HAND: return hand_e::RIGHT_HAND;
        case MSRProto::GenericItemFull::HAND_PLAYERHANDS: return hand_e::HAND_PLAYERHANDS;
        case MSRProto::GenericItemFull::ANY_HAND: return hand_e::ANY_HAND;
        case MSRProto::GenericItemFull::BOTH_HANDS: return hand_e::BOTH_HANDS;
        default:
          return -1;
      }
    };
    const auto GetGameQuickSlotId = [](MSRProto::CharData::QuickSlot::QuickSlotTypeId protoQuickslot, bool &active) -> std::int32_t {
      switch (protoQuickslot) {
        case MSRProto::CharData::QuickSlot::QS_EMPTY:
          active=false;
          return 0;
        case MSRProto::CharData::QuickSlot::QS_ITEM: return quickslottype_e::QS_ITEM;
        case MSRProto::CharData::QuickSlot::QS_SPELL: return quickslottype_e::QS_SPELL;
        case MSRProto::CharData::QuickSlot::QS_ARROW: return quickslottype_e::QS_ARROW;
        default:
          return -1;
      }
    };
    const auto GetGameGender = [](MSRProto::CharData::SaveData::GenderId gender) -> gender_e{
      switch(gender)
      {
        case MSRProto::CharData::SaveData::Male: return GENDER_MALE;
        case MSRProto::CharData::SaveData::Female: return GENDER_FEMALE;
        case MSRProto::CharData::SaveData::GenderId_UNKNOWN: return GENDER_UNKNOWN;
        default:
          return GENDER_UNKNOWN;
      }
    };
  }


  void ProtoSave::CreateCharData(CBasePlayer *pPlayer, savedata_t *pData, MSRProto::CharData *out)
  {
    if (pPlayer->m_CharacterState == CHARSTATE_UNLOADED) return; //Can't save if no character is created
    if (MSGlobals::DevModeEnabled) return;
    if (pPlayer->MaxHP() < 15) return; //FEB2015_25 Thothie - don't save if <15 hp (char delete bug workaround)
    if (!MSChar_Interface::HasVisited(MSGlobals::MapName, pPlayer->m_Maps)) pPlayer->m_Maps.add(MSGlobals::MapName);

    // create the main protobuf message
    MSRProto::CharData::SaveData *saveData = out->mutable_savedata();

    saveData->set_version(SAVECHAR_VERSION_MSR_PROTOBUF);
    if (pData == nullptr) // saving existing character
    {
      MS_WARN("Saving existing character: {}", pPlayer->m_DisplayName);
      saveData->set_name(pPlayer->m_DisplayName);
      saveData->set_steamid(GETPLAYERAUTHID(pPlayer->edict()));
      saveData->set_race("Human"); // LEGACY

      saveData->set_gender(GetProtoGender(pPlayer->m_Gender));
      // map
      saveData->set_mapname(MSGlobals::MapName);
      saveData->set_nextmap(pPlayer->m_NextMap);
      saveData->set_oldtransition(pPlayer->m_OldTransition);
      saveData->set_newtransition(pPlayer->m_NextTransition);
      // stats
      // MISSING: Origin
      // MISSING: Angle
      saveData->set_iselite(pPlayer->m_fIsElite);
      saveData->set_gold(pPlayer->m_Gold);
      saveData->set_maxhp(pPlayer->m_MaxHP);
      saveData->set_maxmp(pPlayer->m_MaxMP);
      if (pPlayer->pev->deadflag == DEAD_NO)
      {
        saveData->set_currenthp(pPlayer->m_HP);
        saveData->set_currentmp(pPlayer->m_MP);
      }
      else
      {
        //Tried to save while dead.
        //Set Health/Mana to 0 to indicate this
        saveData->set_currenthp(0);
        saveData->set_currentmp(0);
      }
      // party
      saveData->set_partyname(pPlayer->GetPartyName());
      saveData->set_partyid(pPlayer->GetPartyID());

      // PKs
      saveData->set_playerkills(pPlayer->m_PlayersKilled);
      saveData->set_timewaitedtoforgetkill(pPlayer->m_TimeWaitedToForgetKill);
      saveData->set_timewaitedtoforgetsteal(pPlayer->m_TimeWaitedToForgetSteal);
    }
    else // saving new character
    {
      MS_WARN(std::format("Saving new character: {}", pData->Name).c_str());
      saveData->set_name(pData->Name);
      saveData->set_steamid(pData->SteamID);
      saveData->set_race(pData->Race);
      saveData->set_gender(GetProtoGender(static_cast<gender_e>(pData->Gender)));
      // map
      saveData->set_mapname(pData->MapName);
      saveData->set_nextmap(pData->NextMap);
      saveData->set_oldtransition(pData->OldTrans);
      saveData->set_newtransition(pData->NewTrans);
      // stats
      auto protoOrigin = saveData->mutable_origin();
      protoOrigin->set_x(pData->Origin.x);
      protoOrigin->set_y(pData->Origin.y);
      protoOrigin->set_z(pData->Origin.z);
      auto protoAngle = saveData->mutable_angles();
      protoAngle->set_x(pData->Angles.x);
      protoAngle->set_y(pData->Angles.y);
      protoAngle->set_z(pData->Angles.z);
      saveData->set_iselite(pData->IsElite);
      saveData->set_gold(pData->Gold);
      saveData->set_maxhp(pData->MaxHP);
      saveData->set_maxmp(pData->MaxMP);

      saveData->set_currenthp(pData->HP);
      saveData->set_currentmp(pData->MP);
      // party
      saveData->set_partyname(pData->Party);
      saveData->set_partyid(pData->PartyID);

      // PKs
      saveData->set_playerkills(pData->PlayerKills);
      saveData->set_timewaitedtoforgetkill(pData->TimeWaitedToForgetKill);
      saveData->set_timewaitedtoforgetsteal(pData->TimeWaitedToForgetSteal);
    }

    // visitedMaps
    for (int mapIdx = 0; mapIdx < pPlayer->m_Maps.size(); ++mapIdx) /// @todo replace with range based for
    {
      out->add_visitedmaps(pPlayer->m_Maps[mapIdx]);
    }
    for (int statId = 0; statId < pPlayer->m_Stats.size(); ++statId)
    {
      const CStat &stat = pPlayer->m_Stats[statId];
      auto protoStat = out->add_statlist();
      protoStat->set_statid(GetProtoStatId(statId));
      for (int subStatId = 0; subStatId < stat.m_SubStats.size(); ++subStatId)
      {
        const CSubStat &subStat = stat.m_SubStats[subStatId];
        auto protoSubStat = protoStat->add_substats();
        protoSubStat->set_substatid(GetProtoSubStatId(subStatId, (protoStat->statid() == MSRProto::Stat::SpellCasting)));
        protoSubStat->set_value(subStat.Value);
        protoSubStat->set_exp(subStat.Exp);
      }
    }


    // knownSpells
    for (int spellIdx = 0; spellIdx < pPlayer->m_SpellList.size(); ++spellIdx) /// @todo replace with range based for
    {
      out->add_knownspells(pPlayer->m_SpellList[spellIdx]);
    }

    // items
    struct {
      // recursion is easier to do in a struct rather than passing a lambda to itself
      void PopulateProtoItem (const genericitem_full_t &item, MSRProto::GenericItemFull *out){
        auto gItem = out->mutable_genericitem();
        gItem->set_id(item.ID);
        gItem->set_name(item.Name.c_str());
        gItem->set_properties(item.Properties);
        if (FBitSet(item.Properties, ITEM_GROUPABLE)) gItem->set_quantity(item.Quantity);

        if (FBitSet(item.Properties, ITEM_PERISHABLE) || FBitSet(item.Properties, ITEM_DRINKABLE))
        {
          gItem->set_quality(item.Quality);
          gItem->set_maxquality(item.MaxQuality);
        }

        out->set_location(item.Location);
        out->set_hand(GetProtoHandId(item.Hand));

        if (FBitSet(item.Properties, ITEM_CONTAINER)) //Writing contained items should be the *LAST* thing you do
        {
          for(int containerIdx = 0; containerIdx < item.ContainerItems.size(); ++containerIdx) /// @todo replace with range based for
          {
            auto protoItem = out->add_containeritems();
            PopulateProtoItem(genericitem_full_t(item.ContainerItems[containerIdx]), protoItem);
          }
        }
      }
    } itemHelper;


    for (const auto currentItem : pPlayer->Gear)
    {
      if (currentItem == pPlayer->PlayerHands) continue; //Skip player hands
      if (FBitSet(currentItem->Properties, ITEM_SPELL)) continue; // skip learned spells, they are stored in knownSpells

      auto protoItem = out->add_items();
      itemHelper.PopulateProtoItem(genericitem_full_t(currentItem), protoItem);
    }

    // storage items
    for(int storageIdx = 0; storageIdx < pPlayer->m_Storages.size(); ++storageIdx) /// @todo replace with range based for
    {
      auto protoStorage = out->add_storages();
      const auto currentStorage = pPlayer->m_Storages[storageIdx];
      protoStorage->set_name(currentStorage.Name.c_str());
      for(int storageItemIdx = 0; storageItemIdx < currentStorage.Items.size(); ++storageItemIdx) /// @todo replace with range based for
      {
        auto protoItem = protoStorage->add_items();
        itemHelper.PopulateProtoItem(genericitem_full_t(currentStorage.Items[storageItemIdx]), protoItem);
      }
    }

    // companions
    for (int companionIdx = 0; companionIdx < pPlayer->m_Companions.size(); ++companionIdx) /// @todo replace with range based for
    {
      companion_t &Companion = pPlayer->m_Companions[companionIdx];

      //Save any variables that start with "companion.save."
      CBaseEntity *pEntity = Companion.Entity.Entity();
      if (!pEntity) continue;

      IScripted *pScripted = pEntity->GetScripted();

      if (!pScripted || pScripted->m_Scripts.size() == 0) continue;

      pScripted->CallScriptEvent("game_companion_save");

      CScript *Script = pScripted->m_Scripts[0]; /// @todo why is the first script the right one?

      std::unordered_map<std::string, std::string> saveVarMap {};
      for (int variableIdx = 0; variableIdx < Script->m_Variables.size(); ++variableIdx) /// @todo replace with range based for
      {
        if (Script->m_Variables[variableIdx].Name.starts_with("companion.save."))
        {
          saveVarMap.insert_or_assign(Script->m_Variables[variableIdx].Name.c_str(), Script->m_Variables[variableIdx].Value.c_str());
        }
      }
      if(!saveVarMap.empty())
      {
        auto protoCompanion = out->add_companions();
        protoCompanion->set_scriptname(Companion.ScriptName);
        auto protoMap = protoCompanion->mutable_scriptedsavevariables();

        for(const auto &[tmpKey, tmpValue] : saveVarMap)
        {
          (*protoMap)[tmpKey] = tmpValue;
        }
      }
    }

    // viewed help tips
    for(int helpIdx = 0; helpIdx < pPlayer->m_ViewedHelpTips.size(); ++helpIdx) /// @todo replace with range based for
    {
      out->add_viewedhelptips(pPlayer->m_ViewedHelpTips[helpIdx]);
    }

    // quests
    std::unordered_map<std::string, std::string> questData;
    for(int questIdx = 0; questIdx < pPlayer->m_Quests.size(); ++questIdx) /// @todo replace with range based for
    {
      questData.insert_or_assign(pPlayer->m_Quests[questIdx].Name.c_str(), pPlayer->m_Quests[questIdx].Data.c_str());
    }
    if(!questData.empty())
    {
      auto protoQuestMap = out->mutable_quests();

      for(const auto &[tmpKey, tmpValue] : questData)
      {
        (*protoQuestMap)[tmpKey] = tmpValue;
      }
    }

    // quickslots
    for (int quickSlotIdx = 0; quickSlotIdx < MAX_QUICKSLOTS; quickSlotIdx++)
    {
      const auto &quickSlot = pPlayer->m_QuickSlots[quickSlotIdx];
      auto protoQuickSlot = out->add_quickslots();
      if (quickSlot.Active)
      {
        protoQuickSlot->set_type(GetProtoQuickSlotId(quickSlot.Type));
        protoQuickSlot->set_id(quickSlot.ID);
      }
      else
      {
        protoQuickSlot->set_type(MSRProto::CharData::QuickSlot::QS_EMPTY);
      }
    }
  }

  bool ProtoSave::LoadCharProtobuf(void *pData, unsigned long Size, chardata_t *CharData)
  {
    MS_INFO("Loading protobuf character");


    const std::string protoDataByteString(static_cast<char*>(pData), Size);
    MS_INFO(std::format("Loaded protobuf data size: {}, checksum: {}", protoDataByteString.size(), std::hash<std::string>{}(protoDataByteString)).c_str());

    MSRProto::CharData protoData;
    if (!protoData.ParseFromString(protoDataByteString)) {
      MS_ERROR("Protobuf failed to deserialize character");
      return false;
    }

    const auto &protoSaveData = protoData.savedata();
    CharData->Version = protoSaveData.version();
    std::strncpy(CharData->Name, protoSaveData.name().c_str(), sizeof(CharData->Name));
    std::strncpy(CharData->SteamID, protoSaveData.steamid().c_str(), sizeof(CharData->SteamID));
    std::strncpy(CharData->Race, protoSaveData.race().c_str(), sizeof(CharData->Race));
    CharData->Gender = GetGameGender(protoSaveData.gender());
    std::strncpy(CharData->MapName, protoSaveData.mapname().c_str(), sizeof(CharData->MapName));
    std::strncpy(CharData->NextMap, protoSaveData.nextmap().c_str(), sizeof(CharData->NextMap));
    std::strncpy(CharData->OldTrans, protoSaveData.oldtransition().c_str(), sizeof(CharData->OldTrans));
    std::strncpy(CharData->NewTrans, protoSaveData.newtransition().c_str(), sizeof(CharData->NewTrans));
    // CharData->Origin = protoSaveData.origin();
    const auto &protoAngles = protoSaveData.angles();
    CharData->Angles.x = protoAngles.x();
    CharData->Angles.y = protoAngles.y();
    CharData->Angles.z = protoAngles.z();
    // CharData->Angles = protoSaveData.angles();
    const auto &protoOrigin = protoSaveData.origin();
    CharData->Origin.x = protoOrigin.x();
    CharData->Origin.y = protoOrigin.y();
    CharData->Origin.z = protoOrigin.z();
    CharData->IsElite = protoSaveData.iselite();
    CharData->Gold = protoSaveData.gold();
    CharData->MaxHP = protoSaveData.maxhp();
    CharData->MaxMP = protoSaveData.maxmp();
    CharData->HP = protoSaveData.currenthp();
    CharData->MP = protoSaveData.currentmp();
    std::strncpy(CharData->Party, protoSaveData.partyname().c_str(), sizeof(CharData->Party));
    CharData->PartyID = protoSaveData.partyid();
    CharData->PlayerKills = protoSaveData.playerkills();
    CharData->TimeWaitedToForgetKill = protoSaveData.timewaitedtoforgetkill();
    CharData->TimeWaitedToForgetSteal = protoSaveData.timewaitedtoforgetsteal();


    // ReadMaps1(DataID, m_File); equivalent
    CharData->m_VisitedMaps.clear();
    for (const auto &mapName : protoData.visitedmaps())
    {
      CharData->m_VisitedMaps.add(mapName.c_str());
    }

    // ReadSkills1(DataID, m_File); equivalent
    CStat::InitStatList(CharData->m_Stats);

    CStat* pStat = NULL;
    CSubStat* pSubStat = NULL;

    for (const auto tmpStat : protoData.statlist())
    {
      pStat = CharData->GetStat(GetGameStatId(tmpStat.statid()));

      for (const auto tmpSubStat : tmpStat.substats())
      {
        pSubStat = (pStat ? pStat->GetSubStat(GetGameSubStatId(tmpSubStat.substatid(), (tmpStat.statid() == MSRProto::Stat::SpellCasting))) : NULL);

        if (pStat && pSubStat)
        {
          pSubStat->Value = tmpSubStat.value();
          pSubStat->Exp = tmpSubStat.exp();

          // check for newly added stats not yet in the save file
          if(tmpStat.statid() != MSRProto::Stat::SpellCasting
             && tmpSubStat.substatid() == MSRProto::Stat::SubStat::Power
             && tmpSubStat.value() == 0)
          {
            pSubStat->Value = 1; // power defaults to 1
          }
        }
      }
    }

    // ReadSpells1(DataID, m_File); equivalent
    for (const auto &spell : protoData.knownspells())
    {
      CharData->m_Spells.add(spell.c_str());
    }

    struct {
      // ReadItem1(...); equivalent
      bool ReadProtoItem(const MSRProto::GenericItemFull *protoItem, genericitem_full_t &outItem){
        clrmem(outItem);

        outItem.ID = protoItem->genericitem().id();
        outItem.Name = protoItem->genericitem().name().c_str();
        const msstringstringhash::iterator iAlias = CGenericItemMgr::mItemAlias.find(outItem.Name);
        if (iAlias != CGenericItemMgr::mItemAlias.end()) outItem.Name = iAlias->second;

        outItem.Properties = protoItem->genericitem().properties();
        if (FBitSet(outItem.Properties, ITEM_PERISHABLE) ||
            FBitSet(outItem.Properties, ITEM_DRINKABLE))
        {
          outItem.Quality = protoItem->genericitem().quality();
          outItem.MaxQuality = protoItem->genericitem().maxquality();
        }

        if (FBitSet(outItem.Properties, ITEM_GROUPABLE))
        {
          outItem.Quantity = protoItem->genericitem().quantity();
        }

        outItem.Location = protoItem->location();
        outItem.Hand = GetGameHandId(protoItem->hand());
        if (FBitSet(outItem.Properties, ITEM_CONTAINER))
        {
          genericitem_full_t PackItem;
          for (const auto &protoContainerItem : protoItem->containeritems())
          {
            bool Success = ReadProtoItem(&protoContainerItem, PackItem);
            if (!Success)
            {
              MS_ERROR("Protobuf Bad item in container, skipping...");
              continue;
            }
            outItem.ContainerItems.add(PackItem);
          }
        }

        return true;
      }
    } protoItemizer;

    // ReadItems1(DataID, m_File); equivalent
    for(const auto &protoItem : protoData.items())
    {
      genericitem_full_t Item;
      if (!protoItemizer.ReadProtoItem(&protoItem, Item)) continue;
      CharData->m_Items.add(Item);
    }

    // ReadStorageItems1(DataID, m_File); equivalent
    for(const auto &protoStorage : protoData.storages())
    {
      storage_t Storage;
      Storage.Name = protoStorage.name().c_str();

      for(const auto &protoStorageItem : protoStorage.items())
      {
        genericitem_full_t Item;
        if (!protoItemizer.ReadProtoItem(&protoStorageItem, Item)) continue;
        Storage.Items.add(Item);
      }
      CharData->m_Storages.add(Storage);
    }

    // ReadCompanions1(DataID, m_File); equivalent
    for(const auto &protoCompanion : protoData.companions())
    {
      companion_t &Companion = CharData->m_Companions.add(companion_t());
      Companion.ScriptName = protoCompanion.scriptname().c_str();
      Companion.Active = false;

      for(const auto &[protoKey, protoValue] : protoCompanion.scriptedsavevariables())
      {
        Companion.SaveVarName.add(protoKey.c_str());
        Companion.SaveVarValue.add(protoValue.c_str());
      }
    }

    // ReadHelpTips1(DataID, m_File);
    CharData->m_ViewedHelpTips.clear();
    for(const auto &protoHelpTip : protoData.viewedhelptips())
    {
      CharData->m_ViewedHelpTips.add(protoHelpTip.c_str());
    }

    // ReadQuests1(DataID, m_File); equivalent
    CharData->m_Quests.clear();
    for(const auto &[protoQuestkey, protoQuestValue] : protoData.quests())
    {
      CharData->m_Quests.add(quest_t{.Name=protoQuestkey.c_str(), .Data=protoQuestValue.c_str()});
    }

    // ReadQuickSlots1(DataID, m_File);
    CharData->m_QuickSlots.clear();
    for(const auto &protoQuickslot : protoData.quickslots())
    {
      quickslot_t QuickSlot;
      QuickSlot.Active = true;
      QuickSlot.Type = static_cast<quickslottype_e>(GetGameQuickSlotId(protoQuickslot.type(), QuickSlot.Active));
      if(QuickSlot.Active)
      {
        QuickSlot.ID = protoQuickslot.id();
      }
    }

    return true;
  }

  void ProtoSave::SaveCharJSON(CBasePlayer *pPlayer, savedata_t *pData)
  {
    MS_INFO("Protobuf exporting save to json");
    std::string jsonSaveName {GetSaveFileName(pPlayer->m_CharacterNum, pPlayer)};
    jsonSaveName += ".json";

    MSRProto::CharData charData;

    ProtoSave::CreateCharData(pPlayer, pData, &charData);

    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;

    std::string jsonResult;
    google::protobuf::util::MessageToJsonString(charData, &jsonResult, options);

    CGameFile jsonFile;
    jsonFile.OpenWrite(jsonSaveName.c_str());
    jsonFile.Write(jsonResult.c_str(), jsonResult.size());
    jsonFile.Close();
  }

  void ProtoSave::SaveCharProtobuf(CBasePlayer *pPlayer, savedata_t *pData)
  {
    MSRProto::CharData charData;

    ProtoSave::CreateCharData(pPlayer, pData, &charData);
    std::string charDataByteString;
    if(!charData.SerializeToString(&charDataByteString))
    {
      MS_ERROR("Protobuf failed to serialize string");
      return;
    }
    MS_INFO(std::format("Serialized protobuf data size: {}, checksum: {}", charDataByteString.size(), std::hash<std::string>{}(charDataByteString)).c_str());

    if (FNShared::IsEnabled())
    {
      // If Central Server is enabled, save to the Central Server instead of locally
      FNShared::CreateOrUpdateCharacter(pPlayer, pPlayer->m_CharacterNum, charDataByteString.c_str(), charDataByteString.size(), (pData == NULL));
      return;
    }
    else if (!MSGlobals::ServerSideChar)
    {
      charinfo_t &CharInfo = pPlayer->m_CharInfo[pPlayer->m_CharacterNum];
      CharInfo.AssignChar(pPlayer->m_CharacterNum, LOC_CLIENT, charDataByteString.c_str(), charDataByteString.size(), pPlayer);
      return;
    }

    CGameFile saveFile;
    saveFile.OpenWrite(GetSaveFileName(pPlayer->m_CharacterNum, pPlayer));
    saveFile.Write(charDataByteString.c_str(), charDataByteString.size());
    saveFile.Close();
  }
} // namespace msr
