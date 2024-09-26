#include "msdllheaders.h"

#include "script.h"
#include "weapons/genericitem.h"
#include "store.h"
#include "stats/statdefs.h"
#include "stats/stats.h"
#include "stats/races.h"
#include "msitemdefs.h"
#include "logger.h"

#define ERROR_MISSING_PARMS MSErrorConsoleText("CGenericItem::ExecuteScriptCmd", UTIL_VarArgs("Script: %s, %s - not enough parameters!\n", Script->m.ScriptFile.c_str(), Cmd.Name().c_str()))

scriptcmdname_list CMSMonster::m_ScriptCommands;

//Register my script commands
void CMSMonster::Script_Setup()
{
	if (!m_ScriptCommands.size())
	{
		//NPC Commands
		m_ScriptCommands.add(scriptcmdname_t("blind"));		//Thothie MAR2008b
		m_ScriptCommands.add(scriptcmdname_t("invisible")); //Thothie MAR2008b
		m_ScriptCommands.add(scriptcmdname_t("say"));
		m_ScriptCommands.add(scriptcmdname_t("clearfx")); //Thothie JAN2008a
		m_ScriptCommands.add(scriptcmdname_t("hp"));
		m_ScriptCommands.add(scriptcmdname_t("width"));
		m_ScriptCommands.add(scriptcmdname_t("height"));
		m_ScriptCommands.add(scriptcmdname_t("stepsize"));
		m_ScriptCommands.add(scriptcmdname_t("race"));
		m_ScriptCommands.add(scriptcmdname_t("gold"));
		m_ScriptCommands.add(scriptcmdname_t("addgold")); //Thothie - JUN2007a
		m_ScriptCommands.add(scriptcmdname_t("fov"));
		m_ScriptCommands.add(scriptcmdname_t("hearingsensitivity"));
		m_ScriptCommands.add(scriptcmdname_t("dmgmulti")); //Thothie - SEP2007a
		//m_ScriptCommands.add( scriptcmdname_t( "hitmulti" ) ); //Thothie - FEB2009_18 - moved to main script.cpp
		m_ScriptCommands.add(scriptcmdname_t("nopush")); //Thothie - MAR2008b - new method for stability pot
		m_ScriptCommands.add(scriptcmdname_t("float"));
		m_ScriptCommands.add(scriptcmdname_t("fly"));
		m_ScriptCommands.add(scriptcmdname_t("setanim.movespeed"));
		m_ScriptCommands.add(scriptcmdname_t("movespeed"));
		m_ScriptCommands.add(scriptcmdname_t("maxslope"));
		m_ScriptCommands.add(scriptcmdname_t("invincible"));
		m_ScriptCommands.add(scriptcmdname_t("gaitframerate"));
		m_ScriptCommands.add(scriptcmdname_t("roam"));
		m_ScriptCommands.add(scriptcmdname_t("roamdelay"));
		m_ScriptCommands.add(scriptcmdname_t("skilllevel"));
		m_ScriptCommands.add(scriptcmdname_t("giveitem"));
		m_ScriptCommands.add(scriptcmdname_t("removeitem"));
		m_ScriptCommands.add(scriptcmdname_t("offer"));
		m_ScriptCommands.add(scriptcmdname_t("catchspeech"));
		m_ScriptCommands.add(scriptcmdname_t("saytext"));
		m_ScriptCommands.add(scriptcmdname_t("saytextrange"));
		m_ScriptCommands.add(scriptcmdname_t("setstat"));
		//m_ScriptCommands.add( scriptcmdname_t( "setskin" ) );
		m_ScriptCommands.add(scriptcmdname_t("setidleanim"));
		m_ScriptCommands.add(scriptcmdname_t("setmoveanim"));
		//m_ScriptCommands.add( scriptcmdname_t( "setactionanim" ) );
		m_ScriptCommands.add(scriptcmdname_t("setmovedest"));
		m_ScriptCommands.add(scriptcmdname_t("createstore"));
		m_ScriptCommands.add(scriptcmdname_t("addstoreitem"));
		m_ScriptCommands.add(scriptcmdname_t("offerstore"));
		m_ScriptCommands.add(scriptcmdname_t("npcstore.create"));
		m_ScriptCommands.add(scriptcmdname_t("npcstore.additem"));
		m_ScriptCommands.add(scriptcmdname_t("npcstore.offer"));
		m_ScriptCommands.add(scriptcmdname_t("npcstore.remove"));
		m_ScriptCommands.add(scriptcmdname_t("recvoffer"));
		m_ScriptCommands.add(scriptcmdname_t("dodamage"));
		m_ScriptCommands.add(scriptcmdname_t("tossprojectile"));
		m_ScriptCommands.add(scriptcmdname_t("cancelattack"));
		m_ScriptCommands.add(scriptcmdname_t("look"));
		m_ScriptCommands.add(scriptcmdname_t("takedmg"));
		m_ScriptCommands.add(scriptcmdname_t("storage"));
		m_ScriptCommands.add(scriptcmdname_t("blood"));
		m_ScriptCommands.add(scriptcmdname_t("stopmoving"));
		m_ScriptCommands.add(scriptcmdname_t("setanimtorso"));
		m_ScriptCommands.add(scriptcmdname_t("setanimlegs"));
		m_ScriptCommands.add(scriptcmdname_t("setturnrate"));
		m_ScriptCommands.add(scriptcmdname_t("setstatus"));
		m_ScriptCommands.add(scriptcmdname_t("setanim.frame"));
		m_ScriptCommands.add(scriptcmdname_t("setanim.framerate"));
		m_ScriptCommands.add(scriptcmdname_t("setmonsterclip"));
		m_ScriptCommands.add(scriptcmdname_t("menuitem.register"));
		m_ScriptCommands.add(scriptcmdname_t("menuitem.remove"));
		m_ScriptCommands.add(scriptcmdname_t("menu.open"));
		m_ScriptCommands.add(scriptcmdname_t("menu.autoopen"));
		m_ScriptCommands.add(scriptcmdname_t("expadj"));   //NOV2014_21 - trying to make xp adjustment more managable
		m_ScriptCommands.add(scriptcmdname_t("playanim")); //DEC2014_01 - Mib Said Add This to fix Hashes bug

		//NPC Conditions
		m_ScriptCommands.add(scriptcmdname_t("see", true));
	}

	m_pScriptCommands = &m_ScriptCommands;
}

bool CMSMonster::Script_ExecuteCmd(CScript *Script, SCRIPT_EVENT &Event, scriptcmd_t &Cmd, msstringlist &Params)
{
#ifdef VALVE_DLL

	// Executes a single script event
	startdbg;

	msstring sTemp;

	dbg(msstring("Command: ") + Cmd.Name());

	//************************** SAY **************************
	if (Cmd.Name() == "say")
	{ //say <sound>[mouth open length]
		if (Params.size() >= 1)
		{
			for (int i = 0; i < Params.size(); i++)
			{
				float fDuration = 0.2;
				msstring &FullWord = Params[i];
				msstring UseWord;

				int lengthpos = FullWord.find("[");
				if (lengthpos != msstring_error)
				{
					if (lengthpos)
						UseWord = FullWord.substr(0, lengthpos);
					fDuration = atof(FullWord.substr(lengthpos + 1));
				}
				msstring SayText;
				if (UseWord.len() && UseWord != "*" && !UseWord.starts_with("RND")) //Thothie's totally frustrated and just hacking now - this SHOULD stop it from converting base_chat random mouth movement to waves
				{
					//Thothie JUN2007b allow using vars for say lengths
					//- without winding up with loading VAR_NAME.wav
					//- also be nice to be able to use waves from any folder
					SayText = "npc/";
					SayText += UseWord;
					SayText += ".wav";
				}
				//ALERT( at_console,"saytext %s",SayText.c_str());
				Say(SayText, fDuration);
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** CLEARFX ********************
	//MiB Dec2007a - For removing applyeffects on death
	//- JAN2008a moved to script side as suspect of causing un-undead monsters
	else if (Cmd.Name() == "clearfx")
	{
		if (m_Scripts.size() > 0)
		{
			//old way doesn't work, trying round about way
			CBaseEntity *pTarget = RetrieveEntity(ENT_ME);
			IScripted *pScripted = pTarget->GetScripted(); // UScripted? IScripted.

			for (int i = 0; i < pScripted->m_Scripts.size(); i++) // Check each
			{
				if (pScripted->m_Scripts[i]->VarExists("game.effect.id")) //This is an effect
				{
					if (strcmp(pScripted->m_Scripts[i]->GetVar("game.effect.removeondeath"), "1") == 0 || !pScripted->m_Scripts[i]->VarExists("EFFECT_IGNORE_CLEARFX")) //If the effect is SUPPOSED to be removed
					{
						pScripted->m_Scripts[i]->RunScriptEventByName("effect_die"); //Call this effect's die function
						pScripted->m_Scripts[i]->m.RemoveNextFrame = true;
					}
				}
			}

			//JAN2008a - prevent un-undead monsters
			/*
			IScripted *pScripted = GetScripted(); // UScripted? IScripted.
			if ( pScripted )
			{
				foreach( i, pScripted->m_Scripts.size() ) // Check each 
					if( pScripted->m_Scripts[i]->VarExists("game.effect.id") ) //This is an effect 
						if( strcmp( pScripted->m_Scripts[i]->GetVar("game.effect.removeondeath"), "1" ) == 0) //If the effect is SUPPOSED to be removed 
						{
							pScripted->m_Scripts[i]->RunScriptEventByName( "effect_die" ); //Call this effect's die function
							pScripted->m_Scripts[i]->m.RemoveNextFrame = true;
						}
			}
			*/
		}
	}
	//************************** HP **************************
	//MAY2008a - make current and max HP adjustable independantly if second param is present
	//hp <current> <max>
	else if (Cmd.Name() == "hp")
	{
		if (Params.size() == 1)
			m_HP = pev->health = pev->max_health = m_MaxHP = atof(Params[0]);
		if (Params.size() >= 2)
		{
			m_HP = pev->health = atof(Params[0]);
			pev->max_health = m_MaxHP = atof(Params[1]);
		}
		if (Params.size() < 1)
			ERROR_MISSING_PARMS;
	}
	//************************** WIDTH **************************
	else if (Cmd.Name() == "width")
	{
		if (Params.size() >= 1)
			m_Width = atof(Params[0]);

		else
			ERROR_MISSING_PARMS;
	}
	//************************* HEIGHT **************************
	else if (Cmd.Name() == "height")
	{
		if (Params.size() >= 1)
			m_Height = atof(Params[0]);

		else
			ERROR_MISSING_PARMS;
	}
	//************************* STEPSIZE ************************
	else if (Cmd.Name() == "stepsize")
	{
		if (Params.size() >= 1)
			m_StepSize = atof(Params[0]);

		else
			ERROR_MISSING_PARMS;
	}
	//************************** RACE **************************
	else if (Cmd.Name() == "race")
	{
		if (Params.size() >= 1)
			 strncpy(m_Race,  Params[0], sizeof(m_Race) );

		else
			ERROR_MISSING_PARMS;
	}
	//************************** GOLD **************************
	else if (Cmd.Name() == "gold")
	{
		if (Params.size() >= 1)
			m_Gold = atoi(Params[0]);

		else
			ERROR_MISSING_PARMS;
	}
	//************************** ADDGOLD **************************
	//Thothie - JUN2007a, self explanitory (mainly cuz $get(ent_me,gold) wont work, but this saves a step anyways
	else if (Cmd.Name() == "addgold")
	{
		if (Params.size() >= 1)
		{
			int iGoldAmount = atoi(Params[0]);
			GiveGold(iGoldAmount);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** BLOOD *************************
	else if (Cmd.Name() == "blood")
	{
		if (Params.size() >= 1)
		{
			if (Params[0] == "red")
				m_bloodColor = BLOOD_COLOR_RED;
			else if (Params[0] == "green")
				m_bloodColor = BLOOD_COLOR_GREEN;
			else if (Params[0] == "none")
				m_bloodColor = DONT_BLEED;
			else
				MSErrorConsoleText("CGenericItem::ExecuteScriptCmd", UTIL_VarArgs("Script: %s, %s - invalid blood color!\n", Script->m.ScriptFile.c_str(), Cmd.Name().c_str()));
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** FOV ***************************
	else if (Cmd.Name() == "fov")
	{
		if (Params.size() >= 1)
			m_flFieldOfView = cosf((atof(Params[0]) / 2.0) * (M_PI / 180));

		else
			ERROR_MISSING_PARMS;
	}
	//************************* HEARINGSENSITIVITY *************
	else if (Cmd.Name() == "hearingsensitivity")
	{
		if (Params.size() >= 1)
			m_HearingSensitivity = atof(Params[0]);

		else
			ERROR_MISSING_PARMS;
	}
	//************************* DMGMULTI *************
	//Thothie SEP2007a
	//- attempt to handle dmg multipliers internally
	else if (Cmd.Name() == "dmgmulti")
	{
		if (Params.size() >= 1)
		{
			m_DMGMulti = atof(Params[0]);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//commented- needs to be main script.cpp
	//************************* HITMULTI *************
	//Thothie FEB2009_18
	//- use this to apply hit-damage penalties
	/*
	else if( Cmd.Name() == "hitmulti" )
	{
		if( Params.size() >= 1 )
		{
			m_HITMulti = atof(Params[0]);
		}
		else ERROR_MISSING_PARMS;
	}
	*/
	//************************* NOPUSH *************
	//Thothie SEP2007a
	//- nopush <0|1> - allow/disallow setvelocity/addvelocity effects from others
	else if (Cmd.Name() == "nopush")
	{
		if (Params.size() >= 1)
		{
			SetScriptVar("IMMUNE_PUSH", Params[0]);
			if (atoi(Params[0]))
				m_nopush = true;
			else
				m_nopush = false;
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** ITEMMODELBODYOFS **************
	/*else if( Cmd.Name() == "itemmodelbodyofs" ) {
		while( GetString( (char*)parm, saEvent->Action, i, "\r\n" )) {
			i += strlen(parm) + 1;
			ItemModelBodyOffset = atoi(SCRIPTVAR(parm));
			if( saEvent->Action[i-1] == '\n' ) break;
		}
	}*/
	//************************** ROAM **************************
	else if (Cmd.Name() == "roam")
	{
		if (Params.size() >= 1)
		{
			if (atoi(Params[0]))
				SetConditions(MONSTER_ROAM);
			else
				ClearConditions(MONSTER_ROAM);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** BLIND **************************
	//MAR2008b
	else if (Cmd.Name() == "blind")
	{
		if (Params.size() >= 1)
		{
			//if( atoi(Params[0]) ) SetBits( pev->flags, 16 );
			//else ClearBits( pev->flags, 16 );
			if (atoi(Params[0]))
				SetConditions(MONSTER_BLIND);
			else
				ClearConditions(MONSTER_BLIND);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** INVISIBLE **************************
	//MAR2008b
	else if (Cmd.Name() == "invisible")
	{
		if (Params.size() >= 1)
		{
			//wrong approach here, perhaps
			if (atoi(Params[0]))
				SetBits(pev->flags, 16);
			else
				ClearBits(pev->flags, 16);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** ROAMDELAY **************************
	else if (Cmd.Name() == "roamdelay")
	{
		if (Params.size() >= 1)
			m_RoamDelay = atof(Params[0]);

		else
			ERROR_MISSING_PARMS;
	}
	//************************** SKILLLEVEL ********************
	else if (Cmd.Name() == "skilllevel")
	{
		if (Params.size() >= 1)
		{
			m_SkillLevel = atoi(Params[0]);
			//MIB JUN2010_15 - reset damage tracking on skill reset

			//Thothie NOV2014_21 - storing first use [begin]
			float oldxp = atof(GetFirstScriptVar("NPC_ORIG_EXP"));
			if (oldxp == 0)
			{
				GetScripted()->SetScriptVar("NPC_ORIG_EXP", Params[0]); //iffy
			}
			//[end]

			for (int p = 0; p < m_PlayerDamage.size(); p++)
			{
				// for (int r = 0; r < SKILL_MAX_ATTACK; r++)
				// {
				// 	for (int s = 0; s < STATPROP_ALL_TOTAL; s++)
				// 		m_PlayerDamage[p].dmg[r][s] = 0;
				// }
				// m_PlayerDamage[p].dmgInTotal = 0;
				m_PlayerDamage[p].Clear(); // MiB MAR2019_22 [SLOT_EXP] - Use new clear function
			}
			m_SkillLevel = atoi(Params[0]);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** EXPADJ ********************
	//Thothie NOV2014_21 - XP adjustment
	//expadj <ratio|flat> ["scale"] ["debug-reason"]
	//- If a decimal appears in the first param, it assumes you want to adjust XP by a ratio, otherwise, it adds or subtracts a flat amount.
	//- The scale flag cases the XP ratio to be added or subtracted to by a ratio of the mob's original XP (postive or negative)
	//-- eg. 10XP mob 1.0 scale = 20XP, the first time, 30 the next, similarly after that would be -0.5 = 20XP
	//- Does nothing if skilllevel has not been set.
	//- Executing this erases all player hit data on the mob (meaning executing it mid combat will rob players of XP.)
	//- Intention is to only apply scale ratios until FN+PlayerCount bonus, so system is predictable
	//-- todo: rework on both ends, this system is screwy as fuck
	else if (Cmd.Name() == "expadj")
	{
		if (Params.size() >= 1)
		{
			if (atof(GetFirstScriptVar("NPC_ORIG_EXP")) > 0)
			{
				//first, wipe player hits, otherwise, we'll have problems
				for (int p = 0; p < m_PlayerDamage.size(); p++)
				{
					// for (int r = 0; r < SKILL_MAX_ATTACK; r++)
					// {
					// 	for (int s = 0; s < STATPROP_ALL_TOTAL; s++)
					// 		m_PlayerDamage[p].dmg[r][s] = 0;
					// }
					// m_PlayerDamage[p].dmgInTotal = 0;
					m_PlayerDamage[p].Clear(); // MiB MAR2019_22 [SLOT_EXP] - Use new clear function
				}

				if (Params[0].contains("."))
				{
					float oldxp = atof(GetFirstScriptVar("NPC_ORIG_EXP"));
					if (Params.size() > 1 && Params[1] == "scale")
					{
						m_SkillLevel += (oldxp * atof(Params[0]));
					}
					else
					{
						m_SkillLevel *= atof(Params[0]);
					}
				}
				else
					m_SkillLevel += atof(Params[0]);

				if (m_SkillLevel < 0)
					m_SkillLevel = 0;

				ALERT(at_console, "%s (base %.2f) - ExpAdj: %.2f NewXP: %.2f %s\n", m_DisplayName.c_str(), atof(GetFirstScriptVar("NPC_ORIG_EXP")), atof(Params[0]), m_SkillLevel, (Params.size() >= 3) ? Params[2].c_str() : " ");
			}
			else
				MSErrorConsoleText("CGenericItem::ExecuteScriptCmd", UTIL_VarArgs("Script: %s, %s - initial XP not set, ignoring\n", Script->m.ScriptFile.c_str(), Cmd.Name().c_str()));
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** FLOAT *************************
	else if (Cmd.Name() == "float")
	{
		if (Params.size() >= 1)
		{
			if (atoi(Params[0]))
				SetBits(pev->flags, FL_FLOAT | FL_SWIM);
			else
				ClearBits(pev->flags, FL_FLOAT | FL_SWIM);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************* FLY ****************************
	else if (Cmd.Name() == "fly")
	{
		if (Params.size() >= 1)
		{
			if (atoi(Params[0]))
				SetBits(pev->flags, FL_FLY);
			else
				ClearBits(pev->flags, FL_FLY);

			pev->movetype = FBitSet(pev->flags, FL_FLY) ? MOVETYPE_FLY : MOVETYPE_STEP;
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** SETANIM.MOVESPEED ***********************
	else if (Cmd.Name() == "setanim.movespeed")
	{
		if (Params.size() >= 1)
			m_flGroundSpeed = atof(Params[0]);
		else
			ERROR_MISSING_PARMS;
	}
	//********************** MOVESPEED *************************
	else if (Cmd.Name() == "movespeed")
	{
		if (Params.size() >= 1)
			m_SpeedMultiplier = atof(Params[0]); //NOV2014_19 this was atoi

		else
			ERROR_MISSING_PARMS;
	}
	//********************** MAXSLOPE *************************
	else if (Cmd.Name() == "maxslope")
	{
		if (Params.size() >= 1)
		{
			m_MaxGroundSlope = (90.0f - atof(Params[0])) / 90.0f;
			m_MaxGroundSlope = V_min(m_MaxGroundSlope, 1.0f);
			m_MaxGroundSlope = V_max(m_MaxGroundSlope, 0.0f);
		}

		else
			ERROR_MISSING_PARMS;
	}
	//************************** INVINCIBLE ********************
	else if (Cmd.Name() == "invincible")
	{
		if (Params.size() >= 1)
		{
			byte Type = atoi(Params[0]);
			if (Type)
			{
				SetScriptVar("NPC_INVINCIBLE", 1); //Thothie OCT2015_30 - since we've no reliable way to pull invincibility
				SetBits(pev->flags, FL_GODMODE);
				//if( Type == 1 )	SetConditions( MONSTER_REFLECTIVEDMG ); //AUG2013_21 Thothie - appears to be a hold over from times of yor
			}
			else
			{
				SetScriptVar("NPC_INVINCIBLE", 0); //Thothie OCT2015_30 - since we've no reliable way to pull invincibility
				ClearBits(pev->flags, FL_GODMODE);
				//ClearConditions( MONSTER_REFLECTIVEDMG );
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** SETMONSTERCLIP ********************
	else if (Cmd.Name() == "setmonsterclip")
	{
		if (Params.size() >= 1)
		{
			if (atoi(Params[0]))
				SetBits(pev->flags, FL_MONSTERCLIP);
			else
				ClearBits(pev->flags, FL_MONSTERCLIP);
		}
		else
			ERROR_MISSING_PARMS;
	}

	//******************** GAITFRAMERATE ***********************
	else if (Cmd.Name() == "gaitframerate")
	{
		if (Params.size() >= 1)
			pev->fuser1 = atof(SCRIPTVAR(Params[0]));

		else
			ERROR_MISSING_PARMS;
	}
	//************************** GIVEITEM **********************
	else if (Cmd.Name() == "giveitem")
	{
		if (Params.size() >= 1)
		{
			/*int iGiveFlags = (1<<1);

			msstring_ref Location = Params[0];
			if( !_stricmp(Location,"use") )
				SetBits( iGiveFlags, (1<<0) );
			else if( !_stricmp(Location,"hide") )
				SetBits( iGiveFlags, (1<<1) );
			else if( !_stricmp(Location,"hand") )
				SetBits( iGiveFlags, (1<<2) );

			msstring_ref ItemName = Params[0];*/

			CGenericItem *NewItem = NewGenericItem(Params[0]);
			if (NewItem)
			{
				if (AddItem(NewItem, false, false)) //Don't check weight for monsters
				{
					/*if( iGiveFlags & (1<<0) ) NewItem->UseItem( );  //Use item
					if( iGiveFlags & (1<<1) || !(iGiveFlags & (1<<2)) ) //Hide item
						SetBits( NewItem->pev->effects, EF_NODRAW );
					if( !(iGiveFlags & (1<<2)) ) //Not a hand item (inventory item)
						NewItem->m_Location = ITEMPOS_BACK;*/
					SetBits(NewItem->pev->effects, EF_NODRAW);
				}
				else
					NewItem->Drop();

				if (Params.size() >= 2)
					NewItem->iQuantity = atoi(Params[1]);
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** REMOVEITEM **********************
	//Thothie - this doesn't work
	else if (Cmd.Name() == "removeitem")
	{
		if (Params.size() >= 1)
		{
			CGenericItem *pItem = GetItem(Params[0]);
			if (pItem)
			{
				RemoveItem(pItem);
				pItem->SUB_Remove();
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** OFFER **********************
	else if (Cmd.Name() == "offer")
	{

		if (Params.size() >= 2)
		{
			CBaseEntity *pTarget = RetrieveEntity(Params[0]);
			CBasePlayer *pPlayer = pTarget ? (pTarget->IsPlayer() ? (CBasePlayer *)pTarget : NULL) : NULL;
			bool inventory_full = false;
			if (pPlayer)
			{
				if (pPlayer->NumItems() >= NUM_MAX_ITEMS)
					inventory_full = true;
			}

			if (!inventory_full)
			{
				int GoldAmt = 0;
				itemtype_e ItemType = ITEM_NORMAL;
				if (!_stricmp("gold", Params[1]) && Params.size() >= 2)
				{
					ItemType = ITEM_GOLD;
					GoldAmt = atoi(Params[2]);
				}

				CBaseEntity *pEnt = RetrieveEntity(Params[0]);
				if (pEnt && pEnt->MyMonsterPointer())
				{
					CMSMonster *pMonster = (CMSMonster *)pEnt;
					if (ItemType == ITEM_NORMAL)
					{
						CGenericItem *pItem = NewGenericItem(Params[1]);
						if (pItem)
						{
							//AUG2013_15 Thothie - allow giving of stacks of items (projectiles etc)
							if (pItem->m_MaxGroupable > 1 && Params.size() >= 2)
							{
								pItem->iQuantity = atoi(Params[2]);
							}
							if (!pItem->GiveTo(pMonster))
								pItem->pev->origin = pEnt->pev->origin; //If couldn't give, drop the item on the floor
						}
						else
							ALERT(at_console, "%s - offer: item %s not found!", DisplayName(), Params[1].c_str());
					}
					else if (ItemType == ITEM_GOLD)
						pMonster->GiveGold(GoldAmt);
				}
			}
			else
			{
				pPlayer->SendEventMsg(HUDEVENT_UNABLE, "Cannot recieve items while inventory is full.");
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** CATCHSPEECH ********************
	else if (Cmd.Name() == "catchspeech")
	{
		if (Params.size() >= 2)
		{ //Parameters: <script event> <trigger1> [trigger2] [trigger3] [etc.]
			listenphrase_t newphrase;
			newphrase.ScriptEvent = Params[0];
			for (int i = 0; i < (Params.size() - 1); i++) // Starts at parameter 2
				newphrase.Phrases.add(Params[i + 1]);

			m_Phrases.add(newphrase);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** SAYTEXT ******************************
	else if (Cmd.Name() == "saytext")
	{
		if (Params.size() >= 1)
		{
			for (int i = 0; i < Params.size(); i++)
			{
				if (i)
					sTemp += " ";
				sTemp += Params[i];
			}

			Speak(sTemp, SPEECH_LOCAL);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** SAYTEXT ******************************
	else if (Cmd.Name() == "saytextrange")
	{
		if (Params.size() >= 1)
		{
			if (Params[0] == "default")
				m_SayTextRange = SPEECH_LOCAL_RANGE;
			else
				m_SayTextRange = atof(Params[0]);
		}

		else
			ERROR_MISSING_PARMS;
	}
	//************************** NPCSTORE.CREATE / CREATESTORE **************************
	else if (Cmd.Name() == "npcstore.create" || Cmd.Name() == "createstore")
	{
		if (Params.size() >= 1)
		{
			CStore *NewStore = CStore::GetStoreByName(Params[0]);

			if (!NewStore)
				NewStore = CStore::m_gStores.add(new CStore);

			if (NewStore)
				NewStore->SetName(Params[0]);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** NPCSTORE.ADDITEM / ADDSTOREITEM ***********************
	else if (Cmd.Name() == "npcstore.additem" || Cmd.Name() == "addstoreitem")
	{
		// <storename> <itemname> [quantity] [cost] [sellratio] [bundleamt]
		if (Params.size() >= 2)
		{
			/*
			int iParam = 1, iQuantity = 1, iCost = 100, iBundleAmt = 0;
			float flSellRatio = 0.25;
			msstring_ref StoreName = Params[0];
			msstring_ref ItemName = Params[1];
			if( Params.size() >= 3 ) iQuantity = atoi(Params[2]);
			if( Params.size() >= 4 ) iCost = atoi(Params[3]);		
			if( Params.size() >= 5 ) flSellRatio = atof(Params[4]);		
			if( Params.size() >= 6 ) iBundleAmt = atoi(Params[5]);		
			*/

			//MIB JAN2010_16 - cap resell values
			int iParam = 1, iQuantity = 1, iCost = 100, iBundleAmt = 0;
			float flSellRatio = 0.25;
			msstring_ref StoreName = Params[0];
			msstring_ref ItemName = Params[1];
			if (Params.size() >= 3)
				iQuantity = atoi(Params[2]);
			if (Params.size() >= 4)
				iCost = atoi(Params[3]);
			if (Params.size() >= 5)
				flSellRatio = V_min(atof(Params[4]), .9);
			if (Params.size() >= 6)
				iBundleAmt = atoi(Params[5]);

			msstring StoreDbg = msstring("addstoreitem: ") + StoreName + " " + ItemName + " - ";
			dbg(StoreDbg + (int)100);
			CStore *NewStore = CStore::GetStoreByName(StoreName);

			if (NewStore)
			{
				dbg(StoreDbg + (int)200);
				// MiB MAR2019_13 - Clean up of "temporary items" in favor of global table
				CGenericItem* pItem = CGenericItemMgr::GetGlobalGenericItemByName(ItemName, true);
				if (pItem)
				{
					//Error checking for out-of-bound values
					uint iRealCost = 0;
					if (pItem->m_Value)
					{
						uint MaxPercent = int(0xffffffff / (double)pItem->m_Value);
						//iRealCost = int(pItem->m_Value * min(iCost/100.0,MaxPercent));
						if (iCost / 100.0 < MaxPercent)
							iRealCost = int(pItem->m_Value * (iCost / 100.0));
						else
							iRealCost = int(pItem->m_Value * MaxPercent);
					}

					dbg(StoreDbg + (int)900);
					if (FBitSet(pItem->MSProperties(), ITEM_GROUPABLE) && !iBundleAmt)
						iBundleAmt = pItem->iMaxGroupable;

					// MiB MAR2019_13 - Clean up of "temporary items" in favor of global table (lines commented out as part of effort)
					//dbg( StoreDbg + (int)1000 );
					//pItem->SUB_Remove( ); //MIB MAR2019_12

					dbg(StoreDbg + (int)2000);
					NewStore->AddItem(ItemName, iQuantity, iRealCost, flSellRatio, iBundleAmt);
				}
				else
					MSErrorConsoleText("CMSMonster::Script_ExecuteCmd()", UTIL_VarArgs("Script: %s, %s: non-existant item %s!\n", Script->m.ScriptFile.c_str(), Cmd.Name().c_str(), ItemName));
			}

			//Print( "Stores: %i\n", CStore::m_gStores.size() );
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** NPCSTORE.OFFER / OFFERSTORE **************************
	else if (Cmd.Name() == "npcstore.offer" || Cmd.Name() == "offerstore")
	{
		// <storename> <target> [flags] [callback]
		if (Params.size() >= 3)
		{
			//old style, 3 parameter... target is assumed
			msstring_ref StoreName = Params[0];
			CMSMonster *pDestMonster = NULL;
			msstring_ref BuyFlags = Params[1];
			msstring_ref CallBack = Params[2];

			CBaseEntity *pTarget = RetrieveEntity(Params[1]);
			CBasePlayer *pPlayer = pTarget ? (pTarget->IsPlayer() ? (CBasePlayer *)pTarget : NULL) : NULL;
			bool inventory_full = false;
			if (pPlayer)
			{
				pPlayer->CallScriptEvent("ext_remove_afk"); //flag player as non-afk when first offered store
				if (pPlayer->NumItems() >= NUM_MAX_ITEMS)
				{
					inventory_full = true;
					pPlayer->SendEventMsg(HUDEVENT_UNABLE, "Cannot use stores/chests while inventory full.");
				}
				//pPlayer->InMenu = true;
			}

			if (Params.size() >= 4)
			{
				//new style... 4 parameters... target is specified
				CBaseEntity *pEnt = RetrieveEntity(Params[1]);
				if (pEnt && pEnt->MyMonsterPointer())
					pDestMonster = (CMSMonster *)pEnt;

				BuyFlags = Params[2];
				CallBack = Params[3];
			}

			int iBuyFlags = 0;
			if (strstr(BuyFlags, "buy"))
				SetBits(iBuyFlags, STORE_BUY);
			if (strstr(BuyFlags, "sell"))
				SetBits(iBuyFlags, STORE_SELL);
			if (strstr(BuyFlags, "inv"))
				iBuyFlags = STORE_INV;

			m_TradeCallBackEvent = strlen(CallBack) ? CallBack : 0;

			if (!HasConditions(MONSTER_TRADING))
			{
				CStore *NewStore = CStore::GetStoreByName(StoreName);
				if (pPlayer)
				{
					pPlayer->InMenu = false;
					CLIENT_COMMAND(pPlayer->edict(), "+use;wait;-use\n");
				}
				if (NewStore && !inventory_full)
				{
					OpenStore = NewStore;
					OpenStore->Offer(pDestMonster ? pDestMonster->edict() : pev->euser1, iBuyFlags, this);
					if (m_TradeCallBackEvent.len())
						CallScriptEvent(UTIL_VarArgs("%s_success", m_TradeCallBackEvent.c_str()));
				}
				else if (m_TradeCallBackEvent.len())
					CallScriptEvent(UTIL_VarArgs("%s_fail", m_TradeCallBackEvent.c_str()));
			}
			else if (m_TradeCallBackEvent.len())
				CallScriptEvent(UTIL_VarArgs("%s_busy", m_TradeCallBackEvent.c_str()));
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** NPCSTORE.REMOVE **************************
	else if (Cmd.Name() == "npcstore.remove")
	{
		if (Params.size() >= 1)
		{
			CStore *pStore = CStore::GetStoreByName(Params[0]);

			if (pStore)
			{
				if (Params.size() >= 2)
				{
					if (Params[1] == "allitems")
					{
						pStore->RemoveAllItems();
					}
					else if (Params[1] == "item" && Params.size() >= 3)
					{
						pStore->RemoveItem(Params[2]);
					}
				}
				else
					pStore->Deactivate();
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//******************************* RECVOFFER **************************
	else if (Cmd.Name() == "recvoffer")
	{
		if (Params.size() >= 1)
		{
			if (Params[0] == "accept")
				AcceptOffer();
			else if (Params[0] == "reject" || Params[0] == "decline")
				m_OfferInfo.SrcMonsterIDX = 0;
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** MENUOPTION.REGISTER *********************
	else if (Cmd.Name() == "menuitem.register")
	{
		if (m_MenuCurrentOptions)
		{
			mslist<menuoption_t> &Menuoptions = *m_MenuCurrentOptions;

			menuoption_t MenuOption;
			clrmem(MenuOption);

			MenuOption.ID = SCRIPTVAR("reg.mitem.id");
			MenuOption.Access = MOA_ALL;
			MenuOption.Title = SCRIPTVAR("reg.mitem.title");

			msstring Type = SCRIPTVAR("reg.mitem.type");
			if (Type == "callback")
				MenuOption.Type = MOT_CALLBACK;
			else if (Type == "say")
				MenuOption.Type = MOT_SAY;
			else if (Type == "payment")
				MenuOption.Type = MOT_PAYMENT;
			else if (Type == "payment_silent")
			{
				MenuOption.Type = MOT_PAYMENT;
				MenuOption.SilentPayment = true;
			}
			else if (Type == "disabled")
				MenuOption.Type = MOT_DISABLED;
			else if (Type == "itemdesc")
				MenuOption.Type = MOT_DESC;
			else if (Type == "forgive")
				MenuOption.Type = MOT_FORGIVE;
			else if (Type == "green")
				MenuOption.Type = MOT_GREEN; //Thothie AUG2013_12 - new green type for completed tally
			else
				MenuOption.Type = MOT_CALLBACK;

			MenuOption.Data = SCRIPTVAR("reg.mitem.data");
			MenuOption.CB_Name = Script->VarExists("reg.mitem.callback") ? SCRIPTVAR("reg.mitem.callback") : "";
			MenuOption.CB_Failed_Name = Script->VarExists("reg.mitem.cb_failed") ? SCRIPTVAR("reg.mitem.cb_failed") : "";
			MenuOption.Priority = atoi(SCRIPTVAR("reg.mitem.priority"));

			bool Inserted = false;
			for (int i = 0; i < Menuoptions.size(); i++)
				if (MenuOption.Priority > Menuoptions[i].Priority)
				{
					Menuoptions.add_blank(); //Add blank item to end
											 //Scoot all the items with lower priority down the line to make space for this item
					for (int r = 0; r < (Menuoptions.size() - 1) - i; r++)
					{
						int DstIdx = (Menuoptions.size() - 1) - r;
						int SrcIdx = DstIdx - 1;
						Menuoptions[DstIdx] = Menuoptions[SrcIdx];
					}
					Menuoptions[i] = MenuOption; //Insert this item in the middle
					Inserted = true;
					break;
				}

			if (!Inserted) //If didn't I insert it to middle, add to end
				Menuoptions.add(MenuOption);
		}
	}
	//*************************** MENUOPTION.REMOVE **********************
	else if (Cmd.Name() == "menuitem.remove")
	{
		if (Params.size() >= 1)
		{
			if (m_MenuCurrentOptions)
			{
				mslist<menuoption_t> &Menuoptions = *m_MenuCurrentOptions;
				for (int i = 0; i < Menuoptions.size(); i++)
					if (Params[0] == Menuoptions[i].ID) //Erase _all_ with this name.  Makes erasing big menus easy
						Menuoptions.erase(i--);
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//***************************** MENU.OPEN ****************************
	else if (Cmd.Name() == "menu.open")
	{ //<target player>
		if (Params.size() >= 1)
		{
			CBaseEntity *pTarget = RetrieveEntity(Params[0]);
			CBasePlayer *pPlayer = pTarget ? (pTarget->IsPlayer() ? (CBasePlayer *)pTarget : NULL) : NULL;
			if (pPlayer)
			{
				if (pPlayer->NumItems() < NUM_MAX_ITEMS)
				{
					OpenMenu(pPlayer);
				}
				else
				{
					pPlayer->SendEventMsg(HUDEVENT_UNABLE, "Cannot use menus while inventory is full.");
				}
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//*************************** MENU.AUTOOPEN **************************
	else if (Cmd.Name() == "menu.autoopen")
	{
		m_Menu_Autoopen = atoi(Params[0]) ? true : false;
	}
	//******************************* CANCELATTACK ***********************
	else if (Cmd.Name() == "cancelattack")
	{
		CancelAttack();
	}
	//******************************* LOOK *******************************
	else if (Cmd.Name() == "look")
	{
		m_LookTime = 0;
		Look();
	}
	//******************************* TAKEDMG ****************************
	else if (Cmd.Name() == "takedmg")
	{
		if (Params.size() >= 2)
		{
			float flModifier = atof(Params[1]);

			if (Params[0] == "all")
			{
				m.GenericTDM = flModifier;
				SetScriptVar("MSC_ARMOR_ALL", flModifier);
			}
			else
			{
				takedamagemodifier_t TDM;
				TDM.DamageType = Params[0];
				TDM.modifier = flModifier;

				//Thothie - these stack, trying to undo (MAY2007a)
				bool found_entry = false;
				//char Debug_Type[32];
				//float Debug_AMT = 0.0;
				for (int i = 0; i < m.TakeDamageModifiers.size(); i++)
				{
					takedamagemodifier_t &TDMC = m.TakeDamageModifiers[i];
					if (TDM.DamageType == TDMC.DamageType)
					{
						m.TakeDamageModifiers[i] = TDM;
						found_entry = true;
						//ALERT(at_console,"Entry %s exists, not re-adding",Debug_Type);
					}
					//strcpy(Debug_Type,TDMC.DamageType);
					//Debug_AMT = TDMC.modifier;
					//ALERT(at_console,"Takedmg %s is %f",Debug_Type, Debug_AMT);
				}
				if (!found_entry)
					m.TakeDamageModifiers.add(TDM);
			}
			
			//Thothie DEC2017_13 - game_set_takedmg sets a base for later temporary adjustments
			msstringlist Parameters;
			Parameters.add(Params[0].c_str());
			Parameters.add(UTIL_VarArgs("%f", flModifier));
			if (Params.size() >= 3)
				Parameters.add( Params[2].c_str() ); //'adjust' tells the element adjust system in externals_shared.script not to register as base
			CallScriptEvent("game_set_takedmg", &Parameters);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//******************************** DODAMAGE ***************************
	else if (Cmd.Name() == "dodamage")
	{
		//Thothie: Modding DoDamage to add elemental types
		//Normal: <target> <range> <damage> <hitpercent> [type]
		//Direct: <target> direct <damage> <hitpercent> <source> [type]
		//Radius: <origin> <radius> <damage> <hitpercent> [attenuation] [flags] [type]
		if (Params.size() >= 4)
		{
			damage_t Damage;
			SetBits(Damage.iDamageType, DMG_GENERIC | DMG_SIMPLEBBOX);
			Damage.sDamageType = "generic"; //thothie
			Damage.pAttacker = this;
			if (RetrieveEntity(ENT_EXPOWNER))
				Damage.pAttacker = RetrieveEntity(ENT_EXPOWNER);
			Damage.flAOERange = 0;
			Damage.vecSrc = EyePosition();

			if (Params.size() >= 5) //Thothie
			{
				if (Params[0][0] != '(')
					Damage.sDamageType = Params[4];
			}

			if (Params[1] == "direct")
			{
				SetBits(Damage.iDamageType, DMG_DIRECT);
				Damage.pDirectDmgTarget = RetrieveEntity(Params[0]);

				//Thothie JUN2010_14 - make sure dead players don't do damage (may crash server)
				if (!Damage.pDirectDmgTarget || (Damage.pDirectDmgTarget->IsPlayer() && !Damage.pDirectDmgTarget->IsAlive()))
					return 0;

				if (Params.size() >= 6)
					Damage.sDamageType = Params[5]; //Thothie
			}
			else
			{
				//FEB2010_28 Thothie - Add monster width to range if non-AOE/Direct
				if (Params[0][0] != '(')
				{
					float flMonsterDamageRange = atof(Params[1]);
					CMSMonster *pMonsterMe = IsMSMonster() ? (CMSMonster *)this : NULL;
					CMSMonster *pTarget = IsMSMonster() ? (CMSMonster *)RetrieveEntity(Params[0]) : NULL;
					if (pMonsterMe)
					{
						flMonsterDamageRange += (pMonsterMe->m_Width / 2);
						if (pTarget)
							flMonsterDamageRange += (pTarget->m_Width / 2);
					}
					Damage.flRange = Damage.flDamageRange = flMonsterDamageRange;
				}
				else
					Damage.flRange = Damage.flDamageRange = atof(Params[1]);
			}

			Damage.flDamage = atof(Params[2]);
			//Thothie SEP2007a - make dmg multipliers internal
			if (m_DMGMulti > 0)
				Damage.flDamage *= m_DMGMulti;
			Damage.flHitPercentage = atof(Params[3]);
			//FEB2009_18 - apply to-hit penalties, if any
			if (m_HITMulti > 0)
				Damage.flHitPercentage *= m_HITMulti;
			Damage.flAOEAttn = 1.0f;

			Vector vForward;
			CBaseEntity *pTargetEnt = NULL;
			if (Params[0][0] == '(')
			{
				//Radius damage
				Damage.flAOERange = Damage.flDamageRange;
				Damage.flRange = Damage.flAOERange;
				if (Params.size() >= 5)
					Damage.flAOEAttn = atof(Params[4]);
				if (Params.size() >= 6)
				{
					if (Params[5].contains("reflective"))
						SetBits(Damage.iDamageType, DMG_REFLECTIVE);
				}
				if (Params.size() >= 7)
					Damage.sDamageType = Params[6]; //Thothie
				Damage.vecEnd = Damage.vecSrc;
			}
			else
			{
				pTargetEnt = RetrieveEntity(Params[0]);
				if (Params.size() >= 5)
				{
					CBaseEntity *pAttackerEnt = RetrieveEntity(Params[4]);
					if (pAttackerEnt)
						Damage.pAttacker = pAttackerEnt;
				}
			}

			if (pTargetEnt && (pTargetEnt->Center() - EyePosition()).Length() <= Damage.flDamageRange) //Always contacts (unless a shield is up)
				Damage.vecEnd = pTargetEnt->Center();

			else if (!Damage.flAOERange) //Not directed at an entity or a radius attack, attacks straight forward at anything
			{
				UTIL_MakeVectorsPrivate(pev->v_angle, vForward, NULL, NULL);
				Damage.vecEnd = EyePosition() + vForward * Damage.flDamageRange;
			}

			if (m_UseExpStat)
			{
				Damage.ExpUseProps = true;
				Damage.ExpProp = m_ExpStat;
				Damage.ExpStat = m_ExpProp;
			}

			Damage.pInflictor = this;

			//Thothie JUN2010_14 - make sure dead players don't do damage (may crash server)
			if (Damage.pInflictor->IsPlayer())
			{
				if (!Damage.pInflictor->IsAlive())
					return 0;
			}

			hitent_list Hits;
			DoDamage(Damage, Hits);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** TOSSPROJECTILE ***************************
	else if (Cmd.Name() == "tossprojectile")
	{
		//Parameters: <projectile scriptname> <"view"|(src_origin)> <target|(targ_origin)> <speed> <damage> <cof> <skill|none>
		if (Params.size() >= 7)
		{
			float flRange = atof(Params[3]),
				  flDamage = atof(Params[4]),
				  flAccuracy = atof(Params[5]);

			//Thothie SEP2007a - make dmg multipliers internal
			if (m_DMGMulti > 0)
				flDamage *= m_DMGMulti;

			CBaseEntity *pAttackEnt = NULL;
			bool fAttack = true;
			bool fLocation = false;
			Vector Location;

			if (Params[2][0] == '(')
			{
				Location = StringToVec(Params[2]); //Specified origin to shoot at
				fLocation = true;				   //MIB JUL2010_23 - fix tossing at specific location - Dogg forgot to set this
			}
			else
				pAttackEnt = RetrieveEntity(Params[2]);

			if (fAttack)
			{
				CGenericItem *pProjectile = NewGenericItem(Params[0]);
				if (pProjectile)
				{
					Vector vAngle = pev->v_angle, //Default fire angle
						vForward,
						vStartPos;

					if (Params[1] == "view")
						vStartPos = EyePosition(); //Default start position if not specified
					else
						vStartPos = StringToVec(Params[1]);

					if (fLocation)
					{
						vAngle = UTIL_VecToAngles((Location - vStartPos).Normalize());
						vAngle.x *= -1;
					}
					else if (pAttackEnt)
					{
						vAngle = UTIL_VecToAngles((pAttackEnt->Center() - vStartPos).Normalize());
						vAngle.x *= -1;
					}

					vAngle.x += (flAccuracy * RANDOM_FLOAT(-1, 1));
					vAngle.y += (flAccuracy * RANDOM_FLOAT(-1, 1));

					UTIL_MakeVectorsPrivate(vAngle, vForward, NULL, NULL);

					Vector vTemp = vForward * flRange;

					int Stat = 0, Prop = 0;
					if (IsPlayer())
					{
						GetStatIndices(Params[6], Stat, Prop);
					}

					pProjectile->TossProjectile(this, vStartPos, vTemp, flDamage, Stat, Prop);
				}
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** SETSTAT ******************************
	else if (Cmd.Name() == "setstat")
	{
		if (Params.size() >= 2)
		{
			msstring msInputStatName = Params[0];

			if (!IsPlayer())
			{
				if (msInputStatName == "parry")
				{
					float out_stat = atof(Params[1]);
					SetScriptVar("MONSTER_PARRY", out_stat);
				}
			}

			if (IsPlayer())
			{
				if (!msInputStatName.contains("."))
				{
					CStat *pStat = FindStat(Params[0]);
					if (pStat)
					{
						int NextParm = 1;
						//Thothie NOV2007a - This doesn't work with spellcasting (no changes, just bitching)
						//- will eventually need this to function on spellcasting for apostle quests
						for (int i = 0; i < pStat->m_SubStats.size(); i++)
						{
							if ((signed)Params.size() <= NextParm)
								break; //Keep assigning stats until I run out of stats (for loop) or out of parameters (this check)
							pStat->m_SubStats[i].Value = atoi(Params[NextParm++]);
						}
					}
					else
						MSErrorConsoleText("CMSMonster::Script_ExecuteCmd()", UTIL_VarArgs("Script: %s, %s: stat %s not found!\n", Script->m.ScriptFile.c_str(), Cmd.Name().c_str(), Params[0].c_str()));
				}
				else
				{
					//Thothie DEC2007a - more detailed stat settings (can't set some magic skills under old)
					msstring parse_stat = msInputStatName.thru_char(".");
					CStat *pStat = FindStat(parse_stat);
					if (pStat)
					{
						msstring PropName = msInputStatName.substr(parse_stat.len() + 1);
						int iProp = GetSubSkillByName(PropName);
						if (iProp > -1)
						{
							int value = atoi(Params[1]);
							pStat->m_SubStats[iProp].Value = V_min(value, STATPROP_MAX_VALUE);
						}
					}
				}
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** SETSTATUS ***************************
	else if (Cmd.Name() == "setstatus")
	{
		if (Params.size() >= 2)
		{
			msstring &Action = Params[0];
			msstring &Flag = Params[1];
			int Flags = 0;
			if (Flag.contains("swimming"))
				SetBits(Flags, PLAYER_MOVE_SWIMMING);
			if (Flag.contains("sitting"))
				SetBits(Flags, PLAYER_MOVE_SITTING); //JAN2010_09 - attempting to allow inv while sitting again

			if (Flags)
			{
				if (Action == "add")
					SetBits(m_StatusFlags, Flags);
				else
					ClearBits(m_StatusFlags, Flags);
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** SETSKIN *******************************
	/*else if( Cmd.Name() == "setskin" ) {
		if( Params.size() >= 1 )
		{
			if( Params[0] == "none" ) Skin = 0;
			else Skin = ALLOC_STRING( Params[0] );
		}
		else ERROR_MISSING_PARMS;
	}*/
	//************************** STORAGE *******************************
	else if (Cmd.Name() == "storage")
	{
		if (Params.size() >= 3)
		{
			msstring_ref StorageName = Params[1];
			CBaseEntity *pTarget = RetrieveEntity(Params[2]);
			if (pTarget && pTarget->IsPlayer())
			{
				CBasePlayer *pPlayer = (CBasePlayer *)pTarget;

				//openaccount storagename target cb_prefix
				if (Params[0] == "openaccount")
				{
					msstring Callback = Params.size() >= 4 ? Params[3] : "game_openaccount";

					if (pPlayer->Storage_GetStorage(StorageName))
						CallScriptEvent(Callback + "_exists");
					else
					{
						storage_t Storage;
						Storage.Name = StorageName;

						if (pPlayer->Storage_CreateAccount(Storage))
							CallScriptEvent(Callback + "_success");
						else
							CallScriptEvent(Callback + "_failed");
					}
				}
				//checkaccount storagename target cb_prefix
				else if (Params[0] == "checkaccount")
				{
					msstring Callback = Params.size() >= 4 ? Params[3] : "game_checkaccount";

					if (pPlayer->Storage_GetStorage(StorageName))
						CallScriptEvent(Callback + "_success");
					else
						CallScriptEvent(Callback + "_failed");
				}
				//trade storagename target feeratio displayname cb_prefix
				else if (Params[0] == "trade")
				{
					if (Params.size() >= 5)
					{
						msstring Callback = Params.size() >= 6 ? Params[5] : "game_storage_trade";

						if (pPlayer->Storage_GetStorage(StorageName))
						{
							entityinfo_t newEnt = entityinfo_t(this);
							pPlayer->Storage_Open(Params[4], StorageName, atof(Params[3]), newEnt);
							CallScriptEvent(Callback + "_success");
						}
						else
							CallScriptEvent(Callback + "_noaccount");
					}
					else
						ERROR_MISSING_PARMS;
				}
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** SETIDLEANIM **************************
	else if (Cmd.Name() == "setidleanim")
	{
		if (Params.size() >= 1)
		{
			if (Params[0] == "none")
				m_IdleAnim = "";
			else
				m_IdleAnim = Params[0];
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** SETMOVEANIM **************************
	else if (Cmd.Name() == "setmoveanim")
	{
		if (Params.size() >= 1)
			m_MoveAnim = Params[0];

		else
			ERROR_MISSING_PARMS;
	}
	//************************* SETACTIONANIM *************************
	/*else if( Cmd.Name() == "setactionanim" ) {
		if( Params.size() >= 1 )
			ActionAnim = ALLOC_STRING( Params[0] );

		else ERROR_MISSING_PARMS;
	}*/
	//***************************** PLAYANIM **************************
	else if (Cmd.Name() == "playanim")
	{
		if (Params.size() >= 1)
		{
			msstring_ref pszAnimType = Params[0];
			msstring_ref pszAnimName = NULL;

			if (Params.size() >= 2)
			{
				pszAnimType = Params[0];
				pszAnimName = Params[1];
			}
			//Thothie - JUN2007b
			//- Its impossible to tell what animation a monster is currently using
			//- Since some anim changes are internal
			//- Setting an event here to capture script side.
			//- This isn't dependable as there are too many internal animation changes
			//- MUST figure a way to stop some or all of them, paticularly the break anims
			//- Undone in favor of alternate method (no need to add new events we don't use)
			/*msstringlist Parameters;
			if ( pszAnimName ) Parameters.add( pszAnimName );
			if ( !pszAnimName ) Parameters.add( "break" );
			Parameters.add( pszAnimType );
			CallScriptEvent( "game_anim_new", &Parameters );*/

			MONSTER_ANIM AnimType = MONSTER_ANIM_WALK;
			bool Priority = false;

			if (!_stricmp(pszAnimType, "move"))
				AnimType = MONSTER_ANIM_WALK;
			else if (!_stricmp(pszAnimType, "once"))
				AnimType = MONSTER_ANIM_ONCE;
			else if (!_stricmp(pszAnimType, "hold"))
				AnimType = MONSTER_ANIM_HOLD;
			else if (!_stricmp(pszAnimType, "critical"))
			{
				AnimType = MONSTER_ANIM_ONCE;
				Priority = true;
			}
			else if (!_stricmp(pszAnimType, "break"))
				BreakAnimation(MONSTER_ANIM_BREAK, "", NULL);
			//^^^Thothie JUN2007b - attempting to stop ms.dll from calling break anims
			//- causing AI issues, want to restrict to scripts only

			int Flags = 0;
			if (Params.size() >= 3 && IsPlayer()) //Use gait
			{
				CBasePlayer *pPlayer = (CBasePlayer *)this;
				 strncpy(pPlayer->m_szAnimLegs,  Params[2], sizeof(pPlayer->m_szAnimLegs) );
				if (Params.size() >= 4) //Hold the anim for a set amount of time
					pPlayer->m_TimeResetLegs = gpGlobals->time + atof(Params[3]);
				SetBits(Flags, (1 << 1));
			}
			void *pData = (void *)Flags;

			if (pszAnimName)
			{
				if (Priority)
				{
					BreakAnimation(MONSTER_ANIM_BREAK);
					SetAnimation(AnimType, pszAnimName, pData);
				}
				else
					SetAnimation(AnimType, pszAnimName, pData);
			}
		}
		else
			ERROR_MISSING_PARMS;
	}
	//******************** SETANIMTORSO / SETANIMLEGS ******************
	else if (Cmd.Name() == "setanimtorso" || Cmd.Name() == "setanimlegs")
	{ // Player only
		if (Params.size() >= 1)
		{
			if (IsPlayer())
				strncpy(Cmd.Name() == "setanimtorso" ? ((CBasePlayer *)this)->m_szAnimTorso : ((CBasePlayer *)this)->m_szAnimLegs, Params[0], 32);
		}
		else
			ERROR_MISSING_PARMS;
	}
	//************************** SETTURNRATE ***************************
	else if (Cmd.Name() == "setturnrate")
	{
		if (Params.size() >= 1)
			m_TurnRate = atof(Params[0]);

		else
			ERROR_MISSING_PARMS;
	}
	//**************************** SETANIM.FRAME *************************
	else if (Cmd.Name() == "setanim.frame")
	{
		if (Params.size() >= 1)
			pev->frame = V_max(atof(Params[0]), 0);
		else
			ERROR_MISSING_PARMS;
	}
	//************************** SETANIM.FRAMERATE ***********************
	else if (Cmd.Name() == "setanim.framerate")
	{
		if (Params.size() >= 1)
			m_Framerate = atof(Params[0]);
		else
			ERROR_MISSING_PARMS;
	}

	//************************** STOPMOVING ****************************
	else if (Cmd.Name() == "stopmoving")
	{
		StopWalking();
	}

	//************************** SETMOVEDEST **************************
	else if (Cmd.Name() == "setmovedest")
	{
		//Parameters: "none"
		//Parameters: <target> <stop distance/flee distance> [flags: flee]
		//Parameters: (target) <stop distance/flee distance> [flags: flee]
		if (Params.size() >= 1)
		{
			//Print("CODE: setmovedest.prox %f",atof(Params[1]);
			if (Params[0] == "none")
			{
				StopWalking();
				ClearConditions(MONSTER_HASMOVEDEST);
			}
			else if (Params.size() >= 2)
			{
				bool fMove = false;
				bool fMove_to_entity = false; //AUG2013_22 Thothie - fishies stay in water
				dest_t NewDest;
				clrmem(NewDest);

				if (Params[0][0] == '(')
				{
					NewDest.Origin = StringToVec(Params[0]);
					NewDest.Proximity = atof(Params[1]);
					fMove = true;
				}
				else
				{
					CBaseEntity *pEntity = RetrieveEntity(Params[0]);

					//AUG2013_22 Thothie - fishies stay in water
					if (pEntity && pEntity->pev && pev && (pev->flags & FL_SWIM))
					{
						if (EngineFunc::Shared_PointContents(pEntity->pev->origin) != CONTENTS_WATER)
							pEntity = NULL;
					}

					if (pEntity)
					{
						bool fFlee = false;
						fMove = true;
						if (Params.size() >= 3 && Params[2].contains("flee"))
							fFlee = true;
						float flDistanceParm = atof(Params[1]);

						m_hEnemy = pEntity;
						NewDest.MoveTarget = pEntity;
						fMove_to_entity = true;

						if (!fFlee) //Set dest to target
						{
							if (pEntity->IsMSMonster())
							{
								CMSMonster *pMonster = (CMSMonster *)pEntity;
								float Size = pMonster->IsFlying() ? sqrt(pow(pMonster->m_Width / 2, 2) + pow(pMonster->m_Height / 2, 2)) : pMonster->m_Width / 2;
								Vector vRay = EyePosition() - pEntity->EyePosition();
								//float OriginalDist = vRay.Length( );

								NewDest.Origin = pEntity->EyePosition() + vRay.Normalize() * Size;
							}
							else
								NewDest.Origin = pEntity->EyePosition();
							NewDest.Proximity = flDistanceParm;
						}
						else //Flee
						{
							Vector FleeDir = (Center() - pEntity->Center()).Normalize();
							Vector fleeVec;
							FleeDir.z = 0;

							float StartYaw = UTIL_VecToAngles(FleeDir).y, newYaw;
							bool fFoundVec = false;
							for (int i = 0; i < 20; i++)
							{
								//Try to pick a random flee angle
								newYaw = StartYaw + RANDOM_FLOAT(-90, 90);
								UTIL_MakeVectorsPrivate(Vector(0, newYaw, 0), fleeVec, NULL, NULL);
								TraceResult tr;
								UTIL_TraceLine(Center(), Center() + fleeVec * 200, dont_ignore_monsters, edict(), &tr);
								if (tr.flFraction == 1.0)
								{
									fFoundVec = true;
									break;
								}
							}
							if (!fFoundVec)
							{
								//Couldn't pick a random flee angle, brute force one
								for (int i = 0; i < 359; i++)
								{
									newYaw = StartYaw + i;
									UTIL_MakeVectorsPrivate(Vector(0, newYaw, 0), fleeVec, NULL, NULL);
									TraceResult tr;
									UTIL_TraceLine(Center(), Center() + fleeVec * 128, dont_ignore_monsters, edict(), &tr);
									if (tr.flFraction == 1.0)
									{
										fFoundVec = true;
										break;
									}
								}
							}
							if (fFoundVec)
							{
								NewDest.Origin = Center() + fleeVec * flDistanceParm;
								NewDest.Proximity = GetDefaultMoveProximity();
								m_NodeCancelTime = gpGlobals->time + 5.0;
								m_NextNodeTime = 0;
							}
							else
								fMove = false;
						}
					}
				}

				//AUG2013_22 Thothie - Fishies stay in water
				if (pev->flags & (FL_SWIM) && !fMove_to_entity)
				{
					if (EngineFunc::Shared_PointContents(m_MoveDest.Origin) != CONTENTS_WATER)
					{
						//m_MoveDest.MoveTarget = NULL;
						fMove = false;
						//StopWalking( );
						//ClearConditions( MONSTER_HASMOVEDEST );
					}
				}

				//Move
				if (fMove)
				{
					SetConditions(MONSTER_HASMOVEDEST);
					m_MoveDest = NewDest;
					m_Activity = ACT_WALK;
					m_Wandering = false;

					//Print("CODE_DEBUG: setmovedest %s dist %f", VecToString(NewDest.Origin), NewDest.Proximity );

					SetMoveDest();
				}
			}
			else
				ERROR_MISSING_PARMS;
		}
		else
			ERROR_MISSING_PARMS;
	}
	enddbg;

#endif
	return false;
}
bool CMSMonster::GetScriptVar(msstring &ParserName, msstringlist &Params, CScript *BaseScript, msstring &Return)
{
#ifdef VALVE_DLL
	//msstring FullText = pszText;
	if (ParserName == "$cansee" && Params.size() >= 1)
	{
		msstring &Name = Params[0];
		//If you spot something
		//$cansee(<target/ally/enemy/player/(name)/(classname)>,<range>)
		float ClosestTarget = -1;
		if (Params.size() >= 2)
			ClosestTarget = atof(Params[1]);

		bool fFoundTarget = false;

		CBaseEntity *pSpecificEnt = RetrieveEntity(Name);

		mslist<CBaseEntity *> SightedList;

		if (pSpecificEnt)
			SightedList.add(pSpecificEnt);
		for (int i = 0; i < m_EnemyListNum; i++)
			SightedList.add((CBaseEntity *)m_hEnemyList[i]);

		for (int i = 0; i < SightedList.size(); i++)
		{
			bool fSawTarget = false;

			CBaseEntity *pSighted = SightedList[i];

			if (pSighted == NULL)
				continue;

			//Thothie APR2011_29 - This might work for fishies, but fux up everything else - if we need this behavior, it is available scriptside
			/*
				if( FBitSet(pev->flags,FL_SWIM) &&										//Don't see out of water
					((pev->waterlevel != 3 && pSighted->pev->waterlevel == 3) 
					|| (pev->waterlevel == 3 && pSighted->pev->waterlevel == 0)) )
						continue;
				*/
			//commenting this out does not to seem to have fixed the "blind to things in water" bit

			if (pSighted->pev->deadflag != DEAD_NO)
				continue;

			if (pSpecificEnt)
				fSawTarget = (pSighted == pSpecificEnt);
			else
			{
				if (FStrEq(STRING(pSighted->pev->classname), Name) ||
					FStrEq(STRING(pSighted->pev->netname), Name) ||
					FStrEq(pSighted->DisplayName(), Name))
					fSawTarget = true;

				if (!_stricmp("enemy", Name))
				{
					switch(IRelationship(pSighted))
					{
					case RELATIONSHIP_FR:
					case RELATIONSHIP_DL:
					case RELATIONSHIP_HT:
					case RELATIONSHIP_NM:
						fSawTarget = true;
						break;
					}
				}
				else if (!_stricmp("ally", Name) && IRelationship(pSighted) == RELATIONSHIP_AL)
					fSawTarget = true;
				else if (!_stricmp("wary", Name) && IRelationship(pSighted) == RELATIONSHIP_WA)
					fSawTarget = true;
				else if (!_stricmp("player", Name) && pSighted->IsPlayer())
					fSawTarget = true;
			}

			if (!fSawTarget || !FMVisible(pSighted))
			{
				fSawTarget = false;
				continue;
			}

			//This is always going to use Length(), not Length2D()
			float flDistanceToTarget = (pSighted->Center() - Center()).Length();

			if (pSighted->IsMSMonster())
			{
				//Adjust for the size of the monster
				CMSMonster *pMonster = (CMSMonster *)pSighted;
				float Size = pMonster->IsFlying() ? sqrt(pow(pMonster->m_Width / 2, 2) + pow(pMonster->m_Height / 2, 2)) : pMonster->m_Width / 2;
				flDistanceToTarget -= Size;
			}

			if (ClosestTarget < 0 ||
				(flDistanceToTarget < ClosestTarget))
			{
				StoreEntity(pSighted, ENT_LASTSEEN);
				ClosestTarget = flDistanceToTarget;
				fFoundTarget = true;
			}
		}

		Return = fFoundTarget ? "1" : "0";
		return true;
	}
	//Thothie $get_inarea(<search_type>,<radius>)
	//- hopefully more dependable than $cansee, and less silly than dodamage scans
	//<search_type> can be:
	//player - returns if player in radius
	//enemy - returns if enemy in radius
	//ally - returns if ally in radius
	//This is not working. Gets here, but doesn't return anything
	//Commenting out and cocentrating on $get_insphere in script.cpp, as seems more promissing
	/*
		if( ParserName == "$get_inarea" && Params.size() >= 1 )
		{
			msstring &Name = Params[0];
			float thoth_boxsize = atof(Params[1]);
			float neg_boxsize = thoth_boxsize * -1;
			CBaseEntity *pList[255], *pEnt = NULL;
			ALERT( at_console, "$get_inarea got %f to %f", thoth_boxsize, neg_boxsize );
			int count = UTIL_EntitiesInBox( pList, 255, Vector(neg_boxsize,neg_boxsize,neg_boxsize), Vector(thoth_boxsize,thoth_boxsize,thoth_boxsize), FL_MONSTER|FL_CLIENT );
			 for (int i = 0; i < count; i++) 
			{
				pEnt = pList[i];
				if( !pEnt->pev || FNullEnt(pEnt->edict()) ) continue;

				if( pEnt->edict() == edict() )
				{
					if( (!_stricmp("enemy",Name) && IRelationship(pEnt) < 0) ||
						(!_stricmp("ally",Name) && IRelationship(pEnt) > 0) ||
						(!_stricmp("player",Name) && pEnt->IsPlayer()) )
					{
						Return = EntToString(pEnt);
						return true;
					}
				}
			}
			Return = "none";
			return false;
		}
		*/
#endif

	return false; //IScripted::GetScriptVar( pszText, BaseScript );
}

int CMSMonster::Script_ParseLine(CScript *Script, msstring_ref pszCommandLine, scriptcmd_t &Cmd)
{
	msstring CmdName = msstring(pszCommandLine).thru_char(SKIP_STR);
	if (!_stricmp(CmdName.c_str(), "see"))
	{
		Cmd.m_NewConditional = false;
		return 2;
	}

	return IScripted::Script_ParseLine(Script, pszCommandLine, Cmd);
}

msstring_ref CMSMonster::GetProp(CBaseEntity *pTarget, msstring &FullParams, msstringlist &Params)
{
	if (!pTarget)
		pTarget = this;
	CMSMonster *pMonster = pTarget->IsMSMonster() ? (CMSMonster *)pTarget : NULL;
	bool fSuccess = false;
	static msstring Return;
	msstring &Prop = FullParams;

	if (pMonster && Prop == "walkspeed")
		RETURN_FLOAT(pMonster->WalkSpeed(true))
	else if (pMonster && Prop == "runspeed")
		RETURN_FLOAT(pMonster->RunSpeed(true))
	else if (pMonster && Prop == "movespeed")
		RETURN_FLOAT(pMonster->m_flGroundSpeed) //Thothie MAR2008 - attempting to fix freeze_solid issues
	else if (pMonster && Prop == "framerate")
		RETURN_FLOAT(pMonster->m_Framerate) //Thothie MAR2008 - attempting to fix freeze_solid issues

	return fSuccess ? "1" : CBaseEntity::GetProp(pTarget, FullParams, Params);
}
