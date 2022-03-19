#include "msdllheaders.h"
#include "msitemdefs.h"
#include "Stats/stats.h"
#include "Stats/statdefs.h"
#include "Syntax/syntax.h"
#include "player.h"
#include "Weapons/weapons.h"
#include "Weapons/genericitem.h"
#include "logfile.h"
#include "modeldefs.h"

int CBasePlayer::IdealModel()
{
	return MAKE_STRING(MODEL_HUMAN_REF);
	/*if( m_Gender == GENDER_MALE )
		return MAKE_STRING(MODEL_HUMAN_REF);
	else return MAKE_STRING(MODEL_HUMAN_REF);*/
	//return MAKE_STRING(Race->MaleModel);
	//return MAKE_STRING(Race->FemaleModel);
}
Vector CBasePlayer::Size(int flags)
{
	return Vector(m_Width, m_Width, (!(flags & FL_DUCKING)) ? m_Height : m_Height / 2);
}
void CBasePlayer::SetSize(int flags)
{
	UTIL_SetSize(pev, -(Size(flags) / 2), Size(flags) / 2);
}
void CBasePlayer::SendMenu(int iMenu, TCallbackMenu *cbmMenu)
{
	switch (iMenu)
	{
	case MENU_USERDEFINED:
		if (!cbmMenu)
			return;
		//CurrentCallbackMenu must be dynamic memory
		if (CurrentCallbackMenu && CurrentCallbackMenu != cbmMenu)
		{
			TCallbackMenu *pOldMenu = CurrentCallbackMenu;
			CBaseEntity *pOwner = GetClassPtr((CBaseEntity *)pOldMenu->pevOwner);
			//Let the callback function delete its own memory, then I delete the menu
			if (pOwner && pOldMenu->m_MenuCallback)
				(pOwner->*(pOldMenu->m_MenuCallback))(this, -1, pOldMenu);
			delete pOldMenu;
		}
		CurrentCallbackMenu = cbmMenu;
		ShowMenu(cbmMenu->cMenuText, cbmMenu->iValidslots);
		break;
	}
	CurrentMenu = iMenu;
}
void CBasePlayer::ParseMenu(int iMenu, int slot)
{
	int validselections = 0; //, i;
	switch (iMenu)
	{
	case MENU_USERDEFINED:
		if (!CurrentCallbackMenu || !CurrentCallbackMenu->m_MenuCallback || !CurrentCallbackMenu->pevOwner)
			break;
		CBaseEntity *pOwner = GetClassPtr((CBaseEntity *)CurrentCallbackMenu->pevOwner);
		if (!pOwner)
			break;

		TCallbackMenu *pOldMenu = CurrentCallbackMenu;
		(pOwner->*(pOldMenu->m_MenuCallback))(this, slot, pOldMenu);
		//CurrentCallbackMenu must be dynamic memory
		if (CurrentCallbackMenu == pOldMenu)
			CurrentCallbackMenu = NULL;

		delete pOldMenu;
		break;
	}
}
bool CBasePlayer::LearnSkill(int iStat, int iStatType, int EnemySkillLevel)
{
	bool fSuccess = CMSMonster::LearnSkill(iStat, iStatType, EnemySkillLevel);
	int iStatIdx = iStat - SKILL_FIRSTSKILL;

	//Thoth DEC2008a level cap
	if (GetSkillStat(iStatIdx, iStatType) >= CHAR_LEVEL_CAP)
		return false;

	msstring stat_name = SkillStatList[iStatIdx].Name;
	bool is_spell_stat = stat_name.contains("Spell");
	if (is_spell_stat)
		ALERT(at_console, "Gained XP: %i in skill %s %s \n", EnemySkillLevel, SkillStatList[iStatIdx].Name, SpellTypeList[iStatType]); //Thothie returns XP gained by monsters
	else
		ALERT(at_console, "Gained XP: %i in skill %s %s \n", EnemySkillLevel, SkillStatList[iStatIdx].Name, SkillTypeList[iStatType]); //Thothie returns XP gained by monsters
	if (fSuccess)
	{
		startdbg;
		hudtextparms_t htp;
		memset(&htp, 0, sizeof(hudtextparms_t));
		htp.x = 0.02;
		htp.y = 0.6;
		htp.effect = 2;
		htp.r1 = 0;
		htp.g1 = 128;
		htp.b1 = 0;
		htp.r2 = 178;
		htp.g2 = 119;
		htp.b2 = 0;
		htp.fadeinTime = 0.02;
		htp.fadeoutTime = 3.0;
		htp.holdTime = 2.0;
		htp.fxTime = 0.6;
		dbg("HudMessage");
		UTIL_HudMessage(this, htp, UTIL_VarArgs("%s %s +1\n", SkillStatList[iStatIdx].Name, SkillTypeList[iStatType]));

		if (!is_spell_stat)
		{
			SendInfoMsg("You become more adept at %s.\n", SkillStatList[iStatIdx].Name);
			UTIL_HudMessage(this, htp, UTIL_VarArgs("%s %s +1\n", SkillStatList[iStatIdx].Name, SkillTypeList[iStatType]));
		}
		else
		{
			SendInfoMsg("You become more adept at %s.\n", SkillStatList[iStatIdx]);
			UTIL_HudMessage(this, htp, UTIL_VarArgs("%s %s +1\n", SkillStatList[iStatIdx].Name, SpellTypeList[iStatType]));
		}

		dbg("game_learnskill");
		msstringlist Params;
		Params.add(SkillStatList[iStatIdx].Name);
		if (!is_spell_stat)
			Params.add(SkillTypeList[iStatType]);
		else
			Params.add(SpellTypeList[iStatType]);
		Params.add(UTIL_VarArgs("%i", GetSkillStat(iStatIdx, iStatType)));
		CallScriptEvent("game_learnskill", &Params);
		enddbg;
	}
	return fSuccess;
}
bool CBasePlayer::LearnSkill(int iStat, int EnemySkillLevel)
{
	//Exp is added to a random property of the skill
	CStat *pStat = FindStat(iStat);
	if (!pStat)
		return false;

	int iSubStat = RANDOM_LONG(0, (pStat->m_SubStats.size() - 1));
	//SendInfoMsg( "You gain %d XP", EnemySkillLevel ); //thothie - XP report - no workie

	return LearnSkill(iStat, iSubStat, EnemySkillLevel);
}
