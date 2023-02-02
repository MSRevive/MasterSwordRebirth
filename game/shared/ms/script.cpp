/*
	Master Sword - A Half-life Total Conversion
	Script.cpp - Parses & Executes script event files for NPCs or weapons
*/

#include "inc_weapondefs.h"
#include "script.h"
#include "scriptmgr.h"
#include "groupfile.h"

#ifdef VALVE_DLL
#include "svglobals.h"
#include "global.h"
bool GetModelBounds(CBaseEntity* pEntity, Vector Bounds[2]);
#else
#include "ms/clglobal.h"
#include "hud.h"
#include "cl_util.h"
#include "ms/hudscript.h"
#endif

#ifndef _WIN32
#include <stdlib.h>
#define _gcvt gcvt
#endif

#include "../engine/studio.h"
#include "logger.h"
#include "time.h"
#include "crc/crchash.h" //Wishbone MAR2016 - Our CRC function.
#include "findentities.h"
#include <iterator>
//#include <unordered_map>

#undef SCRIPTVAR
#define VecMultiply( a, b ) Vector( a[0] * b[0], a[1] * b[1], a[2] * b[2] )		//Thothie APR2016_25 - seems we need this here too
#define SCRIPTVAR GetVar								//A script-wide or global variable
#define SCRIPTCONST( a ) SCRIPTVAR(GetConst(a))			//A const, script-wide, or global variable - loadtime only
#define GETCONST_COMPATIBLE( a ) ( a[0] == '$' ? GetConst(a) : SCRIPTCONST(a) )			//Loadtime - Only parse it as a var if it's not a $parser
//int CountPlayers( void );
bool FindSkyHeight(Vector Origin, float& SkyHeight);
bool UnderSky(Vector Origin); //Thothie AUG2010_03
msstring_ref PM_GetValue(msstringlist& Params);
bool GetModelBounds(void* pModel, Vector Bounds[2]);

CScript::msfunchash_t CScript::m_GlobalCmdHash;	// MiB 30NOV_14  Hashed Commands for ScriptCmds.cpp
CScript::msgetterhash_t CScript::m_GlobalGetterHash;
msscripthashhash CScript::mGlobalScriptHashes;
msscriptsethash CScript::mGlobalScriptSets;
::mslist<scriptvar_t> CScript::m_gVariables;				//Global script variables
msscriptarrayhash CScript::GlobalScriptArrays;		    // MiB JUN2010_25
ulong CScript::m_gLastSendID = SCRIPT_ID_START;			//Server: ID of last script sent to a client
//Client: ID of next script to be created
ulong CScript::m_gLastLightID;							//ID of last dynamic light

void CScript::ScriptGetterHash_Setup()
{
	if (m_GlobalGetterHash.empty())
	{
		m_GlobalGetterHash["$len"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Len);
		m_GlobalGetterHash["$lcase"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_LCase);
		m_GlobalGetterHash["$timestamp"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_TimeStamp);
		m_GlobalGetterHash["$get_takedmg"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetTakeDmg);
		m_GlobalGetterHash["$stradd"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_StrAdd);
		m_GlobalGetterHash["$can_damage"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_CanDamage);
		m_GlobalGetterHash["$get_map_legit"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetMapLegit);
		m_GlobalGetterHash["$getcl"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetCl);
		m_GlobalGetterHash["$get"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Get);
		m_GlobalGetterHash["$ucase"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_UCase);
		m_GlobalGetterHash["$get_time"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetTime);
		m_GlobalGetterHash["$veclen2D"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_VecLen);
		m_GlobalGetterHash["$veclen"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_VecLen);
		m_GlobalGetterHash["$filesize"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_FileSize);
		m_GlobalGetterHash["$dist2D"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Dist);
		m_GlobalGetterHash["$within_cone2D"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Cone);
		m_GlobalGetterHash["$within_box"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_WithinBox);
		m_GlobalGetterHash["$anglediff"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_AngleDiff);
		m_GlobalGetterHash["$get_by_name"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetByName);
		m_GlobalGetterHash["$angles3d"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Angles3d);
		m_GlobalGetterHash["$neg"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Neg);
		m_GlobalGetterHash["$get_jointype"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetJoinType);
		m_GlobalGetterHash["$get_by_idx"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Get);
		m_GlobalGetterHash["$get_cvar"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetCVar);
		m_GlobalGetterHash["$get_ground_height"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetGroundHeight);
		m_GlobalGetterHash["$item_exists"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_ItemExists);
		m_GlobalGetterHash["$right"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_StringRightOrLeft);
		m_GlobalGetterHash["$ratio"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetSkillRatio);
		m_GlobalGetterHash["$relvel"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_RelVel);
		m_GlobalGetterHash["$relpos"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_RelPos);
		m_GlobalGetterHash["$angles"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Angles);
		m_GlobalGetterHash["$get_token"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetToken);
		m_GlobalGetterHash["$get_lastmap"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetLastMap);
		m_GlobalGetterHash["$get_tsphere"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetTSphereAndBox);
		m_GlobalGetterHash["$get_isphere"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetTSphereAndBox); //Thothie AUG2016_17 - isphere - returns indexes instead of entities so we can get more bang for our stringspace buck
		m_GlobalGetterHash["$anim_exists"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_AnimExists);
		m_GlobalGetterHash["$string_from"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_StringUpToOrFrom);
		m_GlobalGetterHash["$get_under_sky"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetUnderSky);
		m_GlobalGetterHash["$get_skill_ratio"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetSkillRatio);
		m_GlobalGetterHash["$alphanum"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_AlphaNum);
		m_GlobalGetterHash["$cap_first"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_CapFirst);
		m_GlobalGetterHash["$vec"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Vec);
		m_GlobalGetterHash["$vec.x"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Vec);
		m_GlobalGetterHash["$vec.y"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Vec);
		m_GlobalGetterHash["$vec.z"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Vec);
		m_GlobalGetterHash["$vec.pitch"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Vec);
		m_GlobalGetterHash["$vec.yaw"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Vec);
		m_GlobalGetterHash["$vec.roll"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Vec);
		m_GlobalGetterHash["$cone_dot2D"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Cone);
		m_GlobalGetterHash["$get_contents"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetContents);
		m_GlobalGetterHash["$get_quest_data"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetQuestData);
		m_GlobalGetterHash["$num"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Num);
		m_GlobalGetterHash["$insert"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_ReplaceOrInsert);
		m_GlobalGetterHash["$subst"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_ReplaceOrInsert); //Thothie DEC2017_05 - substitute word string processing
		m_GlobalGetterHash["$min"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_MinMax);
		m_GlobalGetterHash["$get_insphere"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetInSphere);
		m_GlobalGetterHash["$search_string"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_SearchString);
		m_GlobalGetterHash["$string_upto"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_StringUpToOrFrom);
		m_GlobalGetterHash["$func"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Func);
		m_GlobalGetterHash["$get_skillname"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetSkillName);
		m_GlobalGetterHash["$quote"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Quote);
		m_GlobalGetterHash["$int"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Int);
		m_GlobalGetterHash["$get_scriptflag"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetScriptFlag);
		m_GlobalGetterHash["$mid"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Mid);
		m_GlobalGetterHash["$get_tbox"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetTSphereAndBox);
		m_GlobalGetterHash["$get_tbox_abs"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetTSphereAndBox); //Thothie OCT2016_11
		m_GlobalGetterHash["$g_get_array"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetArray);
		m_GlobalGetterHash["$g_get_arrayfind"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetArray);
		m_GlobalGetterHash["$g_get_array_amt"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetArray);
		m_GlobalGetterHash["$dist"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Dist);
		m_GlobalGetterHash["$cone_dot"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Cone);
		m_GlobalGetterHash["$dir"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Dir);
		m_GlobalGetterHash["$replace"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_ReplaceOrInsert);
		m_GlobalGetterHash["$rand"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Rand);
		m_GlobalGetterHash["$randf"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Rand);
		m_GlobalGetterHash["$get_sky_height"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetSkyHeight);
		m_GlobalGetterHash["$get_array"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetArray);
		m_GlobalGetterHash["$get_array_exists"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetArray); //Thothie NOV2016_09 forgot this in port to hash system
		m_GlobalGetterHash["$get_arrayfind"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetArray); //Thothie SEP2019_28 note: Should alias $get_array_find (see also $get_find_token)
		m_GlobalGetterHash["$get_array_amt"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetArray);
		m_GlobalGetterHash["$get_attackprop"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetAttackProp);
		m_GlobalGetterHash["$get_find_token"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetFindToken); //Thothie SEP2019_28 note: Should alias $get_token_find
		m_GlobalGetterHash["$math"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_MathReturn);
		m_GlobalGetterHash["$get_fileline"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetFileLine);
		m_GlobalGetterHash["$get_token_amt"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetTokenAmt);
		m_GlobalGetterHash["$left"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_StringRightOrLeft);
		m_GlobalGetterHash["$get_local_prop"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetCl);
		m_GlobalGetterHash["$sort_entlist"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_SortEntList);
		m_GlobalGetterHash["$float"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Float);
		m_GlobalGetterHash["$pass"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Pass); //Thothie DEC2017_05 - callevent PARAM passthrough
		m_GlobalGetterHash["$get_traceline"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetTraceLine);
		m_GlobalGetterHash["$max"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_MinMax);
		m_GlobalGetterHash["$map_exists"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_MapExists);
		m_GlobalGetterHash["$getcl_tsphere"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetClTSphere);
		m_GlobalGetterHash["$getcl_beam"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_getcl_beam); //Thothie DEC2014_10 - beam_update
		m_GlobalGetterHash["$clcol"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_clcol); //Thothie DEC2014_10 - $clcol - somewhat better client<->server color matching
		m_GlobalGetterHash["$filehash"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_FileHash); //Wishbone MAR2016 - get a file's hash.
		m_GlobalGetterHash["$inrange"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_inrange); //Thothie APR2016_15 $inrange
		m_GlobalGetterHash["$inset_string"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_inrange); //Thothie APR2016_28 $inset
		m_GlobalGetterHash["$inset_num"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_inrange); //Thothie APR2016_28 $inset
		m_GlobalGetterHash["$indiam"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_indiam); //Thothie APR2016_15 $indiam
		m_GlobalGetterHash["$indiam2D"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_indiam); //Thothie APR2016_15 $indiam2D
		m_GlobalGetterHash["$random_of_set"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_random_of_set); //Thothie MAY2016_04 - Woulda saved much effort ages ago
		m_GlobalGetterHash["$get_random_token"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_get_random_token); //Thothie MAY2016_04 - also woulda saved much effort ages ago
		m_GlobalGetterHash["$and"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Conjunction); //MiB FEB2019_23 - Extended If Conditionals
		m_GlobalGetterHash["$or"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Conjunction); //MiB FEB2019_23 - Extended If Conditionals
		m_GlobalGetterHash["$xor"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Conjunction); //MiB FEB2019_23 - Extended If Conditionals
		m_GlobalGetterHash["$get_item_table"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetItemTable); //MiB FEB2019_23 - item table getter
		m_GlobalGetterHash["$scan"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_ScanShape);
		m_GlobalGetterHash["$shape_cylinder"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_ShapeCylinder);
		m_GlobalGetterHash["$shape_rect"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_ShapeRect);
		m_GlobalGetterHash["$shape_sphere"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_ShapeSphere);
		m_GlobalGetterHash["$tokenize"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_tokenize); //Thothie SEP2019_15

		m_GlobalGetterHash["$get_hashmap"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$get_hashmap_haskey"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$get_hashmap_exists"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$get_hashmap_amt"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$get_hashmap_keys"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$get_hashmap_values"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$get_hashmap_pairs"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$g_get_hashmap"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$g_get_hashmap_haskey"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$g_get_hashmap_exists"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$g_get_hashmap_amt"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$g_get_hashmap_keys"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$g_get_hashmap_values"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);
		m_GlobalGetterHash["$g_get_hashmap_pairs"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetHashMap);

		m_GlobalGetterHash["$eval"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_Eval); // Mib SEP2019_18

		m_GlobalGetterHash["$get_set"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetSet); // MiB SEP2019_23
		m_GlobalGetterHash["$get_set_hasvalue"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetSet);
		m_GlobalGetterHash["$get_set_exists"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetSet);
		m_GlobalGetterHash["$get_set_amt"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetSet);
		m_GlobalGetterHash["$g_get_set"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetSet);
		m_GlobalGetterHash["$g_get_set_hasvalue"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetSet);
		m_GlobalGetterHash["$g_get_set_exists"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetSet);
		m_GlobalGetterHash["$g_get_set_amt"] = scriptcpp_cmdfunc_t(&CScript::ScriptGetter_GetSet);
	}
}

bool BreakUpLine(msstring& Line, msstring& outParserName, msstringlist& outParams)
{
	if (Line[0] != '$') //Example: $cansee(ent_lastseen,ATTACK_RANGE)
		return true;	 //Was not a parser, ignore

	//'P' here stands for parenthesis
	size_t StartP = Line.findchar("(");
	if (StartP == msstring_error)
		return false;

	outParserName = Line.substr(0, StartP);

	StartP++;
	int PCount = 1;
	int LastParamEnd = StartP;

	//Use a PCount to keep track of open and close parenthesis and determine the internal parameters
	//PCount starts at 1
	for (int i = StartP; i < (signed)Line.len(); i++)
	{
		char Char = Line[i];
		if (Char == '(') PCount++;						//Increase counter when open P found
		else if (Char == ')') PCount--;				//Decrease counter when close P found

		if ((Char == ',' && PCount == 1) ||				//Found a comma, save parameter
			(Char == ')' && PCount == 0))				//Found the closing P, save parameter
		{
			msstring Temp = Line.substr(LastParamEnd, i - LastParamEnd);
			outParams.add(Temp);

			LastParamEnd = i + 1;
		}

		if (!PCount)										//If this is the closing P, then stop here
			break;
	}

	if (PCount != 0)	//Parenthesis did not close correctly
		return false;

	//Parenthesis closed correctly
	return true;
}

msstring_ref CScript::GetConst(msstring_ref Text)
{
	static msstring ReturnString;
	if (!strcmp(Text, "$currentscript"))
	{
		ReturnString = (msstring_ref)m.ScriptFile;
		ReturnString = ReturnString.thru_substr(SCRIPT_EXT);	//Remove the file extention
		return ReturnString;
	}
	else if (!strcmp(Text, "$currentmapscript"))
	{
		if (MSGlobals::DevModeEnabled)
		{
			ReturnString = "test_scripts/";
			ReturnString += MSGlobals::MapName;
			ReturnString += "/map_startup";
		}
		else
		{
			ReturnString = MSGlobals::MapName;
			ReturnString += "/map_startup";
		}
		return ReturnString;
	}
	else if (!strcmp(Text, "$currentmap_npc_externals")) //Thothie NOV2014_09 - adding for casual addition of additional map-exclusive scripts
	{
		if (MSGlobals::DevModeEnabled)
		{
			ReturnString = "test_scripts/";
			ReturnString += MSGlobals::MapName;
			ReturnString += "/npc_externals";
		}
		else
		{
			ReturnString = MSGlobals::MapName;
			ReturnString += "/npc_externals";
		}
		return ReturnString;
	}
	else if (!strcmp(Text, "$currentmap_player_externals")) //Thothie NOV2014_09 - adding for casual addition of additional map-exclusive scripts
	{
		if (MSGlobals::DevModeEnabled)
		{
			ReturnString = "test_scripts/";
			ReturnString += MSGlobals::MapName;
			ReturnString += "/player_externals";
		}
		else
		{
			ReturnString = MSGlobals::MapName;
			ReturnString += "/player_externals";
		}
		return ReturnString;
	}
	else if (!strcmp(Text, "$currentmap_game_master")) //Thothie NOV2014_09 - adding for casual addition of additional map-exclusive scripts
	{
		if (MSGlobals::DevModeEnabled)
		{
			ReturnString = "test_scripts/";
			ReturnString += MSGlobals::MapName;
			ReturnString += "/game_master";
		}
		else
		{
			ReturnString = MSGlobals::MapName;
			ReturnString += "/game_master";
		}
		return ReturnString;
	}


	if (Text[0] == '!')	//Not operator in if() statements
	{
		ReturnString = msstring("!") + GetConst(&Text[1]);
		return ReturnString;
	}

	//Handle the constant values within an interpreted parameter (Starts with '$')
	if (Text[0] == '$') //Example: $cansee(ent_lastseen,ATTACK_RANGE)
	{
		msstring Line = Text;
		msstring ParserName;
		msstringlist Params;

		if (BreakUpLine(Line, ParserName, Params))
		{
			msstring StackReturn = ParserName;		//This string must be on the stack, so recursive parsing doesn't screw up
			StackReturn += "(";
			for (size_t i = 0; i < Params.size(); i++)
			{
				StackReturn += GetConst(Params[i]);
				if (i != Params.size() - 1) StackReturn += ",";
				else StackReturn += ")";
			}

			ReturnString = StackReturn;		//Transfer from stack to global mem, for return
			return ReturnString;
		}
		else
			MSErrorConsoleText(__FUNCTION__, UTIL_VarArgs("Script: %s, \"%s\" - Mismatched Parenthesis!\n", m.ScriptFile.c_str(), Text));
	}
	else
		for (int i = 0; i < m_Constants.size(); i++)
			if (m_Constants[i].Name == Text)
				return m_Constants[i].Value;

	return Text;
}

bool GetString(char* Return, size_t size, const char* sentence, int start, char* endchars)
{
	// Quickie function to return the next CMD or parameter in a script string
	int i = 0, n, iPosition = start, endCharSize = strlen(endchars);
	strncpy(Return, "", size);
	while (i < (size - 1))
	{
		for (n = 0; n < endCharSize; n++)
			if (sentence[iPosition] == endchars[n])
				return true;

		if (sentence[iPosition] == 0)
			return false;

		Return[i] = sentence[iPosition];
		Return[i + 1] = '\0';
		iPosition++;
		i++;
	}
	return true;
}

scriptvar_t* IVariables::FindVar(msstring_ref Name)
{
	//Check local variables
	for (int i = 0; i < m_Variables.size(); i++)
	{
		if (FStrEq(Name, m_Variables[i].Name)) return &m_Variables[i];
	}

	return NULL;
}
msstring_ref IVariables::GetVar(msstring_ref Name)
{
	scriptvar_t* pScriptvar = FindVar(Name);
	if (pScriptvar) return pScriptvar->Value;

	return Name;
}
scriptvar_t* IVariables::SetVar(msstring_ref Name, msstring_ref Value)
{
	scriptvar_t* pScriptvar = FindVar(Name);
	if (pScriptvar) { pScriptvar->Value = Value; return pScriptvar; }

	return &m_Variables.add(scriptvar_t(Name, Value));
}

// Find an event by its name...
SCRIPT_EVENT* CScript::EventByName(const char* pszEventName)
{
	for (int i = 0; i < m.Events.size(); i++)
	{
		SCRIPT_EVENT& seEvent = m.Events[i];
		if (!seEvent.Name || seEvent.Name != pszEventName)
			continue;

		return &seEvent;
	}

	return NULL;
}
//Variable Handlers
scriptvar_t* CScript::FindVar(const char* pszName)
{
	scriptvar_t* pScriptvar = IVariables::FindVar(pszName);
	if (pScriptvar) return pScriptvar;

	//Check global variables
	for (int i = 0; i < m_gVariables.size(); i++)
		if (FStrEq(pszName, m_gVariables[i].Name)) return &m_gVariables[i];

	for (int i = 0; i < m_Constants.size(); i++)
	{
		if (FStrEq(pszName, m_Constants[i].Name))
		{
			return &m_Constants[i];
		}
	}

	return NULL;
}
bool CScript::VarExists(msstring_ref pszText)
{
	return (GetVar(pszText) != pszText);
}

//$and(Condition1[,...ConditionN])
//- Returns true if all conditions are true
//$or(Condition1[,...ConditionN])
//- Returns true if at least one condition is true
//$xor(Condition1[,...ConditionN])
//- Returns true if at exactly one condition is true
msstring CScript::ScriptGetter_Conjunction(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	// In case more are added
	bool bIsAnd = (ParserName == "$and");
	bool bIsOr = (ParserName == "$or");
	bool bIsXor = (ParserName == "$xor");

	bool bFoundTrue = false;

	if (bIsAnd || bIsOr || bIsXor)
	{
		for (int i = 0; i < Params.size(); i++)
		{
			// Allow shortcutting
			if (atoi(SCRIPTVAR(Params[i])))
			{
				if (bIsOr)
				{
					return "1";
				}
				if (bIsXor)
				{
					if (bFoundTrue)
					{
						return "0";
					}
					bFoundTrue = true;
				}
			}
			else
			{
				if (bIsAnd)
				{
					return "0";
				}
			}
		}

		if (bIsAnd) return "1";
		if (bIsOr)    return "0";
		if (bIsXor) return bFoundTrue ? "1" : "0";
	}

	return "0";
}

//$alphanum(<string>,[extras]) - strips non-alphanumeric except characters found in [extras]
//- eg. $alphanum(SOME_INPUT," ")
//- priority: very low, scope: shared
msstring CScript::ScriptGetter_AlphaNum(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//$alphanum(<string>,[extras]) - strips non-alphanumeric except characters found in [extras]
	//eg. $alphanum(SOME_INPUT," ")
	//priority: very low, scope: shared
	msstring Return;
	if (Params.size() >= 1)
	{
		//yes, this is a shit way of doing this, but I'm in a rush...
		msstring num_conv = Params[0];
		msstring out_str;
		int in_length = num_conv.len();
		char* cc;
		bool char_legit;
		msstring extra_list;
		bool check_extras = false;
		if (Params.size() > 1)
		{
			extra_list = Params[1];
			check_extras = true;
		}
		for (int i = 0; i <= in_length; i++)
		{
			char_legit = false;
			cc = num_conv.substr(i, 1).c_str();
			if (isalnum(cc[0]) > 0) char_legit = true;
			if (check_extras)
			{
				if (extra_list.contains(cc)) char_legit = true;
			}
			if (char_legit) out_str += cc;
		}
		return out_str.c_str();
	}
	else
	{
		return "0";
	}
}

//$anglediff(<destang>,<srcang>)
//- Returns the difference between two angles.  Accounts for 360 degree wrap-around
//priority: very high, scope: shared
msstring CScript::ScriptGetter_AngleDiff(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	msstring Return;
	if (Params.size() >= 2)
	{
		float DestAngle = atof(Params[0]);
		float SrcAngle = atof(Params[1]);
		RETURN_FLOAT(EngineFunc::AngleDiff(DestAngle, SrcAngle));
	}

	return FullName;
}

//$angles(<start_origin>,<end_origin>)
//� returns the 2d angle between two vectors
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_Angles(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Thothie MAR 2008a
	//- $angles(<start_origin>,<end_origin>) � returns the 2d angle between two vectors
	//- worried atan2 maybe CPU intensive. :/
	//priority: very high, scope: shared
	msstring Return;
	if (Params.size() >= 2)
	{
		Vector Start = StringToVec(Params[0]);
		Vector End = StringToVec(Params[1]);
		float dy = End.y - Start.y;
		float dx = End.x - Start.x;
		float rang = atan2(dy, dx); //gets radians
		float xang = rang * 57.295779; //back to degrees
		RETURN_FLOAT(xang)
	}
	else return "0";
}

//$angles3d(<source_origin>,<dest_origin>)
//- Returns 3d angle between two origins (used to determine angles on a vector to reach target)
//- When used client side, only returns the normalized result.
//- Pitch may need flipping.
//- priority: very high, scope: shared (ish)
msstring CScript::ScriptGetter_Angles3d(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	msstring Return;
	// FUCK THIS FUCKING FUNCTION ><
	//priority: very high, scope: shared (ish)
	Vector Start = StringToVec(Params[1]);
	Vector End = StringToVec(Params[0]);
	Vector out_vector;
	//Thoth - FEB2009_12 - Need this function too often, especially client side
	//$angles3d(<viewer_origin>,<origin_to_face>) - returns angles required for a vector to face a location
	//engine function fast, but not very accurate and doesn't work client side (returns normalized) - will need to flip pitch
	out_vector = UTIL_VecToAngles((Start - End).Normalize());
	RETURN_VECTOR(out_vector)
		/*
		//FAIL Thoth JAN2010_22 - steelin from the Quake source code so weez can do this client side
		//- meh, think this is an entirely different function - maybe I can just vectormultiply the results of $dir(vec,vec) for same result?
		Vector Vec_Org = StringToVec( Params[1] );
		Vector Vec_Face = StringToVec( Params[0] );
		Vector out_vector;
		float lforward, lyaw, lpitch;
		if ( Vec_Org.y == 0 && Vec_Org.x == 0 )
		{
			lyaw = 0;
			if ( Vec_Org.z > 0 )
				lpitch = 90;
			else
				lpitch = 270;
		}
		else
		{
			if (Vec_Org.x)
				lyaw = (atan2(Vec_Org.y, Vec_Org.x) * 180 / M_PI);
			else if (Vec_Org.y > 0)
				lyaw = 90;
			else
				lyaw = 270;

			if (lyaw < 0)
				lyaw += 360;

			lforward = sqrt(Vec_Org.x*Vec_Org.x + Vec_Org.y*Vec_Org.y);
			lpitch = (atan2(Vec_Org.y, lforward) * 180 / M_PI);
			if (lpitch < 0)
				lpitch += 360;
		}

		Vec_Face.x = -lpitch;
		Vec_Face.y = lyaw;
		Vec_Face.z = 0;

		out_vector = (Vec_Org-Vec_Face).Normalize();
		*/
}

//$anim_exists(<name>)
//- Checks if anim <name> exists within the calling scripted monster or player.
//- priority: moderate, scope: server
msstring CScript::ScriptGetter_AnimExists(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//priority: moderate, scope: server
#ifdef VALVE_DLL
	msstring Name = FullName.substr(13);
	Name = Name.thru_char(")");
	//Thothie NOV2015_02 - changing this to return index
	if (m.pScriptedEnt->IsMSMonster())
	{
		int seqidx = ((CBaseAnimating*)m.pScriptedEnt)->LookupSequence(GetScriptVar(Name));
		return UTIL_VarArgs("%i", seqidx);
	}
	else return "-1";
#endif

	return FullName;
}

//$can_damage(<target_ent>,[targetting_ent])
//- returns 1 if <target_ent> can damage [targetting_ent]
//- If [targetting_ent] isn't provided, the calling ent is assumed to be [targetting_ent]
//- This command is mostly for easier PVP checks, but has other uses
//- priority: high, scope: server
msstring CScript::ScriptGetter_CanDamage(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
#ifdef VALVE_DLL
	//MiB Mar2008a
	//$can_damage(<target_ent>,[targetting_ent])
	//If targetting_ent isn't provided, the calling ent is assumed to be targetting
	//This command is mostly for easier PVP checks, but has other uses
	//priority: high, scope: server
	msstring Return;
	if (Params.size() >= 1)
	{
		CBaseEntity* ThisEnt = Params.size() >= 2 ? m.pScriptedEnt->RetrieveEntity(Params[1]) : m.pScriptedEnt;
		CBaseEntity* ThatEnt = m.pScriptedEnt->RetrieveEntity(Params[0]);

		if (ThisEnt && ThatEnt)
		{
			RETURN_INT(ThisEnt->CanDamage(ThatEnt) ? 1 : 0)
		}
	}
	RETURN_INT(0)
#endif
		return FullName;
}

//$clcol(<0-255>|<RRR,GGG,BBB>)
//- Converts standard 16-bit color format to floats for when server sends colors to clients (alpha assumed 100%)
//- Some client entities (sprites, beams) use a scaling color system, this helps make dealing with that easier
//- priority: low, scope: shared
msstring CScript::ScriptGetter_clcol(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	if (Params.size() >= 1)
	{
		if (Params[0].starts_with("("))
		{
			Vector fcolor = StringToVec(Params[0]);
			if (fcolor.x > 0) fcolor.x /= 255;
			if (fcolor.y > 0) fcolor.y /= 255;
			if (fcolor.z > 0) fcolor.z /= 255;
			//Thothie OCT2015_29 - tweaking this to actually work proper (had no alpha def)
			char RetString[128];
			_snprintf(RetString, 128, "(%f,%f,%f,1.0)", fcolor.x, fcolor.y, fcolor.z);
			return RetString;
		}
		else
		{
			return FloatToString((atof(Params[0]) > 0) ? atof(Params[0]) / 255 : 0);
		}
	}
	else return "0";
}

//$cap_first(<var>) - Capitalize the first character of <var>
//- priority: low, scope: shared
msstring CScript::ScriptGetter_CapFirst(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB Feb2008a
	//$cap_first(<var>) - Capitalize the first character of <var>
	//priority: low, scope: shared
	if (Params.size() >= 1)
	{
		msstring sen = Params[0];
		msstring f = sen;
		f = strupr(f);
		sen[0] = f[0];
		return sen;
	}
	else
		return "0";
}

//$within_cone(<point origin>,<cone origin>,<cone angles>,<cone apex angle>)
//$within_cone2D(<point origin>,<cone origin>,<cone angles>,<cone apex angle>)
//- Returns 1 if <point origin> is within a cone. (2D ignores the z component.)
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_Cone(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Whether a point is within a defined cone
	//Dot product from defined cone
	//priority: very high, scope: shared
	msstring Return;
	if (Params.size() >= 4)	//point, cone origin, cone angles, cone fov
	{
		Vector PointOrigin = StringToVec(Params[0]);
		Vector ConeOrigin = StringToVec(Params[1]);
		Vector ConeAngles = StringToVec(Params[2]);
		float ConeFOV = cosf(atof(Params[3]) / 2.0f);

		Vector vForward;
		EngineFunc::MakeVectors(ConeAngles, vForward, NULL, NULL);

		Vector	vec2LOS = PointOrigin - ConeOrigin;

		if (ParserName == "$within_cone2D" ||
			ParserName == "$cone_dot2D") {
			vec2LOS.z = 0; vForward.z = 0;
		}

		vec2LOS = vec2LOS.Normalize();
		float flDot = DotProduct(vec2LOS, vForward);

		if (ParserName.starts_with("$within_cone"))
		{
			if (flDot >= ConeFOV) RETURN_TRUE
			else RETURN_FALSE
		}
		else RETURN_FLOAT(flDot);
	}
	else return "0";
}

//$dir(<start origin>,<end origin>)
//- Returns a normalized vector indicating the direction from <start origin> to <end origin> - which can be used with vectormultiply - see also $angles and $angles3d
//- example usage:
//-- setvard BEAM_START $get(ent_me,origin)
//-- setvard BEAM_TARGET $get(NPCATK_TARGET,origin)
//-- setvard BEAM_DIR $dir(BEAM_START,BEAM_TARGET)
//-- setvard BEAM_END BEAM_START
//-- vectormultiply BEAM_DIR 48 //48 units towards target BEAM_TARGET
//-- vectoradd BEAM_END BEAM_DIR
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_Dir(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//dir from point1 to point2
	//priority: very high, scope: shared
	msstring Return;
	if (Params.size() >= 2)
	{
		Vector Start = StringToVec(Params[1]);
		Vector End = StringToVec(Params[0]);
		Vector Dir = (Start - End).Normalize();
		RETURN_VECTOR(Dir);
	}
	else return "0";
}

//$dist(<start origin>,<end origin>)
//$dist2D(<start origin>,<end origin>)
//- Returns a float indicating the distance between the two points. 2D variant ignores the Z component.
//- (Note the capitlization.)
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_Dist(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	// dist between two points
	// $dist2D - Thothie JAN2013_06 - dist between two points, on 2D plane
	// MiB 29NOV_2014 - Comined these two commands because their only difference was the length function call,
	//		and because why not?
	//priority: very high, scope: shared
	msstring Return;
	if (Params.size() >= 2)
	{
		Vector Start = StringToVec(Params[0]);
		Vector End = StringToVec(Params[1]);
		float Dist = (ParserName == "$dist" ? (Start - End).Length() : (Start - End).Length2D());
		RETURN_FLOAT(Dist);
	}
	else
		return "0";
}

//$eval(<$cmd([...])|var_name>)
//- Executes the command and returns the result or resolves the variable.
//- Used to delay resolution of something until used, or to build a command/variable
//- string (MiB is a big fan of python)
msstring CScript::ScriptGetter_Eval(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	if (Params.size())
	{
		return GetVar(Params[0]);
	}
	return FullName;
}

//$indiam(<range>,<vec1>,<vec2>)
//$indiam2D(<range>,<vec1>,<vec2>)
//- Returns 1 if <vec1> is within <range> of <vec2> (or visa versa)
//- Thothie APR2016_15
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_indiam(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	msstring Return;
	if (Params.size() >= 3)
	{
		float range = atof(Params[0].c_str());
		Vector Start = StringToVec(Params[1]);
		Vector End = StringToVec(Params[2]);
		float Dist = (ParserName == "$indiam" ? (Start - End).Length() : (Start - End).Length2D());
		if (Dist <= range)
		{
			return "1";
		}
		else
		{
			return "0";
		}
	}
	else
	{
		//not enough param error
		ALERT(at_console, "Warning: Script: %s, - %s not enough paramters.\n", m.ScriptFile.c_str(), ParserName.c_str());
		return "0";
	}
}

//$filesize(file) - returns size of file in bytes
//- priority: low, scope: shared
msstring CScript::ScriptGetter_FileSize(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB Feb2008a
	//$filesize(file) - returns size of file
	//- for map verification mostly, atm
	//priority: low, scope: shared
	//Print("XEBUG: Filesize in %s\n",Params[0].c_str());

	msstring Return;
	if (Params.size() >= 1)
	{
		char cfileName[MAX_PATH];
#ifdef VALVE_DLL
		GET_GAME_DIR(cfileName);
#else
		strncpy(cfileName, gEngfuncs.pfnGetGameDirectory(), MAX_PATH);
#endif

		//GET_GAME_DIR( cfileName );
		msstring fileName = cfileName;
		fileName += "/";
		fileName += Params[0];
		ifstream file;
		file.open(fileName.c_str(), ios_base::in);
		if (file.is_open())
		{
			file.seekg(0, ios_base::end);
			long fileSize = file.tellg();
			file.close();
			RETURN_INT(fileSize);
		}
		else return "-1";
	}

	return FullName;
}

//$float(<precision>,<number>)
//- convert to float <precision> <value>
//or...
//$float(topercent,<float>,[base],[add_percent_sign:0|1])
//- priority: very low, scope: shared
msstring CScript::ScriptGetter_Float(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	/*
	else if ( op_type.starts_with("float2percent") ) //Thothie SEP2019_22 - tired of doing this script side
	{
		//$math(float2percent,<float>,[base_one:0|1]) - returns an integer with the float converted to a english friendly percentage%
		//0.1,0 returns "10%", 0.9,1 returns "90%"
		//1.1,0 returns "110%", 2.1,1 returns "110%"
		if ( Params.size() >= 2 )
		{
			end_result = atof(Params[1]);
			if ( end_result < 1 )
			{
				if ( Params.size() >= 3 )
				{
					if ( Params[2] != "0" ) return UTIL_VarArgs("%i%%%%",INT((1-end_result)*100));
				}
				return UTIL_VarArgs("%i%%%%",INT(end_result*100));
			}
			else
			{
				if ( Params.size() >= 3 )
				{
					if ( Params[2] != "0" ) return UTIL_VarArgs("%i%%%%",INT((end_result-1)*100));
				}
				return UTIL_VarArgs("%i%%%%",INT(end_result*100));
			}
		}
	}

	*/

	//$float(topercent,<float>,[base],[add_percent_sign])
	msstring Return;
	if (Params.size() >= 2)
	{
		if (Params[0] != "topercent")
		{
			char cReturn[MSSTRING_SIZE];
			msstring Return = msstring("%.") + atoi(Params[0]) + "f";
			_snprintf(cReturn, MSSTRING_SIZE, Return, atof(Params[1]));
			return msstring(cReturn);
		}
		else
		{
			//examples:
			//$float(topercent,0.1,0,1) = "10%"
			//$float(topercent,0.1,1,1) = "90%"
			//$float(topercent,1.5,1,1) = "50%"
			//$float(topercent,1.5,0,0) = "150"
			float perc_end_result = atof(Params[1]);
			float perc_base_point = 0;
			msstring perc_mask = "%i";
			if (Params.size() >= 3)
			{
				perc_base_point = atof(Params[2]);
			}
			if (Params.size() >= 4)
			{
				if (Params[3] != "0") perc_mask = "%i%%%%";
			}

			if (perc_end_result < perc_base_point)
			{
				return UTIL_VarArgs(perc_mask.c_str(), int((perc_base_point - perc_end_result) * 100.0));
			}
			else
			{
				return UTIL_VarArgs(perc_mask.c_str(), int((perc_end_result - perc_base_point) * 100.0));
			}
		}
	}
	return "0";
}

//$pass(<var>)
//- for passing callevent params without storing them
//- eg. callevent some_event $pass(PARAM1)
//- cuz I'm suck of doing this, and it's slightly more optimal anyways
//- priority: low, scope: shared
//- Thothie DEC2017_05
msstring CScript::ScriptGetter_Pass(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	msstring pass_string = Params[0]; //dunno why just returning Params[0].c_str() isn't working - kinda of ironic, given what I'm trying to fix
	return pass_string.c_str();
}

//$func(<eventname>,[params...])
//- calls eventname, replaces itself with return value
//- return doesn't actually stop code execution - if multiple returns are encountered, items will be tokenized
//- priority: moderate, scope: shared
msstring CScript::ScriptGetter_Func(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//AUG2013_27 Thothie - new function function
	//$func(<eventname>,[params...])
	//calls eventname, replaces itself with return value
	//return doesn't actually stop code execution - if multiple returns are encountered, items will be tokenized
	//priority: moderate, scope: shared
	msstring Return;
	msstring func_event = Params[0];
	msstringlist OutParams;
	OutParams.clearitems();
	if (Params.size() > 1)
	{
		for (int i = 0; i < Params.size(); i++)
		{
			if (i >= 1)
			{
				OutParams.add(Params[i]);
			}
		}
	}

	//for whatever reason on the client m_Scripts is empty, so a quick workaround fix...
	//should really figure out why m_Scripts is empty on the client.
	if (m.pScriptedInterface->m_Scripts.size() > 0)
		m.pScriptedInterface->CallScriptEvent(func_event.c_str(), &OutParams);
	else
		RunScriptEventByName(func_event.c_str(), &OutParams);

	if (m.pScriptedInterface->m_ReturnData.len())
	{
		return m.pScriptedInterface->m_ReturnData.c_str();
	}
	else
	{
		return "0";
	}
}

//$get(<entity>,<property>,[options])
//$get_by_idx(<entity_idx>,<property>,[options])
//- Gathers propeties from <entity> or <entity_idx>
//- Properties far too numerous to list, see CBaseEntity::GetProp
//- Partial properties list follows:
//-- [shared by all ents]
//-- id - entity pent id (standard target id)
//-- index - entity's index
//-- exists - entity exists
//-- origin - entity's vector origin (returns centers for players and fliers, foot level for others, does not work for brush entities without origins, nor NPCs without models)
//-- origin_center - as above, but attempts to return center
//-- angles - returns angles as vector
//-- name - entity's display name (sans prefix)
//-- gravity - returns gravity ratio
//-- speed - current forward velocity
//-- speed2D - current forward velocity, ignoring z unit
//-- forwardspeed - current forward velocity, compensating for angles
//-- velocity - current velocity, as vector
//-- anim.current_frame - returns current animation frame
//-- anim.index - returns current animation index
//-- anim.name - (bugged/wip: returns nadda)
//-- anim.max_frames - (bugged/wip: currently always returns 255)
//-- isplayer
//-- is_item
//-- movetype - current movetype
//--
//-- [shared by all scripted ents]
//-- name.prefix - name_prefix (eg a or an)
//-- name.full - full name, with prefix (a barrel of monkeys)
//-- name.full.capital - full name, with prefix, first letter capitalized (A barrel of monkeys)
//-- scriptname - full script name
//-- itemname - script name, sans path
//-- invincible
//-- dist/dist2D/range/range2D - returns target range from calling entity (bjorks if script has no model nor origin brush)
//-- scriptvar,'<var>' - returns scriptvar on target script - desired scriptvar should always be in single quotes to avoid conflicts
//-- haseffect,<effect_id> - returns 1 if applyeffect with EFFECT_ID is attached to target (see removeeffect command for more information)
//--
//-- [shared by players and monsters]
//-- hp/maxhp - health/max health
//-- mp/maxmp - manna/max manna
//-- height - target height in units
//-- width - target width in units
//-- onground
//-- inwater 0|1
//-- waterlevel 0=not in water, 1=up to waist, 2=deeper than waist level, 3=fully submerged
//-- eyepos - returns vector view position (in theory)
//-- viewangles - returns vector view angles
//-- gold - returns amount of gold player, monster, or chest has (in theory, maybe bugged)
//-- race
//-- relationship,<target> - returns enemy, ally, or neutral
//-- nopush - returns 1 if push immune
//-- svbonepos,<bone_index> - returns vector origin of bone, if found
//-- attachpos,<attachment_index> - returns vector origin of attachment, if found
//-- lasthitgroup - supposed to return last hitbox struck, but not currently working
//-- dmgmulti
//-- hitmulti
//-- renderprops - returns a token string with "<scale>;<rendermode>;<renderamt>;<bodyidx>;<skin>"
//--
//-- [players only]
//-- ducking - 0|1
//-- canattack - 0|1 (this will return 0 if player is midst attacking)
//-- canmove - 0|1
//-- canjump - 0|1
//-- canrun - 0|1
//-- canduck - 0|1
//-- sitting 0|1
//-- target - player's current target
//-- findbolt [remove] - returns players current selected ammo type, optionally remove (unless proj_bolt_generic is in use)
//-- steamid - player's full SteamID
//-- playerspawn - player's current spawn
//-- nextplayerspawn - player's next spawn (depreciated, we don't use this anymore.)
//-- lastmap - last map player was on
//-- isbot - various tests to determine if the player is a bot, returns 1 if any test positive
//-- gender - returns "male" or "female"
//-- ip - returns "localhost" for listenservers
//-- spellname,<idx> - returns spell scriptname in slot <idx> or 0
//-- jumping
//-- keydown,<attack1|attack2|use|jump|moveleft|moveright|back|forward|duck>
//-- stamina
//-- stamina.ratio
//-- anim.type - part of the player_animation script that I'd like to move code side
//-- anim.uselegs - ditto
//-- torso_anim - ditto
//-- legs_anim - ditto
//-- currentitem.anim_torso - ditto
//-- currentitem.anim_legs - ditto
//-- in_attack_stance
//-- active_item - id of currently active item
//-- skillavg - average of all skills
//-- title - player's current title
//-- stat.<statname> - returns stat
//-- skill.<skillname>[.substat] - returns skill or substat of skill
//-- numitems - total number of inventory items
//--
//-- [monsters only]
//-- xp/skilllevel - mobs current XP value
//-- spawner - entity ID of ms_monsterspawn spawnarea
//-- roam - returns 1 if monster's roam function is active
//-- movedest.origin - current setmovedest origin
//-- moveprox|movedest.prox - current move range
//-- movedest.id - target of setmovedest, if an entity, and not an origin
//-- anim_end - time my current animation will end (likely way off)
//-- stepsize - mob's current stepsize
//-- hpmulti
//-- fly -  returns 1 if movetype is MOVETYPE_FLY
//-- walkspeed
//-- runspeed
//-- movespeed
//-- framerate
//--
//-- [items only]
//-- value - base gold value
//-- is_projectile
//-- is_drinkable
//-- is_spell
//-- attacking
//-- inhand - returns 0|1, not hand idx
//-- is_worn
//-- inworld
//-- drink_amt (same as quality)
//-- quality
//-- maxquality
//-- quantity
//-- hand_index returns: 0=right, 1=left, 2=both [two handed], 3=undroppable
//-- handpref as above, but returns item's default hand definition instead of current location
//-- owner - owner ID
//-- container.open - 1 if container is open
//-- container.items - #items in container
//--
//- priority: very high, scope: server
msstring CScript::ScriptGetter_Get(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//priority: very high, scope: server
	// $get( <entity>, <property>, [options] )
	// $get( <entity idx>, <property>, [options] )
	// $get | $get_by_idx
#ifdef VALVE_DLL
	msstring Return;
	if (Params.size() >= 2)
	{
		msstring& TargetVar = Params[0];
		msstring& ValueVar = Params[1];

		if (ValueVar == "is_entity_type") return TargetVar.find(ENT_PREFIX) != msstring_error ? "1" : "0";

		if (m.pScriptedEnt)
		{
			CBaseEntity* pEntity = NULL;
			if (FullName.starts_with("$get_by_idx("))
				pEntity = MSInstance(INDEXENT(atoi(TargetVar)));
			else
				pEntity = m.pScriptedEnt->RetrieveEntity(TargetVar);

			//Thothie adding invincible prediction (fail)
			//if ( ValueVar == "invincible" ) return pEntity->pev->flags,FL_GODMODE ? "1" : "0";

			if (pEntity)
				return (m.pScriptedEnt->GetProp(pEntity, ValueVar, Params));
		}
	}
	return "0";
#endif

	return FullName;
}

//$get_array([Target,]<array_name>,<idx>) - Returns value of <idx> in <array_name> or -1, if <idx> not found.
//$get_array_exists([Target,]<array_name>) - Returns 0|1 based on whether array exists
//$get_arrayfind([Target,]<array_name>,<search_str>,[start_idx])	- Returns index of first instance of search_str in <array_name> ( -1 if not found )
//$get_array_amt([Target,]<array_name>) - Returns # of elements in array
//- all these functions:
//-- Return [ERROR_NO_ARRAY] if array not found (except: "exists", which returns 0)
//-- Return [ERROR_MISSING_PARAMS] if not given enough Parameters.
//- priority: high, scope: uncertain
msstring CScript::ScriptGetter_GetArray(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	// MiB JAN2010_26	Scripted array functions
	// MiB JUN2010_25 Added global functionality
	//priority: high, scope: uncertain
	/*
		Return [ERROR_NO_ARRAY] when the given array can't be found.
		Return [ERROR_MISSING_PARAMS] if not given enough Parameters.
		$get_array(<name>,<idx>)							Returns <name>[idx]		( -1 for bad index )
		$get_arrayfind(<name>,<search_str>,[start_idx])		Returns index of first instance of search_str after <name>[idx] ( -1 if not found )
		$get_array_amt(<name>)								Returns number of elements in <name>   ( -1 if couldn't find array )
	*/
	msstring Return;
	size_t vParam = 0;
	bool bIsGlobal = ParserName.starts_with("$g_get_array");
	CBaseEntity* pEnt = NULL;
	bool bExisted;
	if (Params.size() >= 1)
	{
		msstring ArrName = Params[vParam++];
		if (!bIsGlobal && Params.size() > vParam)
		{
			// Check if the first parameter is an entity
			pEnt = RetrieveEntity(ArrName);
			if (pEnt)
			{
				ArrName = Params[vParam++];
			}
		}
		if (!pEnt)
		{
			pEnt = m.pScriptedEnt;
		}

		msscriptarrayhash& vArrayHash = bIsGlobal ? GlobalScriptArrays : pEnt->scriptedArrays;
		msscriptarray* pArray = GetScriptedArrayFromHashMap(vArrayHash, ArrName, false, &bExisted);

		if (!bExisted)
		{
			//Thothie AUG2013_28 - dis not work like it says it work in description above (fixing)
			if (ParserName.ends_with("array") && Params[1].contains("exist")) return "0"; //Thothie JAN2020_02 - fix $get_array(<array>,exists)
			if (ParserName.ends_with("amt"))
			{
				return "-1";
			}
			else
				if (ParserName.ends_with("exists")) //Thothie NOV2014_16 simplify
				{
					return "0";
				}
				else
				{
					return "[ERROR_NO_ARRAY]";
				}
		}
		else
		{
			//Thothie JAN2020_02 - fix $get_array(<array>,exists)
			if (ParserName.ends_with("array") && Params[1].contains("exist")) return "1";
		}

		if (ParserName.ends_with("exists")) return "1"; //Thothie NOV2014_16 simplify

		if (ParserName.ends_with("array"))
		{
			if (Params.size() > vParam)
			{
				int                     vSubIndx = atoi(Params[vParam++]);
				if (vSubIndx > -1 && pArray->size() > (unsigned)vSubIndx) //Thothie DEC2017_19 NOTE: if the script requests an index out of range, this will cause the function to return its literal - screwy, but servicable (just check that pull request does not start with '$get_array').
					return (*pArray)[vSubIndx];
			}
		}
		else
			if (ParserName.ends_with("find"))
			{

				if (Params.size() > vParam)
				{
					msstring                vsSrch = Params[vParam++];
					msstring                vsSrchType = Params.size() > vParam ? Params[vParam++] : "equals";

					int                     vSrchType;
					if (vsSrchType == "startswith")       vSrchType = 0;
					else if (vsSrchType == "endswith")    vSrchType = 1;
					else if (vsSrchType == "contains")    vSrchType = 2;
					else if (vsSrchType == "equals")      vSrchType = 3;
					else
					{
						vSrchType = 3;
						--vParam; // Reset to get start index
					}

					int                     vStrtIndx = Params.size() > vParam ? atoi(Params[vParam++]) : 0;
					bool                    bCaseInsensitive = Params.size() > vParam ? atoi(Params[vParam++]) == 1 : false;

					vStrtIndx = max(vStrtIndx, 0);
					if (bCaseInsensitive) vsSrch = strlwr(vsSrch);
					for (size_t i = vStrtIndx; i < pArray->size(); ++i)
					{
						bool                bFnd = false;
						msstring            vsCur = (*pArray)[i];
						if (bCaseInsensitive) vsCur = strlwr(vsCur);

						switch (vSrchType)
						{
						case 0:
							bFnd = vsCur.starts_with(vsSrch);
							break;
						case 1:
							bFnd = vsCur.ends_with(vsSrch);
							break;
						case 2:
							bFnd = vsCur.contains(vsSrch);
							break;
						case 3:
							bFnd = vsCur == vsSrch;
							break;
						}
						if (bFnd)
							RETURN_INT(i);
					}

					RETURN_INT(-1);
				}
				else
					return "[ERROR_MISSING_PARAMS]";

			}
			else if (ParserName.ends_with("amt"))
				RETURN_INT(pArray->size())
	}
	else
		return "[ERROR_MISSING_PARAMS]";

	return FullName;
}

//$get_attackprop(<item>,<attack_idx>,<property>)
//- Returns registered attack data, see below for details.
//- priority: low, scope: server
msstring CScript::ScriptGetter_GetAttackProp(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB DEC2007a - return stats of an attack
	//$get_attackprop(<target>,<attack_idx>,<property>)
	//priority: low, scope: server
#ifdef VALVE_DLL
	msstring Return;
	if (Params.size() >= 2)
	{
		CGenericItem* pItem = (CGenericItem*)RetrieveEntity(Params[0]);
		int attackNum = atoi(Params[1]);

		//Thothie OCT2016_22 sanity check
		if (!pItem)
		{
			MSErrorConsoleText("$get_attackprop", UTIL_VarArgs(" used on non-item! %s\n", m.ScriptFile.c_str()));
			return "0";
		}

		//Thothie OCT2016_05 no attack data awareness
		int maxattack = pItem->m_Attacks.size();
		maxattack = maxattack - 1;
		if (attackNum > maxattack) return "[NO_ATTACK]";

		attackdata_t& AttData = pItem->m_Attacks[attackNum];
		msstring& PropName = Params[2];

		if (PropName == "type") return msstring(AttData.sDamageType.c_str());
		else if (PropName == "numattacks") RETURN_INT(pItem->m_Attacks.size()) //Thothie OCT2016_22 get number of attacks
		else if (PropName == "range") RETURN_FLOAT(AttData.flRange)
		else if (PropName == "dmg") RETURN_FLOAT(AttData.flDamage)
		else if (PropName == "dmg.range") RETURN_FLOAT(AttData.flDamageRange)
		else if (PropName == "dmg.type") return msstring(AttData.sDamageType.c_str());
		else if (PropName == "dmg.multi") RETURN_FLOAT(AttData.f1DmgMulti)
		else if (PropName == "aoe.range") RETURN_FLOAT(AttData.flDamageAOERange)
		else if (PropName == "aoe.falloff") RETURN_FLOAT(AttData.flDamageAOEAttn)
		else if (PropName == "energydrain") RETURN_FLOAT(AttData.flEnergy)
		else if (PropName == "mpdrain") RETURN_FLOAT(AttData.flMPDrain)
			//else if( PropName == "stat" ) return STRING( AttData.
		else if (PropName == "stat.prof") RETURN_INT(AttData.StatProf)
		else if (PropName == "stat.balance") RETURN_INT(AttData.StatBalance)
		else if (PropName == "stat.power") RETURN_INT(AttData.StatPower)
		else if (PropName == "stat.exp") RETURN_INT(AttData.StatExp)
		else if (PropName == "noise") RETURN_FLOAT(AttData.flNoise)
		else if (PropName == "callback") return AttData.CallbackName;
		else if (PropName == "ofs.startpos") return STRING(VecToString(AttData.StartOffset));
		else if (PropName == "ofs.aimang") return STRING(VecToString(AttData.AimOffset));
		else if (PropName == "priority") RETURN_INT(AttData.iPriority)
		else if (PropName == "keys") return AttData.ComboKeys[0];
		else if (PropName == "dmg.ignore") return AttData.NoDamage ? "1" : "0";
		else if (PropName == "hitchance") RETURN_FLOAT(AttData.flAccuracyDefault)
		else if (PropName == "critthreshold") RETURN_FLOAT(AttData.flCritThreshold)
		else if (PropName == "critmulti") RETURN_FLOAT(AttData.flCritMulti) 
		else if (PropName == "delay.end") RETURN_FLOAT(AttData.tDuration)
		else if (PropName == "delay.strike") RETURN_FLOAT(AttData.tLandDelay)
		else if (PropName == "dmg.noautoaim") return AttData.NoAutoAim ? "1" : "0";
		else if (PropName == "ammodrain") RETURN_INT(AttData.iAmmoDrain)
		else if (PropName == "hold_min&max")
		{
			msstring Return;
			_snprintf(Return, MSSTRING_SIZE, "%.2f;%.2f", AttData.tProjMinHold, AttData.tMaxHold);
			return Return;
		}
		else if (PropName == "projectile") return STRING(AttData.sProjectileType);
		else if (PropName == "COF")
		{
			msstring Return;
			_snprintf(Return, MSSTRING_SIZE, "%.2f;%.2f", AttData.flAccuracyDefault, AttData.flAccBest);
			return Return;
		}
		else if (PropName == "delay.end") return STRING(AttData.tDuration);
		else return "ILLEGAL_PROPERTY";
	}
	else return "-1";
#endif

	return FullName;
}

//$get_by_name(<name>,<property)
//- gets entity properties by targetname or name_unique
//- likely best to $get_by_name(<name>,id), then store in a var for use with standard $get() commands
//- priority: moderate, scope: server
msstring CScript::ScriptGetter_GetByName(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//priority: moderate, scope: server
#ifdef VALVE_DLL
	msstring Return;
	if (Params.size() >= 1)
	{
		CBaseEntity* pEntity = UTIL_FindEntityByString(NULL, "netname", msstring("¯") + Params[0]);
		if (pEntity)
			return EntToString(pEntity);
		else
		{
			//Thothie DEC2014_11 check map ents too
			//Print("XEBUG: Checkin map ents for %s\n",Params[0].c_str());
			CBaseEntity* pEntity = UTIL_FindEntityByString(NULL, "targetname", Params[0].c_str());
			if (pEntity) return EntToString(pEntity);
		}
	}
	return "0";
#endif

	return FullName;
}

//$getcl(<entity idx>,<property>)
//- Client side. Gets various properties from client side entities, including:
//-- origin
//-- center - based on mins/maxs - not always reliable
//-- angles
//-- velocity
//-- inwater
//-- underwater - returns 1 if completely submerged based on mins/maxs
//-- waterorigin - returns current origin, with z set to water level
//-- model - model or sprite name
//-- ducking (players only)
//-- modelidx - precache index of model
//-- anim - current animation index
//-- frame - current frame
//-- framerate - current frame rate (returns ratio, for models)
//-- exists
//-- gravity
//-- scale
//-- scaleHD - returns scale x1000 for greater float percision
//-- fuser1HD - returns fuser1x1000 for greater float percision
//-- renderfx - current renderfx
//-- renderamt - current remderamt
//-- rendermode - current rendermode
//-- rendercolor - vec of current color
//-- visible - returns 0 if no draw flag
//-- isplayer
//-- attachment0-3 - vec origin of attachment0-3
//-- iuser1-4 - current int of iuser1-4
//-- fuser1-4 - current float of fuser1-4
//-- prevstate.iuser1-4 - previous state of iuser1-4
//-- prevstate.fuser1-4 - previous state of fuser1-4
//-- baseline.iuser1-4 - initial state of iuser1-4
//-- baseline.fuser1-4 - initial state of fuser1-4
//-- bonepos,<idx> - vec origin of a model bone
//-- bonecount - number of bones in model
//-- viewangles (players only)
//- most of these can also be returned in tempent update or collision events, via game.tempent.<property>
//- priority: high, scope: client
//$get_local_prop(<property>)
//- provides additional information about the client the script is currently running on
//- currently this only includes: gender and race
//- priority: low, scope: client
msstring CScript::ScriptGetter_GetCl(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	// MiB 30NOV_2014 Putting these together so they can be combined in the future

	//$getcl( <client entity idx>, <property> )
	//priority: high, scope: client

	//MiB FEB2010_02
	//$get_local_prop(<property>)
	//Returns a property about the client player. Difference between this and $getcl is that it accesses
	//CBasePlayer properties of the client, too. Could also add the cl_entity_t properties of $getcl if
	//one wants to save time/processing speed.
	//priority: very low, scope: client


#ifndef VALVE_DLL
	msstring Return;
	if (ParserName == "$getcl")
	{
		if (Params.size() >= 2)
		{
			cl_entity_t* pclEntity = NULL;
			int EntIdx = atoi(Params[0]);
			if (EntIdx > 0)
			{
				for (int i = 0; i < CLPERMENT_TOTAL; i++)
					if (EntIdx == MSCLGlobals::CLViewEntities[i].index)
					{
						pclEntity = &MSCLGlobals::CLViewEntities[i]; break;
					}

				if (!pclEntity) pclEntity = gEngfuncs.GetEntityByIndex(EntIdx);
			}
			if (pclEntity) return (CLGetEntProp(pclEntity, Params));
		}
		return "0";
	}
	else if (ParserName == "$get_local_prop")
	{
		if (Params.size() >= 1)
		{
			msstring Prop = Params[0];

			if (Prop == "gender") return player.m_Gender == GENDER_MALE ? "male" : "female";
			else if (Prop == "race") return "human";
		}
		else
			return "0";
}
#endif

	return FullName;
}

//- This doesn't appear to be in the definitions, and thus maybe bjorked (also, comment inside is wrong)
//- Seems to be trying to return constant value, not much use save for testing
//- be VERY nice to have the ability to pull a constant's value on ANOTHER script
//- but likely best done from GetProps, eg. $get(<target>,scriptconst,'CONST_NAME')
msstring CScript::ScriptGetter_GetConst(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Thothie - JUNE2007 $get_cvar(<cvar_name>)
	for (int i = 0; i < m_Constants.size(); i++)
	{
		if (m_Constants[i].Name == Params[0])
		{
			return m_Constants[i].Value;
		}
	}

	return "CANNOT_FIND";
}

//$get_contents(<origin>)
//- Returns contents of a point [both client and server], possible returns are: empty, solid, water, slime, lava, sky, unknown (last usually indicates error)
//- priority: high, scope: shared
msstring CScript::ScriptGetter_GetContents(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Thothie AUG2010_22
	//$get_contents(<origin>) - return contents value of origin
	//AUG2013_22 Thothie - (wow, three years later) - This is always returning solid (-2) on some maps, but only server side
	//priority: high, scope: shared
	Vector thothie_vec = StringToVec(Params[0]);
	int thothie_result = EngineFunc::Shared_PointContents(StringToVec(Params[0]));
	msstring thothie_return = "unknown";
	if (thothie_result == CONTENTS_EMPTY) thothie_return = "empty";
	else if (thothie_result == CONTENTS_SOLID) thothie_return = "solid";
	else if (thothie_result == CONTENTS_WATER) thothie_return = "water";
	else if (thothie_result == CONTENTS_SLIME) thothie_return = "slime";
	else if (thothie_result == CONTENTS_LAVA) thothie_return = "lava";
	else if (thothie_result == CONTENTS_SKY) thothie_return = "sky";
	return thothie_return.c_str();
}

//$getcl_tsphere(<origin>,<radius>,[flags])
//- Returns indexes of all ents in area as token string.
//- [flags] include:
//-- players - only return player ents
//-- addbrushes - include brush entities (these are normally ommited)
//- priority: moderate, scope: client
msstring CScript::ScriptGetter_GetClTSphere(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
#ifndef VALVE_DLL
	//Thothie - CL side tsphere
	//$getcl_tsphere(<origin>,<radius>,[flags]) - returns indexes of all ents in area as token string
	//priority: moderate, scope: client
	//this is rapidly called, but only used in one or two scripts
	Vector scanpos = StringToVec(Params[0]);
	float scanrange = atof(Params[1]);
	msstring out_str;

	int numents = 1024; //only returnign worldspawn ents - MSCLGlobals::m_ClEntites.size();
	bool flag_justplayers = false;
	bool flag_incbrushes = false;
	Vector Start;
	Vector End;
	float Dist;
	msstring sTemp;

	if (Params[2].contains("players")) flag_justplayers = true;
	if (Params[2].contains("addbrushes")) flag_incbrushes = true;
	for (int idx = 1; idx < numents; idx++)
	{
		cl_entity_t* pclEntity = gEngfuncs.GetEntityByIndex(idx);
		if (!pclEntity) continue;
		if (!pclEntity->model) continue;
		if (!pclEntity->index) continue;
		if (pclEntity->origin == g_vecZero) continue; //brush model with no origin - can't get distance
		if (!flag_incbrushes)
		{
			sTemp = UTIL_VarArgs("%s", pclEntity->model);
			if (sTemp.contains("*")) continue;
		}
		if (flag_justplayers)
		{
			if (!pclEntity->player) continue;
		}
		Start = scanpos;
		End = pclEntity->origin;
		Dist = (Start - End).Length();
		//Print("$getcl_tsphere: %i %s (%f vs %f) \n",pclEntity->index, pclEntity->model, Dist, scanrange);
		if (Dist <= scanrange)
		{
			out_str += UTIL_VarArgs("%i;", pclEntity->index);
		}
	}
	if (out_str.len() > 0) return out_str.c_str();
	else return "none";
#endif

	return FullName;
}

//$getcl_beam(<beam_idx>,<property>)
//- returns value of client side beam properties
//- priority: very low, scope: client
//- Thothie DEC2014_10 beam_update
msstring CScript::ScriptGetter_getcl_beam(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
#ifndef VALVE_DLL
	int beamid = atoi(Params[0]);
	return (CLGetBeamProp(beamid, Params));
#else
	return FullName; //some foo called me server side
#endif
}

//$get_cvar(<cvar_name>)
//- returns value of a cvar (similar to game.cvar.<cvar>)
//- priority: low, scope: shared
msstring CScript::ScriptGetter_GetCVar(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	return EngineFunc::CVAR_GetString(Params[0].c_str());
}

//$get_fileline(<file>,[line])
//- Read line from file
//- priority: very low, scope: server
msstring CScript::ScriptGetter_GetFileLine(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB Feb2008a � �Read� a line from a file
	//- $get_fileline(<file>,[line])
	//priority: very low, scope: server
//#ifdef VALVE_DLL //Thothie MAR2019_02 - MiB says removing this will allow client file access
	if (Params.size() >= 1)
	{
		msstring fileName = Params[0];

		bool found = false;
		if (m.pScriptedEnt->filesOpen.size() >= 1)
		{
			for (int i = 0; i < m.pScriptedEnt->filesOpen.size(); i++)
			{
				//Check to see if we already have this file open
				if (m.pScriptedEnt->filesOpen[i].fileName == fileName)
				{
					found = true;
					if (Params.size() >= 2)
						return m.pScriptedEnt->filesOpen[i].ScriptFile_ReadLine(atoi(Params[1])); //Read specific line
					else
						return m.pScriptedEnt->filesOpen[i].ScriptFile_ReadLine(); //Read next line

					break;
				}
			}
		}

		if (!found) //The file wasn't found
		{
			scriptfile_t file;
			file = fileName; //Open the file
			m.pScriptedEnt->filesOpen.add(file); //Add it to the list of opened files
			if (Params.size() >= 2)
				return m.pScriptedEnt->filesOpen[m.pScriptedEnt->filesOpen.size() - 1].ScriptFile_ReadLine(atoi(Params[1]));
			else
				return m.pScriptedEnt->filesOpen[m.pScriptedEnt->filesOpen.size() - 1].ScriptFile_ReadLine();
		}
	}
	//#endif

	return FullName;
}

//$get_find_token(<token_string>,<search_string>)
//- returns idx of found string, or -1 if not found
//- search is exact, and case senstive - may make flags for partial matches later
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_GetFindToken(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//priority: very high, scope: shared
	//Thothie DEC2007a - save a common complex scripting step
	//$get_find_token(<token_string>,<search_string>)
	//- returns idx of found string, or -1 if not found
	//- search is exact, and case senstive - may make flags for partial matches later
	msstring Return;
	if (Params.size() >= 2)
	{
		static msstringlist Tokens;
		Tokens.clearitems();
		msstring& TokenAdd = Params[1];
		bool thoth_partial_search = false; //Thothie SEP2019_09 - partial searches, needed sooner than later, since I r stuck
		if (Params.size() >= 3) thoth_partial_search = true; //Thothie SEP2019_09 - partial searches

		msstring TokenString = GetVar(Params[0]);
		TokenizeString(TokenString, Tokens);

		int token_found_at = -1;

		for (int i = 0; i < Tokens.size(); i++)
		{
			if (!thoth_partial_search) { if (Tokens[i] == TokenAdd) token_found_at = i; } //Thothie SEP2019_09 - partial searches
			else { if (Tokens[i].contains(TokenAdd)) token_found_at = i; } //Thothie SEP2019_09 - partial searches
		}

		RETURN_INT(token_found_at);
	}
	else return "-1";
}

//$get_random_token(<token_string>)
//- returns random token within set
//- priority: low, scope: shared
//Thothie - MAY2016_04
msstring CScript::ScriptGetter_get_random_token(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	if (Params.size() >= 1)
	{
		static msstringlist Tokens;
		Tokens.clearitems();
		//msstring &TokenAdd = Params[1];

		msstring TokenString = GetVar(Params[0]);
		TokenizeString(TokenString, Tokens);

		//int token_found_at = -1;

		//int max_token = Tokens.size()-1; //Thothie SEP2019_04 Never seems to return 0 token
		int rnd_token_idx = (RANDOM_LONG(1, Tokens.size())) - 1;
		return Tokens[rnd_token_idx].c_str();
	}
	else return "-1";
}

//$tokenize(<strings...>)
//- Puts a bunch of variables or strings into a token set
//- priority: low, scope: shared
//Thothie - SEP2019_15
msstring CScript::ScriptGetter_tokenize(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	if (Params.size() >= 1)
	{
		msstring stemp = "";
		for (int i = 0; i < Params.size(); i++)
		{
			stemp.append(Params[i].c_str());
			stemp.append(";");
		}
		return stemp.c_str();
	}
	else return "-1";
}

//$random_of_set(<params...>)
//- returns random paramater within set
//- priority: low, scope: shared
//Thothie - MAY2016_04
msstring CScript::ScriptGetter_random_of_set(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	if (Params.size() >= 1)
	{
		int max_param = Params.size() - 1;
		int rnd_idx = RANDOM_LONG(0, max_param);
		return Params[rnd_idx].c_str();
	}
	else return "-1";
}

//$get_ground_height(<origin>)
//- returns z coordinate of ground at <origin> or "none" if not found (which I assume means origin is off map.)
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_GetGroundHeight(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//priority: very high, scope: shared
	msstring Return;
	if (Params.size() >= 1)
	{
		Vector StartOrigin = StringToVec(GetScriptVar(Params[0]));
		Vector EndOrigin = StartOrigin + Vector(0, 0, -8092);

		sharedtrace_t Tr;
		EngineFunc::Shared_TraceLine(StartOrigin, EndOrigin, ignore_monsters, Tr, 0, PM_GLASS_IGNORE | PM_WORLD_ONLY, NULL);

		if (Tr.Fraction < 1.0)
			RETURN_FLOAT(Tr.EndPos.z)
			return "none";
	}

	return FullName;
}

// All below return "[ERROR_NO_HASHMAP]" if map doesn't exist, except $get_hashmap_exists
// $[g_]get_hashmap([Entity,]<map_name>,<key>)                              Returns the value at key if it exists, otherwise "[ERROR_NO_KEY]"
// $[g_]get_hashmap_haskey([Entity,]<map_name>,<key>)                       Returns 0|1, whether or not the map has the given key
// $[g_]get_hashmap_exists([Entity,]<map_name>)                             Returns 0|1, whether or not a map exists with the given name
// $[g_]get_hashmap_amt([Entity,]<map_name>)                                Returns the number of entries in the map
// $[g_]get_hashmap_keys([Entity,]<map_name>[,arr_name])                    Gets the list of keys in the map
//                                                                          If arr_name is provided, the keys are dumped into an array of that name and the return is the number of keys
//                                                                          If arr_name is NOT provided, returns a tokenized list of keys. Not responsible for keys that contain ";"
// $[g_]get_hashmap_values([Entity,]<map_name>[,arr_name])                  Gets the list of values in the map
//                                                                          If arr_name is provided, the values are dumped into an array of that name and the return is the number of values
//                                                                          If arr_name is NOT provided, returns a tokenized list of values. Not responsible for values that contain ";"
// $[g_]get_hashmap_pairs([Entity,]<map_name>,<arr_name>[,value_arr_name])  Gets the list of key-value pairs in the map
//                                                                          If value_arr_name is provided, the first array will contain the keys in the map and the second will contain the values in the map
//                                                                          If value_arr_name is NOT provided, the array will contain each pair as "key;value". Not responsible for keys/values that contain ";"
msstring CScript::ScriptGetter_GetHashMap(
	msstring& FullName
	, msstring& ParserName
	, msstringlist& Params
)
{
	msstring                            Return;
	size_t                              vParam = 0;
	msstring                            vsSubCmd;
	bool                                bGlobal = false;
	CBaseEntity* pEntity = NULL;
	msstring                            vsHashMapName;
	bool                                bExisted;
	msscripthash::iterator              iHashMap;
	if (Params.size())
	{
		vsHashMapName = Params[vParam++];
		if (ParserName.starts_with("$g_"))
		{
			bGlobal = true;
			vsSubCmd = ParserName.substr(3);
		}
		else
		{
			vsSubCmd = ParserName.substr(1);
			pEntity = RetrieveEntity(vsHashMapName);
			if (pEntity)
			{
				vsHashMapName = Params[vParam++];
			}
		}

		if (!pEntity)
		{
			pEntity = m.pScriptedEnt;
		}

		msscripthashhash& vHashMapHashMap = bGlobal ? mGlobalScriptHashes : pEntity->mScriptedHashes;
		msscripthash* pHashMap = GetScriptedHashMapFromHashMap(vHashMapHashMap, vsHashMapName, false, &bExisted);
		if (!bExisted)
		{
			if (vsSubCmd == "get_hashmap_exists") return "0";
			ErrorPrint("NO_HASHMAP"
				, ERRORPRINT_CONSOLE | ERRORPRINT_LOG | ERRORPRINT_CVAR
				, "HashMap %s doesn't exist for command %s"
				, vsHashMapName.c_str()
				, ParserName.c_str()
			);
			return "[ERROR_NO_HASHMAP]";
		}

		if (vsSubCmd == "get_hashmap_exists")
		{
			return "1";
		}
		else
			if (vsSubCmd == "get_hashmap")
			{
				if (Params.size() > vParam)
				{
					iHashMap = pHashMap->find(Params[vParam++]);
					if (iHashMap == pHashMap->end()) return "[ERROR_NO_KEY]";
					return iHashMap->second;
				}
			}
			else
				if (vsSubCmd == "get_hashmap_haskey")
				{
					if (Params.size() > vParam)
					{
						return (pHashMap->find(Params[vParam++]) == pHashMap->end()) ? "0" : "1";
					}
				}
				else
					if (vsSubCmd == "get_hashmap_amt")
					{
						RETURN_INT(pHashMap->size());
					}
					else
						if (vsSubCmd == "get_hashmap_keys"
							|| vsSubCmd == "get_hashmap_values"
							|| vsSubCmd == "get_hashmap_pairs"
							)
						{
							bool                        bKeys = false;
							bool                        bValues = false;
							bool                        bPairs = false;
							if (vsSubCmd == "get_hashmap_keys") bKeys = true;
							else if (vsSubCmd == "get_hashmap_values") bValues = true;
							else if (vsSubCmd == "get_hashmap_pairs") bPairs = true;

							bool                        bFir = true;
							msstring                    vsTokenReturn;

							msscriptarray* pFirstArr = NULL;
							msscriptarray* pSecondArr = NULL;
							if (Params.size() > vParam)
							{
								pFirstArr = m.pScriptedEnt->GetScriptedArray(Params[vParam++], true);
								if (bPairs && Params.size() > vParam) pSecondArr = m.pScriptedEnt->GetScriptedArray(Params[vParam++], true);
							}
							else
								if (bPairs)
								{
									return FullName; // Must have at least 1 array for pairs
								}

							for (iHashMap = pHashMap->begin()
								; iHashMap != pHashMap->end()
								; iHashMap++
								)
							{
								msstring                vsToken;
								if (bKeys)
								{
									vsToken = iHashMap->first;
								}
								else
									if (bValues)
									{
										vsToken = iHashMap->second;
									}
									else
										if (bPairs)
										{
											if (pSecondArr)
											{
												pFirstArr->push_back(iHashMap->first);
												pSecondArr->push_back(iHashMap->second);
											}
											else
											{
												vsToken = msstring(iHashMap->first) + ";" + iHashMap->second;
											}
										}

								if (pFirstArr)
								{
									if (!pSecondArr) pFirstArr->push_back(vsToken);
								}
								else
								{
									if (!bFir) vsToken = msstring(";") + vsToken;
									bFir = false;
									if (vsToken.len() + vsTokenReturn.len() > MSSTRING_MAXLEN) break;
									vsTokenReturn += vsToken;
								}
							}
							if (pFirstArr) RETURN_INT(pFirstArr->size());
							return vsTokenReturn;
						}
	}
	return FullName;
}


//$get_insphere(<id|player|monster|any>,<radius>,[(origin)])
//- Depreciated. Returns id of first qualifying ent found in sphere. (Or "1" when searching for specific <id>)
//- Better to use $get_tsphere for proper sorting.
//- More efficient to use range or $dist to see if a specific ent is near something.
//- priority: moderate, scope: server
msstring CScript::ScriptGetter_GetInSphere(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
#ifdef VALVE_DLL
	//$get_insphere(<search_type>,<radius>,[source_origin])
	//returns first detected ent in sphere
	//(Needs to be changed to return nearest instead)
	//<search_type> can be: id, player, monster, or any
	//priority: moderate, scope: server
	//rarely used now, but tends to be called rapidly when it is - more or less defunct
	msstring& Name = Params[0];
	float thoth_boxsize = atof(Params[1]);
	//float neg_boxsize = thoth_boxsize * -1;
	CBaseEntity* pList[255], * pEnt = NULL;
	Vector StartPos;
	if (Params.size() == 2) StartPos = m.pScriptedEnt->pev->origin;
	else if (Params.size() >= 3) StartPos = StringToVec(Params[2]);
	int count = UTIL_MonstersInSphere(pList, 255, StartPos, thoth_boxsize);
	bool spec_search = false;

	CBaseEntity* pSpecificEnt = RetrieveEntity(Name);

	if (!Name.starts_with("player") && !Name.starts_with("monster") && !Name.starts_with("any")) spec_search = true;

	//ALERT( at_aiconsole, "Searching through %i ents\n", count );

	for (int i = 0; i < count; i++)
	{
		pEnt = pList[i];

		if (!pEnt->IsAlive()) continue; //dont count anything dead

		if (m.pScriptedEnt->entindex() == pEnt->entindex()) continue; //dont count self

		if (spec_search)
		{
			if (pSpecificEnt->entindex() == pEnt->entindex()) return "1";
			continue;
		}

		if (!stricmp("player", Name) && pEnt->IsPlayer())
			return EntToString(pEnt);

		if (!stricmp("monster", Name) && pEnt->IsMSMonster() && !pEnt->IsPlayer())
			return EntToString(pEnt);

		if (!stricmp("any", Name))
			return EntToString(pEnt);

		//This doesn't work here - works in $get_tsphere though :(
		if (!strcmp("enemy", Name))
		{
			CMSMonster* pMonster = (CMSMonster*)pEnt;
			int my_relate = pMonster->IRelationship(m.pScriptedEnt);
			switch (my_relate)
			{
			case -3:
			case -4:
			case -5:
				EntToString(pEnt);
				break;
			}
		}

		if (!strcmp("ally", Name))
		{
			CMSMonster* pMonster = (CMSMonster*)pEnt;
			int my_relate = pMonster->IRelationship(m.pScriptedEnt);
			if (my_relate == 1) return EntToString(pEnt);
		}
	}
	return "0";
#endif

	return FullName;
}

//$get_jointype(<player>)
//- retrieve join type status for player (used for proper travel/gauntlet authentication)
//- priority: very low, scope: server
msstring CScript::ScriptGetter_GetJoinType(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Thothie $get_jointype(<target>) - retrieve map status player was on
	//priority: very low, scope: server
#ifdef VALVE_DLL
	CBaseEntity* pEntity = m.pScriptedEnt ? m.pScriptedEnt->RetrieveEntity(Params[0]) : NULL;

	if (pEntity && pEntity->IsPlayer())
	{
		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
		const char* JoinTypeText = "unknown";
		if (pPlayer->m_CharacterState == CHARSTATE_UNLOADED) JoinTypeText = "Spectate";
		else
		{
			switch (pPlayer->m_JoinType)
			{
			case JN_TRAVEL:		JoinTypeText = "Travel"; break;
			case JN_STARTMAP:	JoinTypeText = "Start Map"; break;
			case JN_VISITED:	JoinTypeText = "Previously Visited"; break;
			case JN_ELITE:		JoinTypeText = "GM"; break;
			case JN_NOTALLOWED: JoinTypeText = "Not Allowed"; break;
			}
		}
		return JoinTypeText; // pPlayer->m_JoinType
	}

	return "[unknown]";
#endif

	return FullName;
}

//$get_lastmap(<player>)
//- Returns last map player was on (for gauntlet map authentication).
//- priority: very low, scope: server
msstring CScript::ScriptGetter_GetLastMap(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Thothie $get_lastmap(<target>) - retrieve lastmap player was on
	//priority: very low, scope: server
#ifdef VALVE_DLL
	CBaseEntity* pEntity = m.pScriptedEnt ? m.pScriptedEnt->RetrieveEntity(Params[0]) : NULL;

	if (pEntity && pEntity->IsPlayer())
	{
		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
		return pPlayer->m_cEnterMap;
	}

	return "[unknown]";
#endif

	return FullName;
}

//$get_map_legit(<player>)
//- Yet another way to verify if the player's map travel is authentic.
//- priority: very low, scope: server
msstring CScript::ScriptGetter_GetMapLegit(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Thothie $get_map_valid(<target>) - retrieve map status player was on
	//priority: very low, scope: server
#ifdef VALVE_DLL
	CBaseEntity* pEntity = m.pScriptedEnt ? m.pScriptedEnt->RetrieveEntity(Params[0]) : NULL;

	if (pEntity && pEntity->IsPlayer())
	{
		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
		if (pPlayer->m_MapStatus == INVALID_MAP) return "0";
		if (pPlayer->m_MapStatus != INVALID_MAP) return "1";
	}
	return "[unknown]";
#endif

	return FullName;
}

//$get_quest_data(<player>,<quest_name>)
//- returns current value of quest data <quest_name>
//- priority: low, scope: server
msstring CScript::ScriptGetter_GetQuestData(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
#ifdef VALVE_DLL
	//( <target>, <questname> )
	//priority: low, scope: server
	msstring& Name = Params[1];
	CBaseEntity* pEntity = m.pScriptedEnt ? m.pScriptedEnt->RetrieveEntity(Params[0]) : NULL;

	if (pEntity && pEntity->IsPlayer())
	{
		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
		for (int i = 0; i < pPlayer->m_Quests.size(); i++)
			if (pPlayer->m_Quests[i].Name == Name)
				return pPlayer->m_Quests[i].Data;
	}
	return "0";
#endif

	return FullName;
}

// All below return "[ERROR_NO_SET]" if set doesn't exist, except $get_set_exists
// $[g_]get_set([Entity,]<set_name>,<index>)
// - Gets the <index> element in the set. Note that this isn't strictly "proper" behavior
// - for sets to get a value by index, since they enforce no order, but it'll be easier
// - on the scripts. Order may change when items are inserted/removed, so don't rely on
// - any given index always pointing to an element. Should only be used to loop over values
// $[g_]get_set_hasvalue([Entity,]<set_name>,<value>)
// - Determines if the given value is in the set
// $[g_]get_set_exists([Entity,]<set_name>)
// - Determines if the set exists
// $[g_]get_set_amt([Entity,]<set_name>)
// - Gets the number of values in the set
msstring CScript::ScriptGetter_GetSet(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	msstring Return;
	size_t vParam = 0;
	msstring vsSubCmd;
	bool bGlobal = false;
	CBaseEntity* pEntity = NULL;
	msstring vsSetName;
	bool bExisted;
	msscriptset::iterator iVal;
	if (Params.size())
	{
		vsSetName = Params[vParam++];
		if (ParserName.starts_with("$g_"))
		{
			bGlobal = true;
			vsSubCmd = ParserName.substr(3);
		}
		else
		{
			vsSubCmd = ParserName.substr(1);
			pEntity = RetrieveEntity(vsSetName);
			if (pEntity)
			{
				if (Params.size() > vParam)
				{
					vsSetName = Params[vParam++];
				}
				else
				{
					return FullName;
				}
			}
		}

		if (!pEntity)
		{
			pEntity = m.pScriptedEnt;
		}

		msscriptsethash& vSetHashMap = bGlobal ? mGlobalScriptSets : pEntity->mScriptedSets;
		msscriptset* pSet = GetScriptedSetFromHashMap(vSetHashMap
			, vsSetName
			, false
			, &bExisted
		);
		if (bExisted)
		{
			if (vsSubCmd == "get_set_exists")
			{
				return "1";
			}
		}
		else
		{
			if (vsSubCmd == "get_set_exists")
			{
				return "0";
			}
			ErrorPrint("NO_SET"
				, ERRORPRINT_CONSOLE | ERRORPRINT_LOG | ERRORPRINT_CVAR
				, "Set %s doesn't exist for command %s"
				, vsSetName.c_str()
				, ParserName.c_str()
			);
			return "[ERROR_NO_SET]";
		}

		if (vsSubCmd == "get_set")
		{
			if (Params.size() > vParam)
			{
				int                     vIndx = atoi(Params[vParam++]);
				if (vIndx >= 0 && (unsigned)vIndx < pSet->size())
				{
					iVal = pSet->begin();
					std::advance(iVal, vIndx);
					return (*iVal);
				}
			}
		}
		else
			if (vsSubCmd == "get_set_hasvalue")
			{
				if (Params.size() > vParam)
				{
					msstring                sValu = Params[vParam++];
					iVal = pSet->find(sValu);
					return (iVal == pSet->end()) ? "0" : "1";
				}
			}
			else
				if (vsSubCmd == "get_set_amt")
				{
					RETURN_INT(pSet->size());
				}
	}

	return FullName;
}

//$get_scriptflag(<target>,<type>,type_exists) - returns 1 if <type> found
//$get_scriptflag(<target>,<type>,type_first) - returns first value of <type> found or "none"
//$get_scriptflag(<target>,<type>,type_count) - returns the # of flags of this <type> that exist on the <target> or "none"
//$get_scriptflag(<target>,<type>,type_value) - returns total value of all <type> flags combined, or "none"
//$get_scriptflag(<target>,<type>,type_array) - returns name of array, created on <target>, with all values of <type> or "none"
//$get_scriptflag(<target>,<name>,name_exists) - returns 1 if <name> found
//$get_scriptflag(<target>,<name>,name_value) - returns value if <name> found or "none"
//$get_scriptflag(<target>,<name>,name_type) - returns type if <name> found or "none"
//$get_scriptflag(<target>,listall) - list all flags [debuggary,developer build only]
//- retrieves scriptflag data
//- do not use quotes around <name> or <type>
//- priority: high, scope: server
//Thothie JAN2013_02 scriptflags
msstring CScript::ScriptGetter_GetScriptFlag(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
#ifdef VALVE_DLL
	bool sf_add_values = false;
	bool sf_get_by_name = false;
	bool sf_get_first_value = false;
	bool sf_get_value_by_name = false;
	bool sf_get_type_by_name = false;
	bool sf_count_values = false;
	bool sf_array_values = false;

	msstring sf_not_found_msg;
	bool sf_found = false;
	bool sf_setup_array = false;
	float sf_total_values = 0;
	msstring sf_array_name = "ARRAY_";
	static msstringlist Parameters;
	msstring Return;

	//we may add other function options here later
	if (Params[2] == "totalvalue" || Params[2] == "type_value")
	{
		sf_not_found_msg = "none";
		sf_add_values = true;
	}
	else if (Params[2] == "name_exists")
	{
		sf_not_found_msg = "0";
		sf_get_by_name = true;
	}
	else if (Params[2] == "first_value" || Params[2] == "type_first")
	{
		sf_not_found_msg = "none";
		sf_get_first_value = true;
	}
	else if (Params[2] == "name_value")
	{
		sf_not_found_msg = "none";
		sf_get_by_name = true;
		sf_get_value_by_name = true;
	}
	else if (Params[2] == "name_type")
	{
		sf_not_found_msg = "none";
		sf_get_by_name = true;
		sf_get_type_by_name = true;
	}
	else if (Params[2] == "type_count")
	{
		sf_not_found_msg = "none";
		sf_add_values = true;
		sf_count_values = true;
	}
	else if (Params[2] == "type_array")
	{
		sf_not_found_msg = "none";
		sf_add_values = true;
		sf_array_values = true;
		msstring arrayext = Params[1];
		sf_array_name += _strupr(arrayext.c_str());
	}
	else if (Params[2] == "type_exists")
	{
		sf_not_found_msg = "0";
	}

	CBaseEntity* pEntity = RetrieveEntity(Params[0]);
	IScripted* iScripted = pEntity ? pEntity->GetScripted() : NULL;
	if (pEntity && iScripted)
	{
#if !TURN_OFF_ALERT
		if (Params[1] == "listall")
		{
			Print("Scriptflags for %s:\n", pEntity->m_DisplayName.c_str());
			for (int i = 0; i < pEntity->m_scriptflags.names.size(); i++)
			{
				Print("# %i type %s name %s val %s exp %s\n", i, pEntity->m_scriptflags.types[i].c_str(), pEntity->m_scriptflags.names[i].c_str(), pEntity->m_scriptflags.values[i].c_str(), pEntity->m_scriptflags.expiretimes[i].c_str());
			}
			return "0";
		}
#endif
		for (int i = 0; i < pEntity->m_scriptflags.names.size(); i++)
		{
			if (!sf_get_by_name)
			{
				if (pEntity->m_scriptflags.types[i] == Params[1])
				{
					if (!sf_add_values)
					{
						if (!sf_get_first_value)
						{
							return "1"; //type_exists
						}
						else
						{
							msstring sTemp = pEntity->m_scriptflags.values[i];
							return sTemp.c_str(); //value_first
						}
						break;
					}
					else
					{
						if (sf_array_values)
						{
							//type_array
							sf_found = true;

							if (!sf_setup_array)
							{
								sf_setup_array = true;
								Util_ScriptArray(pEntity, "create", sf_array_name.c_str(), "0");
							}

							Util_ScriptArray(pEntity, "add", sf_array_name.c_str(), pEntity->m_scriptflags.values[i].c_str());
						}
						else if (sf_count_values)
						{
							sf_found = true;
							sf_total_values = sf_total_values + 1;
						}
						else
						{
							sf_found = true;
							sf_total_values = sf_total_values + atof(pEntity->m_scriptflags.values[i].c_str());
						}
					}
				}
			} //end get by value

			if (sf_get_by_name) //would else, but simplify for later additions
			{
				if (pEntity->m_scriptflags.names[i] == Params[1])
				{
					sf_found = true;
					if (sf_get_type_by_name)
					{
						return pEntity->m_scriptflags.types[i].c_str();
					}
					else if (sf_get_value_by_name)
					{
						return pEntity->m_scriptflags.values[i].c_str();
					}
					else
					{
						return "1";
					}
					break;
				}
			} //end get by name
		} //end big ass loop

		//no point in else if's here, as these all return, so save some nests
		if (!sf_found)
		{
			return sf_not_found_msg.c_str();
		}
		else
		{
			//either type_value or type_count or type_array
			if (!sf_array_values)
			{
				RETURN_FLOAT(sf_total_values)
			}
			else
			{
				//Print("XEBUG: $get_scriptflags value_array %f\n",sf_total_values);
				return sf_array_name.c_str();
			}
		}
	}
	else
	{
		MSErrorConsoleText("scriptflags", UTIL_VarArgs("$get_scriptflags target entity not found in %s\n", m.ScriptFile.c_str()));
	}
#endif

	return FullName;
}

//$get_skillname(<skillname>)
//- Turns <skillname> into english friendly variants.
//- eg. "skill.bluntarms" returns "Blunt Arms" (skill. is optional)
//- priority: low, scope: shared
msstring CScript::ScriptGetter_GetSkillName(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB JUL2010_02   $get_skillname(<skill>)
	// Turns "skill.bluntarms" to "Blunt Arms" (skill. is optional)
	//priority: low, scope: shared
	if (Params.size() >= 1)
	{
		msstring Skill = Params[0].starts_with("skill.") ? Params[0].substr(6) : Params[0];
		return GetSkillName(GetSkillStatByName(Skill.c_str()));
	}

	return FullName;
}

//$ratio(<ratio(0-1)>,<Min Amt>,<Max Amt>,[inversed])
//$get_skill_ratio(<ratio(0-1)>,<Min Amt>,<Max Amt>,[inversed])
//- Returns a number between Min and Max based on the float ratio.
//- eg. $ratio(0.5,1,100) == 50
//- inversed flag is supposed to return the inversed ratio, but seems unreliable. (Though I don't see why, off hand.)
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_GetSkillRatio(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//else if( ParserName == "$get_skill_ratio" || ParserName == "$ratio" )
	//priority: very high, scope: shared
	msstring Return;
	if (Params.size() >= 3)
	{
		float Ratio = atof(Params[0]);
		float Min = atof(Params[1]), Max = atof(Params[2]);

		if (Params.size() >= 4)
		{
			if (Params[3] == "inversed") Ratio = 1 - Ratio;
		}

		float Range = Max - Min;
		float Value = Min + Range * Ratio;
		RETURN_FLOAT(Value)
	}

	return FullName;
}

//$get_sky_height(<origin>)
//- Returns Z coordinate of sky brush or "none" - screwy sometimes, $get_traceline is more dependable for finding ceilings
//- priority: high, scope: shared
msstring CScript::ScriptGetter_GetSkyHeight(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//priority: high, scope: shared
	msstring Return;
	if (Params.size() >= 1)
	{
		float SkyHeight = 0;
		bool FoundSky = FindSkyHeight(StringToVec(GetScriptVar(Params[0])), SkyHeight);
		if (FoundSky) RETURN_FLOAT(SkyHeight);
		return "none";
	}

	return FullName;
}

//$get_takedmg(<target>,<type>)
//- Returns ratio of damage taken by a damage type (lightning/fire/blunt/slash/etc)
//- priority: high, scope: server
msstring CScript::ScriptGetter_GetTakeDmg(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//$get_takedmg(<target>,<type>) FEB2009 Thothie
	//priority: high, scope: server
#ifdef VALVE_DLL
	//$get_takedmg(<target>,<type>) FEB2009 Thothie
	CMSMonster* pTarget = (CMSMonster*)RetrieveEntity(Params[0]);
	msstring Return;
	if (!pTarget) return "-1"; //FEB2009
	if (Params[1] == "all") RETURN_FLOAT(pTarget->m.GenericTDM); //MAR2010_03
	for (int i = 0; i < pTarget->m.TakeDamageModifiers.size(); i++)
	{
		CMSMonster::takedamagemodifier_t& TDM = pTarget->m.TakeDamageModifiers[i];
		msstring read_dmgtype = TDM.DamageType;
		//Print("Reading_Takedmg %s\n", read_dmgtype.c_str() );
		if (read_dmgtype.contains(Params[1]))	RETURN_FLOAT(TDM.modifier)
	}
	return "1.0";
#endif

	return FullName;
}

//$get_time(month|day|year|version)
//- not working, except for version, plz fix
//- meanwhile, use: game.time.day|minute|hour|year|month|dow|[.since] (see "time.")
//- these return local server time, or local client time, depending on scope.
//- priority: very low, scope: server
msstring CScript::ScriptGetter_GetTime(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Thothie $get_time(month|day|year|version) - used to verify MS.DLL / holiday events
	//priority: very low, scope: server
	msstring& Flags = Params[0];
	msstring Return;
	/*
	//always returns 0, should return 0-11 :\
	struct tm *timeinfo = (struct tm *)malloc(sizeof(struct tm));

	if( Flags.contains("month" ) )
	{
		int gtmonth = timeinfo->tm_mon;
		ALERT( at_console, "Pulled %i %d %f %s", gtmonth,gtmonth,gtmonth,gtmonth);
		return UTIL_VarArgs("%i",gtmonth);
	}*/

	if (Flags.contains("version"))
	{
		return EngineFunc::CVAR_GetString("ms_version"); //Thothie JUN2007a - changed this to work proper (broken ver was added JAN2007b)
	}

	return FullName;
}

//$get_token(<token_string>,<idx>)
//- returns an item in a token string
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_GetToken(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//priority: very high, scope: shared
	msstring Return;
	if (Params.size() >= 2)
	{
		static msstringlist Tokens;
		Tokens.clearitems();
		int GetItem = atoi(Params[1]);

		msstring TokenString = GetVar(Params[0]);
		TokenizeString(TokenString, Tokens);

		if (GetItem >= 0 && GetItem < (signed)Tokens.size())
		{
			return Tokens[GetItem];
		}
		else return "0";
	}
	else return "0";
}

//$get_token_amt(<token_string>)
//- returns number of items in a token string
//- Note that this will return 1 on an uninitialized string, as the string itself is a taken to be a token.
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_GetTokenAmt(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Get number of tokens in string
	//priority: very high, scope: shared
	msstring Return;
	if (Params.size() >= 1)
	{
		static msstringlist Tokens;
		Tokens.clearitems();

		TokenizeString(Params[0], Tokens);

		RETURN_INT(Tokens.size());
	}
	else return "0";
}

//$get_traceline(<vec_start)>,<vec_end>,[flags:ent|worldonly|tracebox|clworld],[ignore_ent])
//- flags:
//-- worldonly - returns point at which line intersects with world, or <end> if no such collision happens. (If client side, this also stops for collision boxes.)
//-- ent - returns id or index of any intersected entity (untested client side)
//-- tracebox - requires [ignore_ent] be set, and only works server side - return is same as worldonly, but stops for collision boxes, like the client does.
//-- clworld - ignores collision boxes, but also ignores solid map ents, such as func_walls
//-- contents - pending: returns content type at each end of trace (eg. "empty;sky")
//- priority: high, scope: shared
msstring CScript::ScriptGetter_GetTraceLine(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Thothie $get_traceline(<vec>,<vec>,[flag],[ignore_ent])
	//flags: worldonly returns end pos, ent returns ent hit
	//- Traceline does not work, so attempting to make this way instead
	//priority: high, scope: shared
	Vector StartPos = StringToVec(Params[0]);
	Vector EndPos = StringToVec(Params[1]);
	msstring& Flags = Params[2];
	msstring Return;

	bool thoth_return_contents = false;

	CBaseEntity* pIgnoreEnt =
#ifdef VALVE_DLL
	(Params.size() >= 4) ? StringToEnt(Params[3]) : NULL;
#else
		NULL;
#endif

	int iFlags = 0;
#ifdef VALVE_DLL
	if (Flags.contains("ent") || Flags.contains("tracebox"))				iFlags = dont_ignore_monsters;
	else if (Flags.contains("worldonly")) SetBits(iFlags, ignore_monsters);
	else if (Flags.contains("contents"))
	{
		thoth_return_contents = true;
		SetBits(iFlags, ignore_monsters);
	}
#else
	if (Flags.contains("ent"))			iFlags = 0;
	if (Flags.contains("worldonly"))
	{
		//Thothie SEP2011_09 - took some tweaking to get cl traces to actually be worldonly
		//iFlags = 65536;
		//iFlags = 0;
		//NONE OF THIS SHIT WORKS!!!
		//- effing client trace ALWAYS traces hitboxes, no matter what I try
		SetBits(iFlags, PM_WORLD_ONLY); //doesn't work
		//SetBits( iFlags, PM_STUDIO_IGNORE ); //same as PM_WORLD_ONLY
		//SetBits( iFlags, PM_GLASS_IGNORE ); //doesn't actually ignore glass, but when combined with ignore studio, does a proper world trace *sometimes*
	}
	else if (Flags.contains("contents"))
	{
		thoth_return_contents = true;
		SetBits(iFlags, PM_WORLD_ONLY);
	}
	else if (Flags.contains("clworld"))
	{
		iFlags = 32; //SEP2011_16 - thothie's special clworld flag
	}
#endif

	sharedtrace_t Tr;

	EngineFunc::Shared_TraceLine(StartPos, EndPos, iFlags, Tr, 0, iFlags, pIgnoreEnt ? pIgnoreEnt->edict() : NULL);

	if (Flags.contains("worldonly") || Flags.contains("tracebox"))
	{
		return VecToString(Tr.EndPos);
	}
	else if (!Flags.contains("contents"))
	{
		//Thothie AUG2010_05 (comment only)
		//This probably won't work right client side
		//can we work a version that would return hit model indexes?
		CBaseEntity* pHitEnt = Tr.HitEnt ? MSInstance(INDEXENT(Tr.HitEnt)) : NULL;
		//msstring dbg_result = pHitEnt ? EntToString(pHitEnt) : VecToString(Tr.EndPos);
		return (pHitEnt ? EntToString(pHitEnt) : msstring(VecToString(Tr.EndPos)));
	}
	else if (Flags.contains("contents"))
	{
		//Thothie AUG2010_05
		//This doesn't seem to be working right due to the world only flag
		//probably need to work into a differnet function
		//as this also needs to return if the entire trace is empty
		msstring thoth_contents_string;
		//Thothie AUG2013_22 - start pos, then end pos, silly
		int thoth_start_contents = EngineFunc::Shared_PointContents(StartPos);
		int thoth_end_contents = EngineFunc::Shared_PointContents(EndPos);
		if (thoth_start_contents == CONTENTS_EMPTY) thoth_contents_string = "empty;";
		else if (thoth_start_contents == CONTENTS_SOLID) thoth_contents_string = "solid;";
		else if (thoth_start_contents == CONTENTS_WATER) thoth_contents_string = "water;";
		else if (thoth_start_contents == CONTENTS_SLIME) thoth_contents_string = "slime;";
		else if (thoth_start_contents == CONTENTS_LAVA) thoth_contents_string = "lava;";
		else if (thoth_start_contents == CONTENTS_SKY) thoth_contents_string = "sky;";
		else if (thoth_start_contents == 0) thoth_contents_string.append("unknown");

		if (thoth_end_contents == CONTENTS_EMPTY) thoth_contents_string.append("empty");
		else if (thoth_end_contents == CONTENTS_SOLID) thoth_contents_string.append("solid");
		else if (thoth_end_contents == CONTENTS_WATER) thoth_contents_string.append("water");
		else if (thoth_end_contents == CONTENTS_SLIME) thoth_contents_string.append("slime");
		else if (thoth_end_contents == CONTENTS_LAVA) thoth_contents_string.append("lava");
		else if (thoth_end_contents == CONTENTS_SKY) thoth_contents_string.append("sky");
		else if (thoth_end_contents == 0) thoth_contents_string.append("unknown");
		return thoth_contents_string.c_str();
	}

	return FullName;
}

//$get_tsphere(<filter:any|player|monster|enemy|ally>,<radius>,[(origin)])
//$get_tbox(<filter:any|player|monster|enemy|ally>,<box_size>,[(origin)])
//$get_tbox_abs(<filter:any|player|monster|enemy|ally>,<box_mins>,<box_maxs>)
//- makes a token string listing all ents in radius (max ~10 tokens)
//- PS. Inconsistencies between these two in the way they validate found entities, should be merged.
//- PS. Also, $get_asphere, $get_abox - dump to array, might be handy for large scans
//- priority: high, scope: server
msstring CScript::ScriptGetter_GetTSphereAndBox(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Thothie DEC2007a
	//$get_tsphere/isphere(<search_type>,<radius>,[source_origin])
	//Same as in sphere but returns all targets in a token string
	//(original $get_insphere shoulda worked like this, but I ain't thought of it then
	//cant search for specific ents with this, so old $get_insphere still not useless
	//tsphere returns tokenized entities, isphere variant returns tokenized indexes
	//<search_type> can be: any, player, monster, enemy, or ally
	//priority: high, scope: server

	//$get_tbox() - same as $get_tsphere, but uses box for optimization when you don't need to be accurate
	//- tends to be rapidly used where it is, but rarely used, and more or less defunct
	//- should probably merge with $get_tsphere so the functions are consistent
	//priority: moderate, scope: server

	//$get_tbox_abs(<search_type:any|player|monster|enemy|ally>,<absminx>,<absmaxs>)
	//- properly defined box scan
	//priority: moderate, scope: server

	// MiB 30NOV_2014 - Put these two together so they can be combined in the future
#ifdef VALVE_DLL
	if (ParserName.starts_with("$get_tsphere") || ParserName.starts_with("$get_isphere")) //Thothie APR2016_17 - adding isphere for more ents before overflow
	{
		//[start] Thothie APR2016_17 - isphere
		bool return_idx = false;
		if (ParserName.starts_with("$get_isphere")) return_idx = true;
		//[end] Thothie APR2016_17 - isphere
		msstring& Name = Params[0];
		msstring thoth_token_string;
		msstring ent_str;
		int str_limit = 214;
		float thoth_boxsize = atof(Params[1]);
		//float neg_boxsize = thoth_boxsize * -1;
		CBaseEntity* pList[255], * pEnt = NULL;
		Vector StartPos;
		if (Params.size() == 2) StartPos = m.pScriptedEnt->pev->origin;
		else if (Params.size() >= 3) StartPos = StringToVec(Params[2]);
		int count = UTIL_MonstersInSphere(pList, 255, StartPos, thoth_boxsize);

		//ALERT( at_aiconsole, "Searching through %i ents\n", count );

		for (int i = 0; i < count; i++)
		{
			if ((int)thoth_token_string.len() > str_limit) break; //outta room

			pEnt = pList[i];

			if (!pEnt->IsAlive()) continue; //dont count anything dead

			if (m.pScriptedEnt->entindex() == pEnt->entindex()) continue; //dont count self

			if (!stricmp("player", Name) && pEnt->IsPlayer())
			{
				ent_str = return_idx ? msstring(UTIL_VarArgs("%i", pEnt->entindex())) : EntToString(pEnt); //Thothie APR2016_17 - isphere
				int total_len = thoth_token_string.len() + ent_str.len();
				if (total_len < str_limit)
				{
					thoth_token_string += ent_str;
					thoth_token_string += ";";
				}
			}

			if (!stricmp("monster", Name) && pEnt->IsMSMonster() && !pEnt->IsPlayer())
			{
				ent_str = return_idx ? msstring(UTIL_VarArgs("%i", pEnt->entindex())) : EntToString(pEnt); //Thothie APR2016_17 - isphere
				int total_len = thoth_token_string.len() + ent_str.len();
				if (total_len < str_limit)
				{
					thoth_token_string += ent_str;
					thoth_token_string += ";";
				}
			}

			if (!stricmp("any", Name) && !pEnt->IsMSItem())
			{
				ent_str = return_idx ? msstring(UTIL_VarArgs("%i", pEnt->entindex())) : EntToString(pEnt); //Thothie APR2016_17 - isphere
				int total_len = thoth_token_string.len() + ent_str.len();
				if (total_len < str_limit)
				{
					thoth_token_string += ent_str;
					thoth_token_string += ";";
				}
			}

			if (!strcmp("enemy", Name))
			{
				CMSMonster* pMonster = (CMSMonster*)pEnt;
				int my_relate = pMonster->IRelationship(m.pScriptedEnt);
				switch (my_relate)
				{
				case -3:
				case -4:
				case -5:
				{
					if (pMonster->m_Race)
					{
						ent_str = return_idx ? msstring(UTIL_VarArgs("%i", pEnt->entindex())) : EntToString(pEnt); //Thothie APR2016_17 - isphere
						int total_len = thoth_token_string.len() + ent_str.len();
						if (total_len < str_limit)
						{
							thoth_token_string += ent_str;
							thoth_token_string += ";";
						}
					}
					break;
				}
				}
			}

			if (!strcmp("ally", Name))
			{
				CMSMonster* pMonster = (CMSMonster*)pEnt;
				int my_relate = pMonster->IRelationship(m.pScriptedEnt);
				switch (my_relate)
				{
				case 1:
				{
					if (pMonster->m_Race)
					{
						ent_str = return_idx ? msstring(UTIL_VarArgs("%i", pEnt->entindex())) : EntToString(pEnt); //Thothie APR2016_17 - isphere
						int total_len = thoth_token_string.len() + ent_str.len();
						if (total_len < str_limit)
						{
							thoth_token_string += ent_str;
							thoth_token_string += ";";
						}
					}
					break;
				}
				}
			}
		}
		if (thoth_token_string.len() > 0) return thoth_token_string.c_str();
		else return "none";
	}
	else if (ParserName.starts_with("$get_tbox"))
	{
		msstring& Name = Params[0];
		msstring thoth_token_string;
		msstring ent_str;
		int str_limit = 214;
		CBaseEntity* pList[255], * pEnt = NULL;
		Vector StartPos;

		//Thothie OCT2015_11 - $get_tbox_abs(<scan_type>,<absmins>,<absmaxs>) - allow properly defined box
		Vector thoth_mins;
		Vector thoth_maxs;
		if (!ParserName.starts_with("$get_tbox_abs"))
		{
			//$get_tbox
			float thoth_boxsize = atof(Params[1]);
			Vector thoth_delta = Vector(thoth_boxsize, thoth_boxsize, thoth_boxsize);
			if (Params.size() == 2) StartPos = m.pScriptedEnt->pev->origin;
			else if (Params.size() >= 3) StartPos = StringToVec(Params[2]);
			thoth_mins = StartPos - thoth_delta;
			thoth_maxs = StartPos + thoth_delta;
		}
		else
		{
			//$get_tbox_abs
			thoth_mins = StringToVec(Params[1]);
			thoth_maxs = StringToVec(Params[2]);
		}

		int count = UTIL_EntitiesInBox(pList, 255, thoth_mins, thoth_maxs, FL_CLIENT | FL_MONSTER);

		//ALERT( at_aiconsole, "Searching through %i ents\n", count );

		for (int i = 0; i < count; i++)
		{
			if ((int)thoth_token_string.len() > str_limit) continue;

			pEnt = pList[i];

			if (!pEnt->IsAlive()) continue; //dont count anything dead

			if (m.pScriptedEnt->entindex() == pEnt->entindex()) continue; //dont count self

			if (!stricmp("player", Name) && pEnt->IsPlayer())
			{
				ent_str = EntToString(pEnt);
				int total_len = thoth_token_string.len() + ent_str.len();
				if (total_len < str_limit)
				{
					thoth_token_string += ent_str;
					thoth_token_string += ";";
				}
			}

			if (!stricmp("monster", Name) && pEnt->IsMSMonster() && !pEnt->IsPlayer())
			{
				ent_str = EntToString(pEnt);
				int total_len = thoth_token_string.len() + ent_str.len();
				if (total_len < str_limit)
				{
					thoth_token_string += ent_str;
					thoth_token_string += ";";
				}
			}

			if (!stricmp("any", Name) && !pEnt->IsMSItem())
			{
				ent_str = EntToString(pEnt);
				int total_len = thoth_token_string.len() + ent_str.len();
				if (total_len < str_limit)
				{
					thoth_token_string += ent_str;
					thoth_token_string += ";";
				}
			}

			if (!strcmp("enemy", Name))
			{
				CMSMonster* pMonster = (CMSMonster*)pEnt;
				int my_relate = pMonster->IRelationship(m.pScriptedEnt);
				switch (my_relate)
				{
				case -3:
				case -4:
				case -5:
				{
					if (pMonster->m_Race)
					{
						ent_str = EntToString(pEnt);
						int total_len = thoth_token_string.len() + ent_str.len();
						if (total_len < str_limit)
						{
							thoth_token_string += ent_str;
							thoth_token_string += ";";
						}
					}
					break;
				}
				}
			}

			if (!strcmp("ally", Name))
			{
				CMSMonster* pMonster = (CMSMonster*)pEnt;
				int my_relate = pMonster->IRelationship(m.pScriptedEnt);
				if (my_relate == 1 && pMonster->m_Race)
				{
					ent_str = EntToString(pEnt);
					int total_len = thoth_token_string.len() + ent_str.len();
					if (total_len < str_limit)
					{
						thoth_token_string += ent_str;
						thoth_token_string += ";";
					}
				}
			}
		}
		if (thoth_token_string.len() > 0) return thoth_token_string.c_str();
		else return "none";
	}
#endif

	return FullName;
}

//$get_under_sky(<origin>) - returns "1" if point is under sky texture
//- similar issues as found with $get_sky_height, due to bsp structure.
//- priority: high, scope: shared
msstring CScript::ScriptGetter_GetUnderSky(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Thothie AUG2010_03
	//$get_under_sky(<origin>) - returns "1" if point is under sky texture
	//priority: high, scope: shared
	if (Params.size() >= 1)
	{
		bool FoundSky = UnderSky(StringToVec(GetScriptVar(Params[0])));
		return FoundSky ? "1" : "0";
	}
	else
		return FullName;
}

//$int(<var>)
//- convert to integer (flattens)
//- priority: moderate, scope: shared
msstring CScript::ScriptGetter_Int(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//convert to integer
	//priority: moderate, scope: shared
	if (Params.size() >= 1)
	{
		//Return = atoi(Params[0]);
		return msstring() + atoi(Params[0]);
	}
	else return "0";
}

//$item_exists(<target>,<item_name>,[flags:nohands|noworn|notinpack|remove|name|id])
//- Sees if item exists in target's inventory (may or may not work on NPCs - untested). Flags:
//-- remove - remove first instance of item found.
//-- name - instead of searching, returns the english friendly display name of the requested item. (Apparently by spawning a copy of it - which is - ewww. This should be a seperate function, and there should really be a better way.)
//-- nohands - dont check hands.
//-- noworn - dont check gear items.
//-- notinpack - dont check packs.
//-- (( todo: partialname - partial matches are positive.))
//-- id returns the id of the first matching item found (untested)
//- priority: low, scope: server
msstring CScript::ScriptGetter_ItemExists(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
#ifdef VALVE_DLL
	//priority: low, scope: server
	//( <target>, <itemname>, <flags> )
	//Thothie OCT2015_18 - something wrong in here, always returning item name instead of 0|1, added various efforts to ensure fix
	msstring& Name = Params[1];
	CBaseEntity* pEntity = m.pScriptedEnt ? m.pScriptedEnt->RetrieveEntity(Params[0]) : NULL;

	if (pEntity && pEntity->IsPlayer())
	{
		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
		getitem_t ItemDesc;
		clrmem(ItemDesc);
		msstring Flags = (Params.size() > 2) ? Params[2].c_str() : "0";

		ItemDesc.Name = Params[1];
		if (Flags != "0")
		{
			if (Flags.contains("nohands")) ItemDesc.IgnoreHands = true;
			if (Flags.contains("noworn")) ItemDesc.IgnoreWornItems = true;
			if (Flags.contains("notinpack")) ItemDesc.IgnoreInsideContainers = true;
			if (Flags.contains("partialname")) ItemDesc.CheckPartialName = true;
			if (Flags.contains("onlyhands")) //MIB MAR2008a
			{
				ItemDesc.IgnoreHands = false;
				ItemDesc.IgnoreWornItems = true;
				ItemDesc.IgnoreInsideContainers = true;
			}
		}

		bool Found = pPlayer->GetItem(ItemDesc);

		//Print( "XEBUG: $itemexists %s %s (flags %s)\n", Params[1].c_str(), Found ? "1" : "0", Flags.c_str() );

		if (Found)
		{
			if (Flags.contains("remove"))
			{
				//Thothie hack - attempting to remove items remotely (can't remove items from players with removeitem command, so need this.)
				//- this be a hack, since we're using a return to do a method, one side effect is item doesn't vanish proper on the client
				pPlayer->RemoveItem(ItemDesc.retItem);
				return "0";
			}
			else if (Flags.contains("name") && !Flags.contains("partial"))
			{
				// MiB MAR2019_13 - Clean up of "temporary items" in favor of global table
				return ItemDesc.retItem->DisplayName();
			}
			else if (Flags.contains("id"))
			{
				//untested
				return EntToString(ItemDesc.retItem);
			}

			return "1";
		}
	}
#endif
	return "0";
}

//$lcase(<var|string>)
//- Returns lower case of string or variable
//- priority: high, scope: shared
msstring CScript::ScriptGetter_LCase(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Make lower case - Thothie
	//priority: high, scope: shared
	if (Params.size() >= 1)
	{
		char toconv[256];
		strncpy(toconv, Params[0], sizeof(toconv));
		return msstring(_strlwr(toconv));
	}
	else
	{
		return "0";
	}
}

//$len(<var|string>)
//- Returns length of string or variable.
//- Beware that unset vars will return their litteral string length.
//- priority: high, scope: shared
msstring CScript::ScriptGetter_Len(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB Feb2008a
	//$len(<var>) - Returns the string length of <var>
	//priority: high, scope: shared
	msstring Return;
	if (Params.size() >= 1)
		RETURN_INT(Params[0].len())
	else
		return "0";
}

//$inrange(<var>,<low>,<high>)
//- Tests if var is in range from <low> to <high> (so you can kinda do "if ( x > y and x < z)"
//$inset_string(<var>,<params...>)
//$inset_num(<var>,<params...>)
//- Checks if <var> equals any of <params...>, stringwise or mathmatically
//-- These both are intended to simulate "if and/or" conditionals
//- Thothie APR2016_15
//- priority: medium, scope: shared
msstring CScript::ScriptGetter_inrange(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	if (ParserName.starts_with("$inrange"))
	{
		if (Params.size() < 3)
		{
			ALERT(at_console, "Warning: Script: %s, - $inrange(<var>,<low>,<high>) not enough paramters.\n", m.ScriptFile.c_str());
			return "0"; //not enough params
		}
		if (atof(Params[0].c_str()) >= atof(Params[1].c_str()) && atof(Params[0].c_str()) <= atof(Params[2].c_str()))
		{
			return "1";
		}
		else
		{
			return "0";
		}
	}
	else if (ParserName.starts_with("$inset_string"))
	{
		if (Params.size() < 2)
		{
			ALERT(at_console, "Warning: Script: %s, - %s not enough paramters.\n", m.ScriptFile.c_str(), FullName.c_str());
			return "0"; //not enough params
		}
		msstring Result = "0";
		for (int i = 0; i < Params.size(); i++)
		{
			if (i > 0)
			{
				if (Params[0] == Params[i]) Result = "1";
			}
		}
		return Result;
	}
	else if (ParserName.starts_with("$inset_num"))
	{
		if (Params.size() < 2)
		{
			ALERT(at_console, "Warning: Script: %s, - %s not enough paramters.\n", m.ScriptFile.c_str(), FullName.c_str());
			return "0"; //not enough params
		}
		msstring Result = "0";
		for (int i = 0; i < Params.size(); i++)
		{
			if (i > 0)
			{
				if (atof(Params[0]) == atof(Params[i])) Result = "1";
			}
		}
		return Result;
	}
	return "0";
}

//$map_exists(<mapname>)
//- Verifies the existence of a BSP (use sans the .bsp extension).
//- priority: very low, scope: server
msstring CScript::ScriptGetter_MapExists(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Thothie - JAN2008a - attempting to verify map existance to allow future map vote system
	//- $map_exists(<mapname>) - sans the .bsp
	//priority: very low, scope: server
#ifdef VALVE_DLL
	return (IS_MAP_VALID(Params[0]) ? "1" : "0");
#endif
	return FullName;
}

msstring CScript::ScriptGetter_MathReturn(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//NOV2014_20 Thothie - Math macro
	//$math("add|multiply|subtract|divide",<amt|var>,<amt|var>)
	//returns result, does not modify vars
	//todo: add more functions
	//priority: high, scope: shared
	msstring Return;
	if (Params.size() < 3)
	{
		if (Params[0] == "sqrt")
		{
			//Thothie OCT2016_15 - adding sqrt function
			//This is hacky, yet optimal, as there's only one reason we'd have this few params
			if (Params.size() > 1)
			{
				float num = atof(Params[1]);
				num = sqrt(num);
				RETURN_FLOAT(num);
			}
		}
		MSErrorConsoleText("ExecuteScriptCmd", UTIL_VarArgs("Script: %s, %s - not enough parameters!\n", m.ScriptFile.c_str(), ParserName.c_str()));
		return "0";
	}

	msstring op_type = Params[0];
	bool legit_command = false;

	//Thothie APR2016_25 - adding vectoradd/vectormultiply to math functions
	if (op_type == "vectoradd")
	{
		Vector Result;
		if (Params.size() >= 4)
		{
			if (Params[2] == "x")		Result = StringToVec(Params[1]) + Vector(atof(Params[3]), 0, 0);
			else if (Params[2] == "y") Result = StringToVec(Params[1]) + Vector(0, atof(Params[3]), 0);
			else if (Params[2] == "z") Result = StringToVec(Params[1]) + Vector(0, 0, atof(Params[3]));

			return VecToString(Result);
		}
		else
		{
			Result = StringToVec(Params[1]) + StringToVec(Params[2]);
			return VecToString(Result);
		}
	}
	else if (op_type == "vectormultiply")
	{
		Vector Result;
		if (Params.size() >= 4)
		{
			if (Params[2] == "x")		Result = VecMultiply(StringToVec(Params[1]), Vector(atof(Params[3]), 0, 0));
			else if (Params[2] == "y") Result = VecMultiply(StringToVec(Params[1]), Vector(0, atof(Params[3]), 0));
			else if (Params[2] == "z") Result = VecMultiply(StringToVec(Params[1]), Vector(0, 0, atof(Params[3])));
			return VecToString(Result);
		}
		else
		{
			if (isdigit(Params[2][0])) Result = StringToVec(Params[1]) * atof(Params[2]);
			else Result = VecMultiply(StringToVec(Params[1]), StringToVec(Params[2]));
			return VecToString(Result);
		}
	}

	float maths1 = atof(Params[1].c_str());
	float maths2 = atof(Params[2].c_str());
	int imaths1 = (int)maths1;
	int imaths2 = (int)maths2;

	float end_result;

	if (op_type.starts_with("add"))
	{
		legit_command = true;
		end_result = maths1 + maths2;
	}
	else if (op_type.starts_with("subtract"))
	{
		legit_command = true;
		end_result = maths1 - maths2;
	}
	else if (op_type.starts_with("multiply"))
	{
		legit_command = true;
		end_result = (maths1 * maths2);
	}
	else if (op_type.starts_with("divide"))
	{
		legit_command = true;
		if (maths1 == 0 || maths2 == 0) end_result = 0; //ya know, ya'd think after all these decades, computers would stop trying to do this automatically
		else end_result = (maths1 / maths2);
	}
	else if (op_type.starts_with("intdivide"))
	{
		legit_command = true;
		if (imaths1 == 0 || imaths2 == 0) end_result = 0; //ya know, ya'd think after all these decades, computers would stop trying to do this automatically
		else end_result = (imaths1 / imaths2);
	}
	else if (op_type.starts_with("mod"))
	{
		legit_command = true;
		if (imaths1 == 0 || imaths2 == 0) end_result = 0; //ya know, ya'd think after all these decades, computers would stop trying to do this automatically
		else end_result = (imaths1 % imaths2);
	}
	else if (op_type.starts_with("capvar")) //Thothie SEP2019_12 - inline capvar
	{
		//$math(capvar,<in>,<floor>,<ceiling>)
		//2,1,3 returns 2 - 2,1,1.5 returns 1.5
		legit_command = true;
		if (Params.size() >= 4)
		{
			end_result = atof(Params[1]);
			float maths_mincap = atof(Params[2]);
			float maths_maxcap = atof(Params[3]);
			if (end_result < maths_mincap) end_result = maths_mincap;
			else if (end_result > maths_maxcap) end_result = maths_maxcap;
		}
		else
		{
			end_result = 0;
			MSErrorConsoleText("ExecuteScriptCmd", UTIL_VarArgs("Script: %s, %s - not enough parameters for $math capvar!\n", m.ScriptFile.c_str(), ParserName.c_str()));
		}
	}

	if (!legit_command)
	{
		MSErrorConsoleText("ExecuteScriptCmd", UTIL_VarArgs("Script: %s, %s - %s is an invalid operation!\n", m.ScriptFile.c_str(), ParserName.c_str(), op_type.c_str()));
		return "0";
	}

	RETURN_FLOAT(end_result);
}

//$mid(<var>,<start>,[length])
//- Returns a substring of <var> starting at <start> and going <length> characters
//- priority: moderate, scope: shared
msstring CScript::ScriptGetter_Mid(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB Feb2008a
	//$mid(<var>,<start>,[length]) - Returns a substring of <var> starting at <start> and going <length> characters
	//priority: moderate, scope: shared
	//Thothie DEC2017_10
	//- If [length] is not provided, return remainder of string to end
	msstring Return;
	if (Params.size() >= 3)
	{
		int start = atoi(Params[1]);
		int length = atoi(Params[2]);
		return Params[0].substr(start, length);
	}
	else if (Params.size() >= 2)
	{
		int start = atoi(Params[1]);
		int length = Params[0].len() - atoi(Params[1]);
		return Params[0].substr(start, length);
	}

	return "0";
}

//$min(<1>,[2]..[n]) - returns the lowest number in a series of floats
//$max(<1>,[2]..[n]) - returns the highest number in a series of floats
//- priority: very low, scope: shared
msstring CScript::ScriptGetter_MinMax(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB FEB2010_05 - Return the max/min value of a given list
	//$max(<1>,[2]..[n])
	//$min(<1>,[2]..[n])
	//Thothie NOV2014 comments: We've yet to use this for anything, but nice to have, I suppose
	//priority: very low, scope: shared
	msstring Return;
	if (Params.size() >= 1)
	{
		float best = atof(Params[0].c_str());
		bool max = ParserName == "$max";
		for (int i = 0; i < Params.size(); i++)
			best = max ? max(best, atof(Params[i].c_str())) : min(best, atof(Params[i].c_str()));
		RETURN_FLOAT(best);
	}

	return FullName;
}

//$neg(<value>)
//- Returns negative of <value>
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_Neg(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Make negative
	//priority: very high, scope: shared
	msstring Return;
	if (Params.size() >= 1)
		RETURN_FLOAT((-atof(Params[0])))
	else return "0";
}

//$num(<string>,[extras])
//- Strip non numericals from <string> with the exception of [extras]
//- eg. $num(SOME_INPUT,.)
//- priority: very low, scope: shared
msstring CScript::ScriptGetter_Num(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Strip non number bits Thothie FEB2008a
	//$num(<string>,[extras]) - strips non-digits except characters found in [extras]
	//eg. $num(SOME_INPUT,".")
	//priority: very low, scope: shared
	msstring Return;
	if (Params.size() >= 1)
	{
		//yes, this is a shit way of doing this, but I'm in a rush...
		msstring num_conv = Params[0];
		msstring out_str;
		int in_length = num_conv.len();
		char* cc;
		bool char_legit;
		msstring extra_list;
		bool check_extras = false;
		if (Params.size() > 1)
		{
			extra_list = Params[1];
			check_extras = true;
		}
		for (int i = 0; i <= in_length; i++)
		{
			char_legit = false;
			cc = num_conv.substr(i, 1).c_str();
			if (isdigit(cc[0]) > 0) char_legit = true;
			if (check_extras)
			{
				if (extra_list.contains(cc)) char_legit = true;
			}
			if (char_legit) out_str += cc;
		}
		return out_str.c_str();
	}
	else
	{
		return "0";
	}
}

//$quote(<string>) - put a string in dbl "quotes".
//- If used with no params, eg. $quote(), returns a single dbl quote (")
//- priority: high, scope: shared
msstring CScript::ScriptGetter_Quote(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Encase string in quotes Thothie FEB2008a (or return a ")
	//priority: high, scope: shared
	if (Params.size() >= 1)
	{
		msstring out_str = "\"";

		for (int i = 0; i < Params.size(); i++)
		{
			if (i) out_str += " ";
			out_str += Params[i];
		}
		out_str += "\"";
		return out_str;
	}
	else
	{
		return "\"";
	}
}

//$rand(<min>,<max>) returns random integer from <min> to <max>
//$randf(<min>,<max>) returns random float from <min> to <max> (two digit float percision).
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_Rand(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//priority: very high, scope: shared
	msstring Return;
	if (ParserName[5] == 'f') //float version
		RETURN_FLOAT(RANDOM_FLOAT(atof(Params[0]), atof(Params[1])))
	else  //int version
		RETURN_INT(RANDOM_LONG(atoi(Params[0]), atoi(Params[1])))
}

//$relpos(<vec_ofs>)
//$relpos(<vec_angles>,<vec_ofs>)
//- Returns vector with relative position
//- Second variant always returns relative to vec(0,0,0), for use with vectoradd
//- <vec_angles> are pitch, yaw, roll
//- <vec_ofs> are right, forward, up
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_RelPos(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//priority: very high, scope: shared
	msstring PosString;
	Vector Angle, StartPos;
	msstring Return;
	if (m.pScriptedEnt && Params.size() >= 1)
	{
		if (Params[0][0] != '(')	//(x,y,z)
		{
			if (m.pScriptedEnt)
			{
				StartPos = (m.pScriptedEnt->pev->modelindex ? m.pScriptedEnt->Center() : m.pScriptedEnt->pev->origin),
					Angle = m.pScriptedEnt->pev->angles;
				PosString = FullName.substr(7);
			}
		}
		else	//(pitch,yaw,roll),(x,y,z)
		{
			Angle = StringToVec(Params[0]);
			PosString = Params[1];
			StartPos = g_vecZero;
		}

		Vector vPosStr = StringToVec(PosString);
		Vector vRelPos = GetRelativePos(Angle, vPosStr);
		Vector vPos = StartPos + vRelPos;
		//sprintf( cReturn, "(%.2f,%.2f,%.2f)", Pos.x, Pos.y, Pos.z );
		RETURN_VECTOR(vPos)
	}
	else return "0";
}

//$relvel(<vec_ofs>)
//$relvel(<vec_angles>,<vec_ofs>)
//- Returns adjusted relative velocity.
//- <vec_angles> are pitch, yaw, roll
//- <vec_ofs> are right, forward, up
//- if client side, treats base velocity as 0
//- priority: very high, scope: shared (ish)
msstring CScript::ScriptGetter_RelVel(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//priority: very high, scope: shared (ish)
	msstring Return;
	if (m.pScriptedEnt && Params.size() >= 1)
	{
		Vector Angle, RelVel;
		if (Params[0][0] != '(')	//(x,y,z)
		{
			Angle = ((m.pScriptedEnt->IsPlayer() || FBitSet(m.pScriptedEnt->pev->flags, FL_FLY | FL_SWIM)) ? m.pScriptedEnt->pev->v_angle : m.pScriptedEnt->pev->angles);
			RelVel = StringToVec(&FullName[7]);
		}
		else	//(pitch,yaw,roll),(x,y,z)
		{
			Angle = StringToVec(Params[0]);
			RelVel = StringToVec(Params[1]);
		}

		Vector vForward, vRight, vUp;
		EngineFunc::MakeVectors(Angle, vForward, vRight, vUp);

		Vector Final = vRight * RelVel.x + vForward * RelVel.y + vUp * RelVel.z;

		RETURN_VECTOR(Final)
	}
	else return "0";
}

// $shape_cylinder(<origin|entity>,<radius>[,pos_height[,neg_height]])
msstring CScript::ScriptGetter_ShapeCylinder(
	msstring& FullName
	, msstring& ParserName
	, msstringlist& Params
)
{
	if (Params.size() > 1)
	{
		Vector vOrigin = DetermineOrigin(Params[0]);
		float vRadius = atof(Params[1]);
		float vPosZ = Params.size() > 2 ? atof(Params[2]) : 0;
		float vNegZ = Params.size() > 3 ? atof(Params[3]) : 0;

		CCylinderFilter vFilter(vOrigin, vRadius, vPosZ, vNegZ);
		return vFilter.AsString();

	}
	return FullName;
}

// $shape_rect(<min_point>,<max_point>)
//      If 2D vectors or Zs are equal, doesn't check z
// $shape_rect(<origin|entity>,<x_size>[,y_size[,z_size])
//      If only x_size is provided, a cube with x_size sides
//      If only x_size and y_size are provided, a square that doesn't check z
msstring CScript::ScriptGetter_ShapeRect(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	if (Params.size() > 1)
	{
		Vector vOrigin = DetermineOrigin(Params[0]);
		float vHalfX;
		float vHalfY;
		float vHalfZ;

		if (Params[1][0] == '(')
		{
			Vector vPointMin = vOrigin;
			Vector vPointMax = StringToVec(Params[1]);
			Vector vDelta = vPointMax - vPointMin;
			vHalfX = vDelta.x / 2;
			vHalfY = vDelta.y / 2;
			vHalfZ = vDelta.z / 2;
			vOrigin = (vPointMin + vPointMax) * 0.5;
		}
		else
		{
			vHalfX = fabs(atof(Params[1]) / 2);
			vHalfZ = 0;

			if (Params.size() == 2)
			{
				vHalfY = vHalfX;
				vHalfZ = vHalfX;
			}
			else
				if (Params.size() == 3)
				{
					vHalfY = fabs(atof(Params[2]));
				}
				else
				{
					vHalfY = fabs(atof(Params[2]));
					vHalfZ = fabs(atof(Params[3]));
				}
		}

		CRectangleFilter vFilter(vOrigin, vHalfX, vHalfY, vHalfZ);
		return vFilter.AsString();
	}
	return FullName;
}

// $shape_sphere(<origin|entity>,<radius>)
msstring CScript::ScriptGetter_ShapeSphere(
	msstring& FullName
	, msstring& ParserName
	, msstringlist& Params
)
{
	if (Params.size() > 1)
	{
		Vector                          vOrigin = DetermineOrigin(Params[0]);
		float                           vRadius = atof(Params[1]);

		CSphereFilter                   vFilter(vOrigin, vRadius);
		return vFilter.AsString();
	}
	return FullName;
}

//$replace(<string>,<replace_string>,<start>) - Overwrites a section of <string> with <replace_string> starting at <start>
//$insert(<string>,<replace_string>,<start>) - Same, but inserts instead of overwrite
//- priority: moderate, scope: shared
msstring CScript::ScriptGetter_ReplaceOrInsert(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB Feb2008a
	//$replace(<var>,<replace_string>,<start>) - Overwrites a section of <var> with <replace_string> starting at <start>
	//$insert(<var>,<replace_string>,<start>) - Same, but doesn't overwrite, obviously.
	//$subst(<var>,<search_word>,<replace_word>) - Substitutes <search_word> with <replace_word> in <var>
	//priority: moderate, scope: shared
	msstring Return;
	if (Params.size() >= 3)
	{
		msstring var = Params[0];
		msstring rstr = Params[1];
		int start = atoi(Params[2]);

		msstring temp = var.substr(0, start);
		temp += rstr;
		if (ParserName == "$replace")
		{
			if (temp.len() < var.len())
				temp += var.substr(temp.len());
		}
		else if (ParserName == "$insert")
		{
			temp += var.substr(start);
		}
		else if (ParserName == "$subst")
		{
			//Thothie DEV2017_05 (updated DEV2017_17 for use with substitute msstring) (failing, can't figure out how to define substitute proper)
			/*
			if ( !Params[0].len() || !Params[1].len() || !Params[2].len() ) return "0"; //got a bogus param

			if ( Params[0].contains(Params[1]) )
			{
				temp = Params[0]; //might not need this, but wanna make sure something is in here
				temp.substitute(Params[0],Params[1],Params[2]);
				if ( temp.contains(Params[1]) )
				{
					int i;
					for (i = 0; i < 100; i++) //this limits us to 100 replacements, but while was just locking up
					{
						temp.substitute(temp,Params[1],Params[2]);
						if ( !temp.contains(Params[1]) ) break;
					}
				}
			}
			else
			{
				//search string not found, return untouched
				return Params[0].c_str();
			}
			*/

			//old method
			if (Params[0].contains(Params[1]))
			{
				if (!Params[0].starts_with(Params[1]))
				{
					//this will only substitute one instance
					int tstring_up_to_len = 0;
					temp = Params[0].thru_substr(Params[1]); //$string_upto
					tstring_up_to_len = temp.len();
					temp += Params[2]; //add word
					temp += Params[0].substr(tstring_up_to_len + Params[1].len()); //$string_from.
				}
				else
				{
					//string is at start, different method required
					temp = Params[2]; //start with word
					temp += Params[0].substr(Params[1].len()); //$string_from.
				}
				if (temp.contains(Params[1]))
				{
					msstring temp2;
					int tstring_up_to_len = 0;
					int i;
					for (i = 0; i < 100; i++) //this limits us to 100 replacements, but while was just locking up
					{
						tstring_up_to_len = 0;
						temp2 = temp;

						temp2 = temp.thru_substr(Params[1]); //$string_upto
						tstring_up_to_len = temp2.len();
						temp2 += Params[2]; //add word
						temp2 += temp.substr(tstring_up_to_len + Params[1].len()); //$string_from.

						temp = temp2;
						if (!temp.contains(Params[1])) break;
					}
				}
			}
			else
			{
				//search string not found, return untouched
				return Params[0].c_str();
			}
		}
		return temp;
	}
	else
		return "0";
}

//$scan(<shape>,<filter:any|player|monster|enemy|ally>,["traceline"],[array_name])
msstring CScript::ScriptGetter_ScanShape(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	msstring Return;
	if (Params.size() > 1)
	{
		::mslist<CEntityFilter*> vFilterList;
		try
		{
			size_t                      vParam = 0;
			::mslist<CFindEntity>         vEntityList;
			CShapeFilter* pShapeFilter = NULL;
			msstring                    vsShapeFilter = Params[vParam++];
			msstring                    vsFilterType = Params[vParam++];
			msstring                    vsCur;
			msstring                    vsArrayName;
			bool                        bTraceLine = false;
			msscriptarray* pArray = NULL;

			// Process optional filters
			while (vParam < Params.size())
			{
				vsCur = Params[vParam++];
				if (vsCur == "traceline")
				{
					bTraceLine = true;
				}
				else
				{
					if (!pArray)
					{
						vsArrayName = vsCur;
						pArray = m.pScriptedEnt->GetScriptedArray(vsArrayName, true);
						pArray->clear();
					}
				}
			}
			pShapeFilter = CShapeFilter::CreateFromString(vsShapeFilter, bTraceLine);

			if (!pShapeFilter) return FullName;

			// Convert filter type into the actual entity filter
			if (vsFilterType == "any")
			{
				// Client can't filter out items, so just do literally any
#ifdef VALVE_DLL
				vFilterList.add(new CEntityTypeFilter(true, true, false));
#endif
			}
			else
				if (vsFilterType == "player")
				{
#ifdef VALVE_DLL
					vFilterList.add(new CEntityTypeFilter(false, true, false));
#else
					vFilterList.add(new CEntityTypeFilter(true));
#endif
				}
				else
					if (vsFilterType == "not_player")
					{
#ifdef VALVE_DLL
						vFilterList.add(new CEntityTypeFilter(true, false, true));
#else
						vFilterList.add(new CEntityTypeFilter(false));
#endif
					}
#ifdef VALVE_DLL
					else
						if (vsFilterType == "monster")
						{
							vFilterList.add(new CEntityTypeFilter(true, false, false));
						}
						else
							if (vsFilterType == "enemy")
							{
								vFilterList.add(new CEnemyFilter(m.pScriptedEnt));
							}
							else
								if (vsFilterType == "ally")
								{
									vFilterList.add(new CAllyFilter(m.pScriptedEnt));
								}
#endif
								else
								{
									Print("ERROR: Unknown filter type '%s'\n", vsFilterType.c_str());
									return FullName;
								}

			// More intensive than filter types, add after
			vFilterList.push_back(pShapeFilter);

			int                         vNumFound = UtilFindEntities(vEntityList
				, vFilterList
#ifdef VALVE_DLL
				, m.pScriptedEnt
#endif
			);
			for (int i = 0; i < vFilterList.size(); i++)
			{
				delete vFilterList[i];
			}
			vFilterList.clear();

			msstring                    vsIndxString;
			bool                        bFir = true;
			for (int i = 0; i < vEntityList.size(); i++)
			{
				CFindEntity             vEntity = vEntityList[i];
				if (pArray)
				{
					pArray->push_back(vEntity.AsString());
				}
				else
				{
					msstring            vsEntIndex = UTIL_VarArgs("%s%i"
						, bFir ? "" : ";"
						, vEntity.GetEntIndex()
					);
					bFir = false;
					if (vsEntIndex.len() + vsIndxString.len() > MSSTRING_MAXLEN)
					{
						break;
					}
					vsIndxString += vsEntIndex;
				}
			}
			if (pArray)
			{
				RETURN_INT(vNumFound);
			}
			else
			{
				return vsIndxString;
			}
		}
		catch (...)
		{
			for (int i = 0; i < vFilterList.size(); i++)
			{
				delete vFilterList[i];
			}
			throw;
		}
	}
	return FullName;
}

//$search_string(<var>,<search_string>,[start]) - Returns the position of <search_string> in var, or -1 if not found
//- priority: moderate, scope: shared
msstring CScript::ScriptGetter_SearchString(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB Feb2008a
	//$search_string(<var>,<search_string>,[start]) - Returns the position of <search_string> in var, or -1 if not found
	//priority: moderate, scope: shared
	msstring Return;
	if (Params.size() >= 2)
	{
		int start = Params.size() >= 3 ? atoi(Params[2]) : 0;
		RETURN_INT(Params[0].find(Params[1], start))
	}
	else
		return "0";
}

//$sort_entlist(<ent_token_list>,<sort_type:range|hp|maxhp|mp|maxmp>,[range_origin])
//- sorts entities in a token list
//- priority: high, scope: server
msstring CScript::ScriptGetter_SortEntList(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB Apr2008a
	//$sort_entlist(<ent_token_list>,<var_to_sort_by>)
	//Can sort by distance, hp, maxhp, etc (full list below)
	//priority: high, scope: server
	msstring Return;
	if (Params.size() >= 2)
	{
		static msstringlist Tokens;
		Tokens.clearitems();
		TokenizeString(Params[0], Tokens);

		msstring SortedList;
		msstring compParam = Params[1];

		if (Tokens.size() == 1) return Params[0].c_str(); //Thothie APR2016_23 - smashes if only given one entity otherwise

		int size = (signed)Tokens.size();
		for (int i = 0; i < size; i++)
		{
			int curIdx = 0;
			float curVal = 0;

			CMSMonster* pZeroEnt = (m.pScriptedEnt ? (CMSMonster*)m.pScriptedEnt->RetrieveEntity(Tokens[0]) : NULL);
			if (pZeroEnt && pZeroEnt->pev && m.pScriptedEnt->pev)
			{
				if (compParam == "range")
				{
					if (Params.size() == 2)
						curVal = (pZeroEnt->pev->origin - m.pScriptedEnt->pev->origin).Length();
					else
						curVal = (pZeroEnt->pev->origin - StringToVec(Params[2])).Length();
				}
				else if (compParam == "hp") curVal = pZeroEnt->m_HP;
				else if (compParam == "maxhp") curVal = pZeroEnt->m_MaxHP;
				else if (compParam == "mp") curVal = pZeroEnt->m_MP;
				else if (compParam == "maxmp") curVal = pZeroEnt->m_MaxMP;

				for (int j = 0; j < (Tokens.size() - 1); j++)
				{
					float compVal = 0;
					CMSMonster* pCurEnt = (CMSMonster*)m.pScriptedEnt->RetrieveEntity(Tokens[j + 1]);
					if (pCurEnt && pCurEnt->pev)
					{
						if (compParam == "range")
						{
							if (Params.size() == 2)
								compVal = (pCurEnt->pev->origin - m.pScriptedEnt->pev->origin).Length();
							else
								compVal = (pCurEnt->pev->origin - StringToVec(Params[2])).Length();
						}
						else if (compParam == "hp") compVal = pCurEnt->m_HP;
						else if (compParam == "maxhp") compVal = pCurEnt->m_MaxHP;
						else if (compParam == "mp") compVal = pCurEnt->m_MP;
						else if (compParam == "maxmp") compVal = pCurEnt->m_MaxMP;
					}

					if (compVal < curVal)
					{
						curIdx = j;
						curVal = compVal;
					}
				}
			}

			SortedList += Tokens[curIdx] + ";";
			Tokens.erase(curIdx);
		}

		return SortedList;
	}
	else
		RETURN_INT(-1);
}

//$stradd(<string...>,<string...>,<string...>)
//- Concenates strings - beware of various limitations resulting from $function usage.
//- priority: moderate, scope: shared
msstring CScript::ScriptGetter_StrAdd(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Add strings together
	//priority: moderate, scope: shared
	msstring Return;
	for (int i = 0; i < Params.size(); i++)
		Return += Params[i];

	return Return;
}

//$right(<var>,<length>)
//- Returns a substring of size <length> from the right side of <var>
//- priority: moderate, scope: shared
msstring CScript::ScriptGetter_StringRightOrLeft(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB Feb2008a
	//$right(<var>,<length>) - Returns a substring of size <length> from the right side of <var>
	//priority: moderate, scope: shared
	if (Params.size() >= 2)
	{
		int length = atoi(Params[1]); //MAR2008a Thothie - the -1 here fubared this
		length = max(0, min(length, (signed)Params[0].len()));
		/*Print( "***** %s(%s,%s) == [%s]\n"	, ParserName.c_str()
											, Params[0].c_str()
											, Params[1].c_str()
											, ParserName == "$left" ? Params[0].substr(0,length).c_str() : Params[0].substr(Params[0].len() - length)
											);*/
		if (ParserName == "$left")
			return Params[0].substr(0, length);
		else
			return Params[0].substr(Params[0].len() - length);
	}
	else
		return "0";
}

//$string_from(<string>,<search_for>,[start]) - Return everything in <string> after <search_for>
//$string_upto(<string>,<search_for>,[start]) - Return everything in <string> up to <search_for>
//- priority: low, scope: shared
msstring CScript::ScriptGetter_StringUpToOrFrom(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//MiB Feb2008a
	//priority: low, scope: shared
	//$string_upto(<search>,<string>,[start]) - Return everything up to <search>
	//$string_from(<search>,<string>,[start]) - Return everything after <search>
	if (Params.size() >= 2)
	{
		int start = Params.size() >= 3 ? atoi(Params[2]) : 0;
		msstring temp = Params[0].thru_substr(Params[1], start);
		if (temp != Params[0]) //This is the case if the search string is at the beginning of the string
		{
			if (ParserName == "$string_upto")
				return temp;
			else
			{
				//Thothie MAR2016_05
				//- This was failing when $string_from should return the last character in a string
				//- eg. "$string_from('item_name:2-7','-')" returns ""
				//- original line was: if( temp.len() + Params[1].len() >= Params[0].len() -1 )
				if (temp.len() + Params[1].len() >= Params[0].len())
				{
					//Print("XEBUG: NULL RETURN\n");
					return "";
				}
				return Params[0].substr(temp.len() + Params[1].len());
			}
		}
		else if (Params[0].starts_with(Params[1]))
		{
			if (ParserName == "$string_upto")
				return "";
			return Params[0].substr(Params[1].len());
		}
	}

	return "0";
}
//$timestamp([suffix])
//- Return local time as per Window date format, followed by suffix
//- eg. $timestamp(>) returns: "Fri Feb 01 02:48:38 2008>" in US.
//- actual time stamp format may vary with Windows region settings.
//- priority: moderate, scope: shared
msstring CScript::ScriptGetter_TimeStamp(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	// $timestamp MiB Feb2008a
	// Returns a timestamp in the format "Thu Jan 03 17:40:10
	// - thothie notes: time format will vary with region
	// - for logging
	//priority: moderate, scope: shared
	time_t Time;
	time(&Time);
	msstring_ref TimeString = ctime(&Time);
	msstring out_timestr = TimeString;
	out_timestr = out_timestr.substr(0, out_timestr.len() - 1);
	if (Params.size() > 0) out_timestr += Params[0];
	if (TimeString)
		return out_timestr.c_str();
	else
		return "NO_TIME_AVAILABLE";
}

//$ucase(<string>)
//- Converts <string> to upper case.
//- priority: very low, scope: shared
msstring CScript::ScriptGetter_UCase(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Make upper case - Thothie
	//priority: very low, scope: shared
	if (Params.size() >= 1)
	{
		char toconv[256];
		strncpy(toconv, Params[0], sizeof(toconv));
		return _strupr(toconv);
	}
	else
	{
		return "0";
	}
}

//$vec.[x|y|z|pitch|yaw|roll](<vector>)
//- Returns component of <vector>.
//$vec(<float>,<float>,<float>)
//- Assembles vars or floats into proper vectors.
//- priority: very high, scope: shared
msstring CScript::ScriptGetter_Vec(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	msstring Return = FullName;

	//priority: very high, scope: shared
	if (ParserName == "$vec")	//Create Vector
	{
		if (Params.size() >= 3)
			Return = msstring("(") + Params[0] + "," + Params[1] + "," + Params[2] + ")";
		else return "0";
		return Return;
	}
	//priority: very high, scope: shared
	else if (ParserName.starts_with("$vec."))	//Get vector coord
	{
		msstring CoordName = ParserName.substr(5).thru_char("(");
		if (Params.size() == 1)
		{
			Vector Vec = StringToVec(Params[0]);
			if (CoordName == "x" || CoordName == "pitch") RETURN_FLOAT(Vec.x)
			else if (CoordName == "y" || CoordName == "yaw") RETURN_FLOAT(Vec.y)
			else if (CoordName == "z" || CoordName == "roll") RETURN_FLOAT(Vec.z)
		}
		else if (Params.size() >= 3)
		{
			Return = "0";
			if (CoordName == "x" || CoordName == "pitch") Return = Params[0];
			else if (CoordName == "y" || CoordName == "yaw") Return = Params[1];
			else if (CoordName == "z" || CoordName == "roll") Return = Params[2];
			return Return;
		}
	}

	return Return;
}

//$veclen(<vector>)
//$veclen2D(<vector>)
//- Docs say this returns length of a vector...
//- But given that this only takes a single coordinate, not really sure how you get the "length" of a point?
//- Doesn't seem to be used in any of the existing scripts.
//- priority: low, scope: shared
msstring CScript::ScriptGetter_VecLen(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	// Length of the vector
	// Length of the vector (2D)
	//priority: low, scope: shared
	// MiB 29NOV_2014 Combined these two commands
	msstring Return;
	if (Params.size() >= 1)
	{
		Vector Start = StringToVec(Params[0]);
		RETURN_FLOAT(ParserName == "$veclen" ? Start.Length() : Start.Length2D());
	}
	else return "0";
}

//$within_box(<target|origin>,<testbox_org|0>,<testbox_mins>,<testbox_maxs>)
//- Tests if <target>'s bounding box intersects with test box area, or if <origin> is inside test box.
//- If testbox_org is zero, origin is calced from mins/maxs, and abs min/max must be supplied.
//- When client side, <target> accepts idx but cannot currently extract bounding boxes from client side entities
//- priority: moderate, scope: shared
msstring CScript::ScriptGetter_WithinBox(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	//Thothie NOV2014_08
	//$within_box(<target|origin>,<testbox_org|0>,<testbox_mins>,<testbox_maxs>)
	//tests if target's bounding box intersects with test box area, or if origin is inside test box
	//if testbox_org is zero, origin is calced from mins/maxs, and abs min/max must be supplied
	//priority: moderate, scope: shared
	Vector tOrg; //target origin
	Vector tMins; //target box absmin
	Vector tMaxs; //target box absmax
	Vector bOrg = StringToVec(Params[1]); //test box origin
	Vector bMins = StringToVec(Params[2]); //test box mins
	Vector bMaxs = StringToVec(Params[3]); //test box maxes
	if (bOrg == g_vecZero)
	{
		//g_vecZero assumes a brush box from which the absmins/absmaxes were pulled, find center
		bOrg.x = bMins.x + ((bMaxs.x - bMins.x) * 0.5);
		bOrg.y = bMins.y + ((bMaxs.y - bMins.y) * 0.5);
		bOrg.z = bMins.z + ((bMaxs.z - bMins.z) * 0.5);
		//ALERT( at_console, "Warning: $within_box test origin is 0 - attempting to find center via boundries (%f,%f,%f) and assuming absmins/absmaxs\n",bOrg.x,bOrg.y,bOrg.z );
	}
	else
	{
		bMins = bOrg - StringToVec(Params[2]);
		bMaxs = bOrg + StringToVec(Params[3]);
	}

	if (!Params[0].starts_with("("))
	{
#ifdef VALVE_DLL
		CBaseEntity* pEntity = m.pScriptedEnt ? m.pScriptedEnt->RetrieveEntity(Params[0]) : NULL;
#else
		cl_entity_t* pEntity = gEngfuncs.GetEntityByIndex(atoi(Params[0]));
#endif
		if (pEntity)
		{
#ifdef VALVE_DLL
			tOrg = pEntity->pev->origin;
			tMins = pEntity->pev->absmin;
			tMaxs = pEntity->pev->absmax;
#else
			tOrg = pEntity->origin;
			tMins = pEntity->origin;
			tMaxs = pEntity->origin;
			//not legit, for some reason
			//tMins = tOrg - pEntity->model->mins;
			//tMaxs = tOrg + pEntity->model->maxs;
#endif
		}
		else
		{
			ALERT(at_console, "Warning: $within_box got invalid entity");
			return "-1";
		}
	}
	else
	{
		tOrg = StringToVec(Params[0]);
		tMins = tOrg;
		tMaxs = tOrg;
	}

	if (bMins.x > tMaxs.x ||
		bMins.y > tMaxs.y ||
		bMins.z > tMaxs.z ||
		bMaxs.x < tMins.x ||
		bMaxs.y < tMins.y ||
		bMaxs.z < tMins.z)
	{
		return "0";
	}
	else
	{
		return "1";
	}
}

/*
$filehash(<string|file_path>, <string|crc>)
- Get a file's hash if the file exist.
- Compares hashes if second parameter is supplied.
- Should return an int. Returns 0 if no parameter specified.
- priority: low, scope: shared
*/
msstring CScript::ScriptGetter_FileHash(msstring& FullName, msstring& ParserName, msstringlist& Params)
{
	msstring fcrc;

	//Wishbone MAR2016 - File hash.
	if (Params.size() >= 1)
	{
		char cfileName[MAX_PATH];
#ifdef VALVE_DLL
		GET_GAME_DIR(cfileName);
#else
		strncpy(cfileName, gEngfuncs.pfnGetGameDirectory(), sizeof(cfileName));
#endif

		msstring filePath = cfileName;
		filePath += "/";
		filePath += Params[0];
		ifstream file;
		file.open(filePath);
		if (file.is_open())
		{
			_snprintf(fcrc, MSSTRING_SIZE, "%i", GetFileCheckSum(filePath));
			file.close();
		}
		else {
			file.close();
			return "-1";
		}

		if (Params.size() == 2)
		{
			if (fcrc == Params[1])
			{
				return "1"; //true
			}
			else {
				return "0";
			}
		}
		else {
			return fcrc;
		}
	}
	else {
		return "0";
	}
}

msstring_ref CScript::GetVar(msstring_ref pszText)
{
	static char cReturn[1024];

	msstring FullName = pszText;
	msstring Name;
	static msstring Return;
	Return[0] = 0;
	if (m.CurrentEvent) pszText = m.CurrentEvent->GetLocal(pszText);

	if (FullName[0] == '\'' && FullName[FullName.len() - 1] == '\'')	//String literal.  Don't try to resolve it
	{
		Return = FullName.substr(1).thru_char("'");
		return Return;
	}

	msstring ParserName;
	msstringlist Params;			//Must be allocated each time, because this funtion is called recursively
	if (pszText[0] == '$')
	{
		BreakUpLine(FullName, ParserName, Params);
		for (int i = 0; i < Params.size(); i++)
			Params[i] = SCRIPTVAR(Params[i]);

		//Handle entity-specific parser
		if (m.pScriptedInterface && m.pScriptedInterface->GetScriptVar(ParserName, Params, this, Return))
			return Return;
	}

	msgetterhash_t::iterator            iFunc = m_GlobalGetterHash.find(ParserName);
	if (iFunc != m_GlobalGetterHash.end())
	{
		try
		{
			return (this->*(iFunc->second.GetFunc()))(FullName, ParserName, Params);
		}
		catch (...)
		{
			ErrorPrintCommand("RESOLVE_FUNC"
				, NULL
				, ParserName
				, Params
				, 0
				, "Error while resolving function"
			);
		}
	}
	else
	{
		// MiB 30NOV_2014 Leaving these out of the hash resolution.
		// I think it would be a huge waste of memory to split them into functions,
		//		not to mention a huge headache to verify they're all added or
		//		add additional constants. Shouldn't add much overhead, anyway.
		if (FullName.starts_with("const."))
		{
			msstring PropName = FullName.substr(6);

			if (PropName.starts_with("movetype."))	//eventually move sound to here
			{
				PropName = PropName.substr(9);
				if (PropName == "none")				RETURN_INT(MOVETYPE_NONE)
				else if (PropName == "walk")			RETURN_INT(MOVETYPE_WALK)
				else if (PropName == "step")			RETURN_INT(MOVETYPE_STEP)
				else if (PropName == "fly")			RETURN_INT(MOVETYPE_FLY)
				else if (PropName == "toss")			RETURN_INT(MOVETYPE_TOSS)
				else if (PropName == "push")			RETURN_INT(MOVETYPE_PUSH)
				else if (PropName == "noclip")			RETURN_INT(MOVETYPE_NOCLIP)
				else if (PropName == "flymissle")		RETURN_INT(MOVETYPE_FLYMISSILE)
				else if (PropName == "bounce")			RETURN_INT(MOVETYPE_BOUNCE)
				else if (PropName == "bouncemissle")	RETURN_INT(MOVETYPE_BOUNCEMISSILE)
				else if (PropName == "follow")			RETURN_INT(MOVETYPE_FOLLOW)
				else if (PropName == "pushstep")		RETURN_INT(MOVETYPE_PUSHSTEP)
			}
			else if (PropName.starts_with("snd."))	//eventually move sound to here
			{
				PropName = PropName.substr(4);
				if (PropName == "auto_channel")		RETURN_INT(CHAN_AUTO)
				else if (PropName == "weapon")			RETURN_INT(CHAN_WEAPON)
				else if (PropName == "voice")			RETURN_INT(CHAN_VOICE)
				else if (PropName == "item")			RETURN_INT(CHAN_ITEM)
				else if (PropName == "body")			RETURN_INT(CHAN_BODY)
				else if (PropName == "stream")			RETURN_INT(CHAN_STREAM)
				else if (PropName == "static")			RETURN_INT(CHAN_STATIC)
				else if (PropName == "fullvol" || PropName == "maxvol")	RETURN_INT(10)
				else if (PropName == "slientvol" || PropName == "novol")	RETURN_INT(0)
			}
			else if (PropName.starts_with("localplayer."))	//eventually move sound to here
			{
				PropName = PropName.substr(12);
				if (PropName == "scriptID")		RETURN_INT(PLAYER_SCRIPT_ID)
			}
		}
		else if (FullName.starts_with("game."))
		{
			Name = FullName.substr(5);

			bool fSuccess = false, ValidCmd = true;
			bool IsServer =
#ifdef VALVE_DLL
				true;
#else
				false;
#endif

			if (Name == "time")
			{
				RETURN_FLOAT(gpGlobals->time)
			}
			else if (Name.starts_with("time."))
			{
				// game.time.* functions
				// MiB OCT2007a

				// MiB DEC2014_07 - FN time (when server side, us FN time, if possible)
				time_t curTime;
#ifdef VALVE_DLL
				// if ( MSCentral::Enabled() ) << use central time?
				time(&curTime);
#else
				time(&curTime);
#endif

				struct tm* TheTime = localtime(&curTime);


				msstring dayInfo = ctime(&curTime);
				int theReturn = -1;
				int day = TheTime->tm_mday;         // Correct day of the MONTH
				int month = TheTime->tm_mon + 1;   // Zero indexed month
				int year = TheTime->tm_year + 1900;   // Zero indexed year (starting with 1900)
				int DoW = TheTime->tm_wday + 1;      // Zero indexed Day of Week (0 == Sunday)
				int thoth_hour = TheTime->tm_hour;		//hour (24 format?)
				int thoth_minute = TheTime->tm_min;		//minute (past the hour?)

				if (Name.starts_with("time.day"))         theReturn = day;
				else if (Name.starts_with("time.minute"))   theReturn = thoth_minute;
				else if (Name.starts_with("time.hour"))   theReturn = thoth_hour;
				else if (Name.starts_with("time.year"))   theReturn = year;
				else if (Name.starts_with("time.month"))   theReturn = month;
				else if (Name.starts_with("time.year"))   theReturn = year;
				else if (Name.starts_with("time.dow"))      theReturn = DoW;
				else if (Name.contains(".since")) //game.time.*.since or game.time.minutes
				{

					//Create NewYears as a new time

					time_t NewYears;
					struct tm* timeinfo;

					time(&NewYears);

					timeinfo = localtime(&NewYears);

					timeinfo->tm_hour = 0;   // 12.XX.XX AM
					timeinfo->tm_min = 0;   // 12.00.XX AM
					timeinfo->tm_sec = 0;   // 12.00.00 AM

					timeinfo->tm_mon = 0;   // January XX
					timeinfo->tm_mday = 1;   // January 01 NOT ZERO INDEXED


					timeinfo->tm_yday = 0;   // January 1 "Days since Jan 1" - redundant, but if one works and the other doesn't, it's worth it

					NewYears = mktime(timeinfo);

					//***timeinfo->tm_year = 100;// Y2K -> 2000 - 1900
					// RAWR - Thought you said Y2K >_< Anywhoo, here if you want it

					// Extra flags that you can mess with if ya want.
					// timeinfo->tm_isdst;   //DaylightSavingsTime flag - it IS an int
					// timeinfo->tm_wday;   //[0-6] "Days since Sunday"

					//End Creation


					int dif = (int)difftime(curTime, NewYears); // Seconds since new years

					// Now output the time since New Years in seconds, minutes, hours, or days.

					if (Name.contains("seconds"))   //game.time.seconds.since
						theReturn = dif;
					else if (Name.contains("minutes")) //game.time.minutes.since or game.time.minutes
						theReturn = dif / 60;
					else if (Name.contains("hours")) //game.time.hours.since
						theReturn = dif / 60 / 60;
					else if (Name.contains("days")) //game.time.days.since
						theReturn = dif / 60 / 60 / 24;

				}

				RETURN_INT(theReturn);
			}
			else if (Name == "frametime") RETURN_FLOAT(gpGlobals->frametime)
#ifdef VALVE_DLL
			else if (Name == "maxlevel")
			{
				RETURN_INT(CHAR_LEVEL_CAP);
			}
			//Thothie FEB2013_24 - consolidating all "game.players.xxx" functions into one conditional, and adding noafk
			else if (Name.starts_with("players"))
			{
				//Thothie JUN2007a - make sure game.players does not return bots
				//Thothie NOV2014_09 - using new centralized checking
				if (Name.contains("totalhp")) RETURN_FLOAT(UTIL_TotalHP())
				else if (Name.contains("avghp")) RETURN_FLOAT(UTIL_AvgHP())
				else if (Name.contains("playersnb") || Name.contains("noafk")) RETURN_INT(UTIL_NumActivePlayers())
				else
				{
					RETURN_INT(UTIL_NumPlayers())
				}
			}
#endif
			//seems we commented a little too far in JUL2013a (fixed)
			//Thothie JAN2013_10 - Dynamic music system (tabs be fubar here for some reason)
			else if (Name.starts_with("map.music."))
			{
				msstring Prop = Name.substr(10);
				if (Prop == "idle.file") return MSGlobals::map_music_idle_file;
				else if (Prop == "idle.length") return MSGlobals::map_music_idle_length;
				else if (Prop == "combat.file") return MSGlobals::map_music_combat_file;
				else if (Prop == "combat.length") return MSGlobals::map_music_combat_length;
			}
			else if (Name.starts_with("map."))
			{
				//bool Type[2] = { false };
				msstring Prop = Name.substr(4);
				if (Prop == "name") return MSGlobals::MapName;
				else if (Prop == "skyname") return EngineFunc::CVAR_GetString("sv_skyname");
				else if (Prop == "addparams") return MSGlobals::map_addparams; //DEC2014_17 Thothie - global addparams
				else if (Prop == "flags") return MSGlobals::map_flags; //DEC2014_17 Thothie - map flags
				else if (Prop == "title")
				{
					msstring thoth_tstring = MSGlobals::MapTitle;
					if (thoth_tstring.len() > 1) return MSGlobals::MapTitle;
					else return "0";

				}
				else if (Prop == "weather") //Thothie SEP2007a
				{
					msstring thoth_weather = MSGlobals::MapWeather;
					if (thoth_weather.len() > 1) return MSGlobals::MapWeather;
					else return "0";
				}
				else if (Prop == "desc") //Thothie SEP2007a, updated OCT2007a
				{
					msstring thoth_tstring = MSGlobals::MapDesc;
					if (thoth_tstring.len() > 1) return MSGlobals::MapDesc;
					else return "0";
				}
				else if (Prop == "maxviewdistance") //Thothie SEP2007a, updated OCT2007a
				{
					RETURN_FLOAT(MSGlobals::maxviewdistance)
				}
				else if (Prop == "hpwarn") //Thothie SEP2007a, updated OCT2007a
				{
					msstring thoth_tstring = MSGlobals::HPWarn;
					if (thoth_tstring.len() > 1) return MSGlobals::HPWarn;
					else return "0";
				}
				else if (Prop == "spawnlimit") //Thothie OCT2016_18 spawn limiter
				{
					return UTIL_VarArgs("%i", MSGlobals::SpawnLimit);
				}
			} //end if map.
			else if (Name == "debug") return EngineFunc::CVAR_GetString("developer"); //Thothie MAR2007b - so we can disable client-side debugs from the script level (failed at the .dll level)
			else if (Name == "developer") return EngineFunc::CVAR_GetString("developer");  //ditto, alias
			else if (Name == "pvp")
			{
				//Thothie FEB2008a - having it return the global rule, rather than cvar, for better dynamics
				return MSGlobals::PKAllowed ? "1" : "0";
				//return EngineFunc::CVAR_GetString("ms_pklevel"); //Thothie MAR2007b - the long awaited script PVP awareness
			}
			else if (Name == "central") return EngineFunc::CVAR_GetString("ms_central_enabled"); //Thothie MAY2007a - the long awaited FN awareness
			else if (Name == "revision") return EngineFunc::CVAR_GetString("ms_version"); //Thothie MAY2007a - the long awaited script side version control
			else if (Name == "clientside")  fSuccess = !IsServer;
			else if (Name == "serverside")  fSuccess = IsServer;
			else if (Name.starts_with("cvar."))
			{
				msstring thoth_cvar_return = Name.substr(5);
				return EngineFunc::CVAR_GetString(thoth_cvar_return.c_str());
			}
			else if (Name.starts_with("script."))
			{
				msstring Property = Name.substr(7);
				if (Property == "last_sent_id")		RETURN_INT(m_gLastSendID)
				else if (Property == "last_light_id")  RETURN_INT(m_gLastLightID)
				else if (Property == "iteration")		RETURN_INT(m.m_Iteration)
			}
			else if (Name.starts_with("sound."))
			{
				//ONLY STILL HERE FOR BACKWARDS COMPATIBILITY
				//MOVED TO --> game.const.snd.x
				msstring SoundProp = Name.substr(6);
				if (SoundProp == "auto_channel")			RETURN_INT(CHAN_AUTO)
				else if (SoundProp == "weapon")			RETURN_INT(CHAN_WEAPON)
				else if (SoundProp == "voice")				RETURN_INT(CHAN_VOICE)
				else if (SoundProp == "item")				RETURN_INT(CHAN_ITEM)
				else if (SoundProp == "body")				RETURN_INT(CHAN_BODY)
				else if (SoundProp == "stream")			RETURN_INT(CHAN_STREAM)
				else if (SoundProp == "static")			RETURN_INT(CHAN_STATIC)
				else if (SoundProp == "fullvol" || SoundProp == "maxvol")	RETURN_INT(10)
				else if (SoundProp == "slientvol" || SoundProp == "novol")	RETURN_INT(0)
			}
			else
			{
				static msstringlist Params;
				Params.clearitems();
				TokenizeString(Name, Params, ".");
				Name = Params[0];
				msstring FullProp = &FullName[5 + Name.len() + 1];
				msstring_ref Value = RETURN_NOTHING_STR;
				if ((Name == "entity" || Name == "monster" || Name == "player" || Name == "item" || Name == "container") && m.pScriptedEnt)
				{
					Value = m.pScriptedEnt->GetProp(m.pScriptedEnt, FullProp, Params);
				}
				else if (Name == "pmove")
				{
					Value = PM_GetValue(Params);
				}
#ifndef VALVE_DLL
				else if (Name == "localplayer")
				{

					msstring& Prop = FullProp;
					if (Prop == "index") RETURN_INT(MSCLGlobals::GetLocalPlayerIndex())
					else if (Prop == "viewangles") RETURN_VECTOR(player.pev->v_angle) //Thothie JAN2013_08 - viewangles, sorta, only returns yaw
					else if (Prop == "eyepos") RETURN_VECTOR(player.EyePosition()) //Thothie JAN2013_08 eyepos - sorta, returns pos on model
					else if (Prop == "thirdperson") RETURN_INT((MSCLGlobals::CamThirdPerson ? 1 : 0))
					else if (Prop == "viewmodel.left.id" || Prop == "viewmodel.0.id") RETURN_INT(player.Hand(0) ? player.Hand(0)->GetViewModelID() : -1)
					else if (Prop == "viewmodel.right.id" || Prop == "viewmodel.1.id") RETURN_INT(player.Hand(1) ? player.Hand(1)->GetViewModelID() : -1)
					else if (Prop == "viewmodel.active.id") RETURN_INT(player.ActiveItem() ? player.ActiveItem()->GetViewModelID() : -1)
					else if (Prop == "canattack") RETURN_INT(FBitSet(player.m_StatusFlags, PLAYER_MOVE_NOATTACK) ? 0 : 1)
				}
				else if (Name == "tempent")
					Value = CLGetCurrentTempEntProp(FullProp);
#endif

				else ValidCmd = false;

				if (ValidCmd)
				{
					if (strcmp(Value, RETURN_NOTHING_STR)) return Value;
					else ValidCmd = false;
				}
				}

			if (ValidCmd)
				return fSuccess ? "1" : "0";
			} //end if game.
		}

	scriptvar_t* pScriptvar = FindVar(pszText);
	if (pScriptvar) return pScriptvar->Value;

	return pszText;
	}

Vector CScript::StringToVec(msstring_ref String)
{
	msstring Inside = SCRIPTVAR(msstring(String).thru_char(")"));

	msstring X = SCRIPTVAR(Inside.substr(1).thru_char(" ,"));
	Inside = msstring(Inside.findchar_str(" ,")).skip(" ,");
	msstring Y = SCRIPTVAR(Inside.thru_char(" ,"));
	Inside = msstring(Inside.findchar_str(" ,")).skip(" ,");
	msstring Z = SCRIPTVAR(Inside.thru_char(" )"));

	return Vector(atof(X), atof(Y), atof(Z));
}


scriptvar_t* CScript::SetVar(const char* pszVarName, const char* pszValue, bool fGlobal)
{
	scriptvar_t* pScriptvar = FindVar(pszVarName);
	if (pScriptvar)	//Found existing variable
		pScriptvar->Value = pszValue;
	else
	{					//Create a new variable
		scriptvar_t NewVar(pszVarName, pszValue);
		if (!fGlobal) { IVariables::SetVar(pszVarName, pszValue); }
		else m_gVariables.add(NewVar);
	}
	return pScriptvar;
}
scriptvar_t* CScript::SetVar(const char* pszVarName, const char* pszValue, SCRIPT_EVENT& Event)
{
	scriptvar_t* pScriptvar = Event.FindVar(pszVarName);
	if (pScriptvar)	//Found existing local variable
		pScriptvar->Value = pszValue;
	else pScriptvar = SetVar(pszVarName, pszValue); //Didn't find a local variable, so set it as a regular variable

	return pScriptvar;
}
scriptvar_t* CScript::SetVar(const char* pszVarName, int iValue, bool fGlobal) {
	char ctemp[64];
	_snprintf(ctemp, sizeof(ctemp), "%i", iValue);
	return SetVar(pszVarName, ctemp, fGlobal);
}
scriptvar_t* CScript::SetVar(const char* pszVarName, float flValue, bool fGlobal) {
	char ctemp[64];
	_gcvt(flValue, 10, ctemp);
	return SetVar(pszVarName, ctemp, fGlobal);
}
// Called when creating a new item from a global template
void CScript::CopyAllData(CScript* pDestScript, CBaseEntity* pScriptedEnt, IScripted* pScriptedInterface) {
	//I am the source script

	pDestScript->m = m;
	pDestScript->m.pScriptedEnt = pScriptedEnt;
	pDestScript->m.pScriptedInterface = pScriptedInterface;
	//pDestScript->m.Events = m.Events;

	//Copy Variables
	int variables = m_Variables.size();
	for (int i = 0; i < variables; i++)
		pDestScript->m_Variables.add(m_Variables[i]);
}
float GetNumeric(const char* pszText) {
	return atof(pszText);
}
msstring_ref CScript::GetScriptVar(msstring_ref VarName)
{
	return SCRIPTVAR(VarName);
}


CScript::CScript()
{
	clrmem(m);
	Script_Setup();
	ScriptGetterHash_Setup();

	ScriptMgr::RegisterScript(this);
}
CScript::~CScript()
{
	ScriptMgr::UnRegisterScript(this);

	m_Variables.clear();
	m_Constants.clear();
}


bool CScript::Spawn(string_i Filename, CBaseEntity* pScriptedEnt, IScripted* pScriptedInterface, bool PrecacheOnly, bool Casual)
{
	//Keep track of all #included files... don't allow #including the same file twice
	//Update: A script can specify when it wants to allow duplicate includes
	if (!m.AllowDupInclude)
	{
		for (int i = 0; i < m_Dependencies.size(); i++)
		{
			if (!stricmp(m_Dependencies[i], Filename))
			{
				return true;	//Return true here, so its a 'fake' successful.  This should only happen on #includes
			}
		}
	}

	m_Dependencies.add(Filename);

	//Localize these for later reference
	//pScriptedEnt->ScriptFName = Filename; //MiB DEC2007a - scriptname prop - phayle
	m.pScriptedEnt = pScriptedEnt;
	m.pScriptedInterface = pScriptedInterface;
	m.PrecacheOnly = PrecacheOnly;
	m.ScriptFile = Filename;
	m.DefaultScope = EVENTSCOPE_SHARED;
	m.AllowDupInclude = false;
	m.Included = m_Dependencies.size() >= 2;
	bool fReturn = false;

	//This should always be true for non-dev builds, because it must use the script library
	char* ScriptData;
	msstring ScriptName = m.ScriptFile;
	ScriptName += SCRIPT_EXT;

	//MiB JUL2010_13 - *EXT/ means to external script folder. Lets mappers script.
	//- Thothie - not intuitive, switching to test_scripts/ - same folder scripts are to be placed in.
	bool MapperScript = ScriptName.starts_with("test_scripts/") && MSGlobals::DevModeEnabled;
	if (MapperScript)
	{
		ScriptName = ScriptName.substr(13); //Get rid of the *EXT/
		m.ScriptFile = ScriptName; //.thru_char(".").c_str();
	}

#ifndef SCRIPT_LOCKDOWN
	char cScriptFile[MAX_PATH];
	_snprintf(cScriptFile, "test_scripts/%s", ScriptName.c_str());

	int iFileSize;
	byte* pMemFile = LOAD_FILE_FOR_ME(cScriptFile, &iFileSize);
	if (pMemFile)
	{
		ScriptData = msnew(char[iFileSize + 1]);
		memcpy(ScriptData, pMemFile, iFileSize);
		ScriptData[iFileSize] = 0;
		FREE_FILE(pMemFile);
	}
	else
#endif
		unsigned long ScriptSize;
	if (MapperScript && !MSGlobals::CentralEnabled)
	{
		char cScriptFile[MAX_PATH], cGameDir[MAX_PATH];
#ifdef VALVE_DLL
		GET_GAME_DIR(cGameDir);
#else
		strncpy(cGameDir, gEngfuncs.pfnGetGameDirectory(), MAX_PATH);
#endif

		_snprintf(cScriptFile, MAX_PATH, "test_scripts/%s", ScriptName.c_str()); //Thothie FEB2010_06 - attempting to fix other folks not being able to use test_scripts folder

		int iFileSize;
		byte* pMemFile = LOAD_FILE_FOR_ME(cScriptFile, &iFileSize);
		if (pMemFile)
		{
			ScriptData = msnew(char[iFileSize + 1]);
			memcpy(ScriptData, pMemFile, iFileSize);
			ScriptData[iFileSize] = 0;
			FREE_FILE(pMemFile);
		}
	}
	else if (ScriptMgr::m_GroupFile.ReadEntry(ScriptName, NULL, ScriptSize))
	{
		ScriptData = msnew(char[ScriptSize + 1]);
		ScriptMgr::m_GroupFile.ReadEntry(ScriptName, (byte*)ScriptData, ScriptSize);
		ScriptData[ScriptSize] = 0;
	}
	else
	{
		if (!Casual)
		{
#ifndef SCRIPT_LOCKDOWN
#ifdef VALVE_DLL
			CSVGlobals::LogScript(ScriptName, m.pScriptedEnt, m_Dependencies.size(), m.PrecacheOnly, false);
#endif
			ALERT(at_notice, "Script file: \"%s\" NOT FOUND!\n", ScriptName.c_str()); //thothie - moved to at_notice in hopes of getting bogus script reports
#else
#ifndef RELEASE_LOCKDOWN
#ifdef VALVE_DLL
			logfile << "ERROR: Script not found: " << ScriptName.c_str() << "\n";
#ifndef POSIX
			MessageBox(NULL, msstring("Script not found: ") + ScriptName + "\r\n\r\nThis is probably caused by a script using #include on a non-existant script.", "FIX THIS QUICK!", MB_OK);
#endif
#endif
#else
			//In the release build, this is a fatal error
			//SERVER_COMMAND( "exit\n" ); This crashes the game, currently
//server side so we can retain ability to add server side only scripts
#ifdef VALVE_DLL
#ifndef POSIX
			MessageBox(NULL, msstring("Script not found: ") + ScriptName, "MAP SCRIPT ERROR", MB_OK); //Thothie - JUN2007 Trying to get script bugs to report
			//exit( 0 ); //MAR2008a Thothie - making non-fatal so it can report multiple
#endif
#endif
#endif
#endif
	}
		return false;
	}

#ifdef VALVE_DLL
	CSVGlobals::LogScript(ScriptName, m.pScriptedEnt, m_Dependencies.size(), m.PrecacheOnly, true);
#endif

	if (m.pScriptedInterface)
	{
		m.pScriptedInterface->Script_Setup();
	}

	fReturn = ParseScriptFile(ScriptData);	//Parse events

	delete[] ScriptData;  //Deallocate script data

	RunScriptEventByName("game_precache");	//Run precache event

	//CallLogged(ScriptName, clock_start);
	return fReturn;
}

void CScript::RunScriptEvents(bool fOnlyRunNamedEvents)
{
	startdbg;
	dbg("Proc_Events");
	//Run script events
	//~ Runs unnamed events or named events that were specified with calleventtimed ~
	int events = m.Events.size();
	for (int i = 0; i < events; i++)
	{
		SCRIPT_EVENT& Event = m.Events[i];
		//Skip unnamed events when running named events only
		msstring thoth_event_name = (m.ScriptFile.c_str());
		thoth_event_name.append("->");
		thoth_event_name.append(Event.Name);
		dbg(thoth_event_name);
		if (!Event.Name && fOnlyRunNamedEvents)
			continue;

		//Run an event, if it's time
		::mslist<float> CachedExecutions;

		if (Event.fNextExecutionTime > -1 && gpGlobals->time >= Event.fNextExecutionTime)
		{
			CachedExecutions.add(Event.fNextExecutionTime);
			Event.fNextExecutionTime = -1;
		}

		for (int e = 0; e < Event.TimedExecutions.size(); e++)
		{
			if (gpGlobals->time < Event.TimedExecutions[e])
				continue;

			CachedExecutions.add(Event.TimedExecutions[e]);

			Event.TimedExecutions.erase(e--);
		}

		for (int e = 0; e < CachedExecutions.size(); e++)
		{
			Script_ExecuteEvent(Event);
		}
	}

	enddbg;
}
void CScript::RunScriptEventByName(msstring_ref pszEventName, msstringlist* Parameters)
{
	SCRIPT_EVENT* CurrentEvent = m.CurrentEvent; //Save the event currently executing

	//Run every event with this name
	for (int i = 0; i < m.Events.size(); i++)
	{
		SCRIPT_EVENT& seEvent = m.Events[i];
		if (!seEvent.Name || seEvent.Name != pszEventName)
			continue;

		Script_ExecuteEvent(seEvent, Parameters);
	}

	m.CurrentEvent = CurrentEvent;	//Restore the current event
}

void CScript::CallLogged(msstring_ref title, std::clock_t start)
{
	std::clock_t end = std::clock();
	msstringlist Parameters;
	Parameters.add(title);
	Parameters.add(FloatToString((float)(end - start)));
	RunScriptEventByName("display_timing", &Parameters);
}

//helper function to create a list of parameters from command line.
//also handles quotes properly!
::mslist<std::string> GetParams(std::string& str)
{
	::mslist<std::string> params;
	std::string word = "";
	bool inQuote = false;

	int strlen = str.length();
	for (int i = 0; i < strlen; i++)
	{
		const char ch = str[i];

		if (ch == '"')
		{
			if (inQuote)
			{
				if (!word.empty())
				{
					params.push_back(word);
					word.clear();
				}
				inQuote = false;
			}
			else
				inQuote = true;

			continue;
		}

		if (strutil::isSpace(ch) && !inQuote)
		{
			if (!word.empty())
			{
				params.push_back(word);
				word.clear();
			}
			continue;
		}

		word += ch;

		//process last word.
		if (i == strlen - 1 && !word.empty())
			params.push_back(word);
	}

	return params;
}

bool CScript::ParseScriptFile(const char* pszScriptData)
{
	startdbg;

	dbg("Begin");
	if (!m.ScriptFile.len() || !pszScriptData)
		return false;

#ifdef MSDEBUG
	ALERT(at_console, "Loading Script file: %s...\n", STRING(ScriptFile));
#endif

	SCRIPT_EVENT* CurrentEvent = NULL; //Current event
	scriptcmd_list* CurrentCmds = NULL;	//Current command list within event
	::mslist<scriptcmd_list*> ParentCmds; //List of all my parent command lists, top to bottom

	size_t lineNum = 1;
	std::string sData(pszScriptData);
	std::istringstream ss(sData);

	std::string line;
	while (getline(ss, line))
	{
		line.erase(0, line.find_first_not_of(" \t\v"));

		std::string result = "";
		int lineSize = line.length();
		for (int i = 0; i < lineSize; i++)
		{
			const char ch = line[i];
			const char nextch = line[i + 1]; //get next ch.

			//remove comments.
			if (ch == '/' && nextch == '/')
				break;

			//just remove return carriages here instead of doing erase.
			if (ch == '\r')
				break;

			result += ch;
		}

		if (result.find_first_not_of(" \r\t") != std::string::npos)
		{
			//remove extra spaces
			//credit to https://stackoverflow.com/questions/35301432/remove-extra-white-spaces-in-c
			// result.erase(std::unique(std::begin(result), std::end(result), [](unsigned char a, unsigned char b){
			// 	return isSpace(a) && isSpace(b);
			// }), std::end(result));

			//we use the new parse line function that isn't really new.
			//NewParseLine(result, lineNum, &CurrentEvent, &CurrentCmds, ParentCmds);
			ParseLine(result.c_str(), lineNum, &CurrentEvent, &CurrentCmds, ParentCmds);
			//Log("Line: %s", result.c_str());
		}

		lineNum++;
	}

	enddbg("CSript::ParseScriptFile()");
	//  Uncomment to print out all this function gathered from the script file
	//#ifndef VALVE_DLL
		/*SCRIPT_EVENT *pEvent = seFirstEvent;
		CHAR *pszPtr;
		ALERT( at_console, "\nScript: %s\n", pScriptedEnt->pev?STRING(pScriptedEnt->pev->classname):"new script" );
		while( pEvent ) {
			pszPtr = pEvent->Action;
			if( pEvent->Name ) ALERT( at_console, "Event: %s\n", STRING(pEvent->Name) );
			else ALERT( at_console, "Event: Unnamed\n" );
			for( unsigned int i = 0; i < strlen(pszPtr); i++ ) {
				if( pszPtr[i] == '\r' ) ALERT( at_console, "R" );
				else if( pszPtr[i] == '\n' ) ALERT( at_console, "N" );
				else ALERT( at_console, "%c", pszPtr[i] );
			}
			ALERT( at_console, "\n" );
			pEvent = pEvent->NextEvent;
		}*/
		//#endif
	return true;
}

//returns: 0 - failed, 1 - done, 2 - not done yet.
// int CScript::NewParseLine(std::string &pszCommandLine, int LineNum, SCRIPT_EVENT **pCurrentEvent, scriptcmd_list **pCurrentCmds, ::mslist<scriptcmd_list *> &ParentCmds)
// {
// 	//startdbg;
// 	//dbg("Begin");

// 	//Log("Parse: %s", pszCommandLine.c_str());
// 	SCRIPT_EVENT *CurrentEvent = *pCurrentEvent;
// 	scriptcmd_list &CurrentCmds = **pCurrentCmds;
// 	::mslist<std::string> params = GetParams(pszCommandLine);
// 	int paramSize = params.size(); //we set the size here because it will never change.
// 	std::string TestCommand = params[0]; //key 0 is the command.
// 	//Log("cmd: %s", TestCommand.c_str());
// 	char cBuffer[256]; //this is error buffer.

// 	//keepCmd - This is for pre-commands that also function as normal commands
// 	// pre-commands have to be inside an event.
// 	bool keepCmd = false;

// 	//this cannot be msstring type for whatever reason...
// 	const static std::unordered_map<std::string,int> cmdMap {
// 		{"{", 1},
// 		{"#include", 2},
// 		{"#scope", 3},
// 		{"}", 4},
// 		//{"if", 5}, // if command is too OP for this.
// 		{"else", 6},
// 		{"eventname", 7},
// 		{"repeatdelay", 8},
// 		{"setvar", 9},
// 		{"const", 10},
// 		{"setvarg", 11},
// 		{"removeconst", 12},
// 		{"setvard", 13},
// 		{"const_ovrd", 14},
// 		{"setmodel", 15},
// 		{"setviewmodel", 16},
// 		{"setworldmodel", 17},
// 		{"setpmodel", 18},
// 		{"setshield", 19},
// 		{"attachsprite", 20},
// 		{"svplaysound", 21},
// 		{"svplayrandomsound", 22},
// 		{"svsound.play3d", 23},
// 		{"precache", 24},
// 		{"say", 25},
// 		{"sound", 26},
// 		{"model", 27},
// 		{"sound.play3d", 28},
// 		{"svsound.play3d", 29},
// 	};

// 	switch(cmdMap.count(TestCommand) ? cmdMap.at(TestCommand) : 0)
// 	{
// 	case 1: // {
// 	{
// 		if( !*pCurrentEvent )
// 		{
// 			//Create a new Event
// 			eventscope_e esScope = m.DefaultScope;
// 			msstring Name;
// 			bool Override = false;

// 			//Read options and the event name (all of it is optional and can be in any order)
// 			for(int i = 1; i < paramSize; i++)
// 			{
// 				std::string pam = params[i];

// 				if(!pam.compare("[client]")) esScope = EVENTSCOPE_CLIENT;
// 				else if(!pam.compare("[server]")) esScope = EVENTSCOPE_SERVER;
// 				else if(!pam.compare("[shared]")) esScope = EVENTSCOPE_SHARED;
// 				else if(!pam.compare("[override]")) Override = true;
// 				else Name = msstring(pam.c_str());
// 			}

// 			SCRIPT_EVENT Event;
// 			clrmem(Event);

// 			Event.fNextExecutionTime = -1;
// 			Event.fRepeatDelay = -1;

// 			if(Name.len())
// 			{
// 				//Named event.  Don't run until called
// 				Event.Name = Name;
// 			}
// 			//Unnamed event.  Run once and let repeatdelay decide if it gets run after that
// 			else Event.fNextExecutionTime = 0;		//Run at least once

// 			Event.Scope = esScope;

// 			if( Name.len() && Override )						//Delete all previous occurances of this event
// 				for(int i = 0; i < m.Events.size(); i++)
// 					if( Name == m.Events[i].Name )
// 						{ m.Events.erase( i ); i--; }

// 			m.Events.add(Event);
// 			*pCurrentEvent = &m.Events[m.Events.size()-1];
// 			*pCurrentCmds = &m.Events[m.Events.size()-1].Commands;
// 		}
// 		else
// 		{
// 			if( !ParentCmds.size() )
// 			{
// 				MSErrorConsoleText( __FUNCTION__, UTIL_VarArgs("Script: %s, Line: %i - %s \"{\" following non conditional command!\n", m.ScriptFile.c_str(), LineNum, TestCommand.c_str(), cBuffer) );
// 				return 0;
// 			}

// 			//Entering the group of commands within a conditional statement
// 			CurrentCmds.m_SingleCmd = false;	//Allow multiple commands in this list
// 		}
// 		return 1;
// 	}
// 	case 2: // #include [scope] <name> [allowduplicate]
// 	{
// 		//Include another script file
// 		msstring FileName(params[1].c_str());
// 		eventscope_e Scope = EVENTSCOPE_SHARED;
// 		bool Casual = false;

// 		if(FileName[0] == '[')
// 		{
// 			if(FileName.contains("[server]")) Scope = EVENTSCOPE_SERVER;
// 			else if(FileName.contains("[client]")) Scope = EVENTSCOPE_CLIENT;
// 			if(FileName.contains("[casual]")) Casual = true;

// 			FileName = msstring(params[2].c_str()); //filename is second parameter then.
// 		}

// 		FileName = GetConst(FileName);

// 		//TODO: refactor this
// 		//Check the scope of this include.  Scope == EVENTSCOPE_SHARED means include the file on both
// 		if( (Scope == EVENTSCOPE_SHARED) || MSGlobals::IsServer == (Scope==EVENTSCOPE_SERVER) )
// 		{
// 			bool AllowDupInclude = m.AllowDupInclude;
// 			m.AllowDupInclude = (pszCommandLine.find("[allowduplicate]") != std::string::npos) != msstring_error;

// 			string_i CurrentScriptFile = m.ScriptFile;
// 			bool fSucces = Spawn(FileName, m.pScriptedEnt, m.pScriptedInterface, m.PrecacheOnly, Casual);
// 			m.ScriptFile = CurrentScriptFile;
// 			m.AllowDupInclude = AllowDupInclude;
// 			if( !fSucces && !Casual )
// 			{
// #ifndef POSIX
// 				MessageBox( NULL, msstring("Script: ") + m.ScriptFile + " Tried to include non-existant script: " + FileName + "\r\n\r\nThis is a fatal error in the public build.", "FIX THIS QUICK!", MB_OK );
// #endif
// 				ALERT( at_console, "Script: %s, Line: %i - %s \"%s\" failed!  Possible File Not Found.\n", m.ScriptFile.c_str(), LineNum, TestCommand.c_str(), FileName.c_str() );
// 			}
// 		}
// 		return 1;
// 	}
// 	case 3: // #scope
// 	{
// 		std::string Scope = params[1];
// 		if(!Scope.compare("client")) 
// 			m.DefaultScope = EVENTSCOPE_CLIENT;
// 		else if(!Scope.compare("server")) 
// 			m.DefaultScope = EVENTSCOPE_SERVER;
// 		else if(!Scope.compare("shared")) 
// 			m.DefaultScope = EVENTSCOPE_SHARED;
// 		else 
// 			ALERT( at_console, "Script: %s, Line: %i - %s \"%s\" - Not valid!.\n", m.ScriptFile.c_str(), LineNum, TestCommand.c_str(), Scope.c_str() );
// 		return 1;
// 	}
// 	case 4: // }
// 	{
// 		if( !CurrentEvent )
// 		{ ALERT( at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum ); return 0; }

// 		if( !ParentCmds.size() )
// 		{
// 			//Done with event
// 			*pCurrentEvent = NULL;
// 			*pCurrentCmds = NULL;

// 			//If Event wasn't in the right scope, delete it now
// 			if( m.Events.size( ) && m.Events[m.Events.size()-1].Scope ==
// 				#ifdef VALVE_DLL
// 					EVENTSCOPE_CLIENT
// 				#else
// 					EVENTSCOPE_SERVER
// 				#endif
// 					)
// 			{
// 				CurrentEvent = NULL;
// 				m.Events.erase( m.Events.size() - 1 );
// 			}
// 		}
// 		else
// 		{
// 			//Done with conditional code block
// 			//return control to the parent command list
// 			*pCurrentCmds = ParentCmds[ParentCmds.size()-1];
// 			ParentCmds.erase( ParentCmds.size() - 1 );
// 		}
// 		break;
// 	}
// 	case 6: // else
// 	{
// 		if( !CurrentEvent )
// 		{ ALERT( at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum ); return 0; }

// 		scriptcmd_t &ParentCmd = CurrentCmds.m_Cmds[CurrentCmds.m_Cmds.size()-1];
// 		if( ParentCmd.m_Conditional )
// 		{
// 			ParentCmd.m_AddingElseCmds = true; //If I reach a '{', I know its for else commands and not if commands

// 			ParentCmds.add( *pCurrentCmds ); //Store the current commands list
// 			*pCurrentCmds = &ParentCmd.m_ElseCmds.add( scriptcmd_list() ); //Set the new parent command list to my new else child list
// 			(*pCurrentCmds)->m_SingleCmd = true; //Default to one command only. If I hit a '{' first, then allow a block of commands

// 			if(paramSize >= 2)
// 			{
// 				std::string ParamStr = pszCommandLine.substr(TestCommand.length());
// 				if(NewParseLine(ParamStr, LineNum, pCurrentEvent, pCurrentCmds, ParentCmds) == 2)
// 					return 2; //Return 2 so any parent command knows I'm not done yet
// 			}
// 			else 
// 				return 2; //Return 2 so any parent command knows I'm not done yet
// 		}
// 		else MSErrorConsoleText( "CSript::ParseLine", UTIL_VarArgs("Script: %s, Line: %i - %s \"else\" following non conditional command!\n", m.ScriptFile.c_str(), LineNum, TestCommand.c_str(), cBuffer) );
// 		break;
// 	}
// 	case 7: // eventname
// 	{
// 		if( !CurrentEvent )
// 		{ ALERT( at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum ); return 0; }

// 		if(paramSize >= 2)
// 		{
// 			CurrentEvent->Name = params[1].c_str();
// 			CurrentEvent->fNextExecutionTime = -1;
// 			CurrentEvent->fRepeatDelay = -1;
// 		}
// 		else MSErrorConsoleText( "CSript::ParseLine", UTIL_VarArgs("Script: %s, Line: %i - %s \"eventname\" incorrect amount of parameters!\n", m.ScriptFile.c_str(), LineNum, TestCommand.c_str(), cBuffer) );
// 		break;
// 	}
// 	case 8: // repeatdelay
// 	{
// 		if( !CurrentEvent )
// 		{ ALERT( at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum ); return 0; }

// 		if(paramSize >= 2)
// 		{
// 			CurrentEvent->fRepeatDelay = atof(SCRIPTCONST(params[1].c_str()));
// 			CurrentEvent->fNextExecutionTime = gpGlobals->time + CurrentEvent->fRepeatDelay;
// 			keepCmd = true;
// 		}
// 		else MSErrorConsoleText( "CSript::ParseLine", UTIL_VarArgs("Script: %s, Line: %i - %s \"repeatdelay\" incorrect amount of parameters!\n", m.ScriptFile.c_str(), LineNum, TestCommand.c_str(), cBuffer) );
// 		break;
// 	}
// 	case 9: // setvar
// 	{
// 		if( !CurrentEvent )
// 		{ ALERT( at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum ); return 0; }

// 		if( CurrentEvent->Scope !=
// 			#ifdef VALVE_DLL
// 				EVENTSCOPE_CLIENT
// 			#else
// 				EVENTSCOPE_SERVER
// 			#endif
// 				)
// 		{
// 			msstring VarName(params[1].c_str());
// 			msstring VarValue(params[2].c_str());

// 			conflict_check(VarName,"setvar","preload");

// 			VarValue = msstring(GETCONST_COMPATIBLE(VarValue));
// 			SetVar(VarName, VarValue, false);

// 			keepCmd = true;
// 		}
// 		break;
// 	}
// 	case 10: // const
// 	{
// 		if( !CurrentEvent )
// 		{ ALERT( at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum ); return 0; }

// 		if( CurrentEvent->Scope !=
// 			#ifdef VALVE_DLL
// 				EVENTSCOPE_CLIENT
// 			#else
// 				EVENTSCOPE_SERVER
// 			#endif
// 				)
// 		{
// 			msstring VarName(params[1].c_str());
// 			msstring VarValue(params[2].c_str());

// 			conflict_check(VarName,"const","preload");

// 			VarValue = msstring(GETCONST_COMPATIBLE(VarValue));

// 			bool addConst = true;
// 			for(int i = 0; i < m_Constants.size(); i++)
// 			{
// 				if(m_Constants[i].Name == VarName)
// 				{
// 					addConst = false;
// 					break;
// 				}
// 			}

// 			if (addConst)
// 				m_Constants.add(scriptvar_t(VarName, VarValue)); //Create new constant
// 		}
// 		break;
// 	}
// 	case 11: // setvarg
// 	{
// 		if( !CurrentEvent )
// 		{ ALERT( at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum ); return 0; }

// 		if( CurrentEvent->Scope !=
// 			#ifdef VALVE_DLL
// 				EVENTSCOPE_CLIENT
// 			#else
// 				EVENTSCOPE_SERVER
// 			#endif
// 				)
// 		{
// 			msstring VarName(params[1].c_str());
// 			msstring VarValue(params[2].c_str());

// 			conflict_check(VarName,"setvar","preload");

// 			VarValue = msstring(GETCONST_COMPATIBLE(VarValue));
// 			SetVar(VarName, VarValue, true);

// 			keepCmd = true;
// 		}
// 		break;
// 	}
// 	case 12: // removeconst
// 	{
// 		if( !CurrentEvent )
// 		{ ALERT( at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum ); return 0; }

// 		msstring VarName(params[1].c_str());

// 		//Update constant
// 		for(int i = 0; i < m_Constants.size(); i++)
// 		{
// 			if(m_Constants[i].Name == VarName)
// 			{
// 				m_Constants.erase(i);
// 				break; 
// 			}
// 		}
// 		break;
// 	}
// 	case 13: // setvard
// 	{
// 		if( !CurrentEvent )
// 		{ ALERT( at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum ); return 0; }

// 		//Really just a setvar that skips the loadtime execution
// #if !TURN_OFF_ALERT
// 		//Thothie JUN2013_08 - check for conflicts in developer builds as we go
// 		msstring VarName(params[1].c_str());
// 		conflict_check(VarName,"setvard","preload");
// #endif
// 		TestCommand = "setvar";
// 		keepCmd = true;
// 		break;
// 	}
// 	case 14: // const_ovrd
// 	{
// 		if( !CurrentEvent )
// 		{ ALERT( at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum ); return 0; }

// 		//Thothie NOV2014_18 - fixing const_ovrd to only work during run time
// 		msstring VarName(params[1].c_str());
// 		msstring VarValue(params[2].c_str());

// 		VarValue = msstring(GETCONST_COMPATIBLE(VarValue));

// 		//keep command, but don't use until run time
// 		//HOW DO THIS!?
// 		if ( !m.PrecacheOnly )
// 		{
// 			//Print("DEBUG: const_ovrd to replace %s with %s\n",VarName.c_str(),VarValue.c_str());
// 			for(int i = 0; i < m_Constants.size(); i++)
// 			{
// 				//Print("DEBUG: const_ovrd checking %s vs %s\n",m_Constants[i].Name.c_str(),VarName.c_str());
// 				if ( m_Constants[i].Name == VarName )
// 				{
// 					m_Constants[i].Value = VarValue;
// 					//Print("DEBUG: const_ovrd found! %s is now %s\n",m_Constants[i].Name.c_str(),m_Constants[i].Value.c_str());
// 					break;
// 				}
// 			}
// 		}
// 		keepCmd = true;
// 		break;
// 	}
// 	case 15:
// 	case 16:
// 	case 17:
// 	case 18:
// 	case 19:
// 	case 20:
// 	case 21:
// 	case 22:
// 	case 23:
// 	case 24:
// 	case 25:
// 	case 26:
// 	case 27:
// 	case 28:
// 	case 29:
// 	{
// 		if( !CurrentEvent )
// 		{ ALERT( at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum ); return 0; }

// #ifdef VALVE_DLL
// 		int ResourceIdx = 0;
// 		msstringlist Resources;
// 		enum {pctype_model, pctype_sound, pctype_sprite} Precachetype;

// 		bool SoundType = false;
// 		if(!TestCommand.compare("sound.play3d") || !TestCommand.compare("svsound.play3d"))
// 			SoundType = true;

// 		for(int i = 1; i < paramSize; i++)
// 		{
// 			bool SkipFirst = (!SoundType && (TestCommand.find("sound") != std::string::npos)) ? true : false;
// 			std::string arg = params[i];

// 			if(!SkipFirst || ResourceIdx)
// 			{
// 				msstring Resolved = SCRIPTCONST(arg.c_str());

// 				if(!TestCommand.compare("say"))
// 				{
// 					Resolved = Resolved.thru_char("[");

// 					if(Resolved == "*")
// 					{
// 						Resolved = "";
// 						ALERT(at_console, "%s Warning: Old 'say' command using '*' as sound name\n", m.ScriptFile.c_str());
// 					}

// 					if(Resolved.len())
// 					{
// 						if(!Resolved.contains("RND_SAY"))
// 							Resolved = msstring("npc/") + Resolved + ".wav"; //still total h4x, but there's no good way to fix this without changing how the command works
// 					}

// 					if(Resolved.len() && strstr(Resolved, "."))
// 						Resources.add(Resolved);
// 				}
// 			}

// 			ResourceIdx++;
// 		}

// 		for(int it = 0; it < Resources.size(); it++)
// 		{
// 			msstring &FileName = Resources[it];
// 			if(FileName == "none") 
// 				continue;

// 			if(!FileName.contains(".wav") && !FileName.contains(".mdl") && !FileName.contains(".spr"))
// 				continue;

// 			msstring Extension = &FileName[FileName.len()-4];

// 			if( Extension == ".wav" ) 
// 				Precachetype = pctype_sound;
// 			else if( Extension == ".mdl" )
// 				Precachetype = pctype_model;
// 			else if( Extension == ".spr" )
// 				Precachetype = pctype_sprite;
// 			else { /*Print( "Found unknown preache: %s\n", FileName.c_str() );*/ continue; }

// 			msstring Dirname;
// 			Dirname = (Precachetype==pctype_model) ? "models/" : (Precachetype==pctype_sprite) ? "sprites/" : "";
// 			msstring Fullpath = Dirname + FileName;

// 			static msstring Precaches[16384] = { "" };

// 			static int PrecachesTotal = 0;
// 			char *pszGlobalPointer = NULL;

// 			for(int p = 0; p < PrecachesTotal; p++)
// 			{
// 				if( Fullpath == Precaches[p] )
// 				{
// 					pszGlobalPointer = Precaches[p];
// 					break;
// 				}
// 			}

// 			if( !pszGlobalPointer )
// 				pszGlobalPointer = (Precaches[PrecachesTotal++] = Fullpath).c_str();

// 			switch(Precachetype)
// 			{
// 				case pctype_sound:
// 					if (TestCommand.compare("precache")) PRECACHE_SOUND(pszGlobalPointer); //Thothie MAR2012_27 - no longer precahing sounds from here, using client side sounds wherever possible
// 					break;
// 				case pctype_sprite:
// 					PRECACHE_MODEL(pszGlobalPointer);	
// 					break;
// 				case pctype_model:
// 					PRECACHE_MODEL(pszGlobalPointer);  
// 					break;
// 			}
// 		}
// #endif
// 		if(TestCommand.compare("precache")) //Don't keep the precache command
// 			keepCmd = true;

// 		break;
// 	}
// 	// this block processes the more complex commands ex. (if())
// 	// or is a unknown command.
// 	case 0:
// 	{
// 		std::string Testy = pszCommandLine.substr(0, 2);
// 		if (!Testy.compare("if"))
// 		{
// 			if( !CurrentEvent )
// 			{ ALERT( at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum ); return 0; }

// 			if(pszCommandLine.find("(") == std::string::npos)
// 			{
// 				keepCmd = true;
// 			}
// 			else
// 			{
// 				scriptcmd_t &ScriptCmd = CurrentCmds.m_Cmds.add(scriptcmd_t("if()", true));	//Change the command name to "if()
// 				ScriptCmd.m_NewConditional = true;

// 				std::string cLine = pszCommandLine.substr(pszCommandLine.find_first_of('(')+1);
// 				//we take advantage of msstring's functions for the rest.
// 				msstring ParamStr = msstring(cLine.c_str()).skip(SKIP_STR);
// 				Log("cmdline: %s", cLine.c_str());
// 				Log("paramstr: %s", ParamStr.c_str());

// 				for(int i = 0; i < 3; i++ )
// 				{
// 					ScriptCmd.m_Params.add(GetConst(ParamStr.thru_char(SKIP_STR))); //Save the next parameter - Resolve Contants but not variables
// 					ParamStr = msstring(ParamStr.findchar_str(SKIP_STR)).skip(SKIP_STR); //Skip over the parameter's text and any spaces
// 					if(!i && ParamStr[0] == ')')
// 						break; //Compare parameter was ')' -- this if statement only has one parameter Ex: if( var ) command
// 				}

// 				if(ParamStr[0] == ')')
// 				{
// 					ParentCmds.add(*pCurrentCmds); //Store the current commands list
// 					*pCurrentCmds = &ScriptCmd.m_IfCmds; //Set the new parent command list to my true statment child list
// 					(*pCurrentCmds)->m_SingleCmd = true; //Default to one command only.  If I hit a '{' first, then allow a block of commands

// 					//Check if there are any commands at the end of the if line and parse them under this if statement
// 					ParamStr = msstring(ParamStr.findchar_str(SKIP_STR)).skip(SKIP_STR); //Skip the ')' and any spaces

// 					if(!ParamStr[0] || ParamStr[0] == ')')
// 						return 2;
// 					else
// 					{
// 						if(NewParseLine(std::string(ParamStr), LineNum, pCurrentEvent, pCurrentCmds, ParentCmds) == 2)
// 							return 2;
// 					}
// 				}
// 				else
// 				{
// 					MSErrorConsoleText("SCript::ParseLine()", msstring("Script: ") + m.ScriptFile.c_str() + " Line: " + LineNum + " - if() statement missing ')'!\n");
// 				}
// 			}
// 		}
// 		else
// 		{
// 			keepCmd = true; //normal command
// 		}
// 		break;
// 	}
// 	}

// 	if (!keepCmd || m.PrecacheOnly)
// 	{
// 		if((*pCurrentCmds) && (*pCurrentCmds)->m_SingleCmd)
// 		{
// 			//This is the first line after a conditional and we didn't encounter an opening brace.
// 			//The conditional only gets this one command... we now return control to the parent command list
// 			if(ParentCmds.size())
// 			{
// 				*pCurrentCmds = ParentCmds[ParentCmds.size()-1];
// 				ParentCmds.erase(ParentCmds.size()-1);

// 			}
// 			else 
// 				MSErrorConsoleText("", UTIL_VarArgs("Script: %s, Line: %i - Conditional command returned to parent cmd list but the parent list wasn't found!\n", m.ScriptFile.c_str(), LineNum));

// 			return 1;
// 		}
// 	}
// 	else
// 	{
// 		//Check if this word is a command
// 		const char* testcmd = TestCommand.c_str();
// 		scriptcmdname_t Command(testcmd);
// 		msfunchash_t::iterator iFunc = m_GlobalCmdHash.find(msstring(testcmd));
// 		bool fFoundCmd = iFunc != m_GlobalCmdHash.end();

// 		//Create the command
// 		scriptcmd_t ScriptCmd(msstring(testcmd), fFoundCmd ? iFunc->second.GetConditional() : false);
// 		if(!fFoundCmd)
// 		{
// 			//First word was not a command

// 			//I have an owner, try his parseline function
// 			int iReturn = m.pScriptedInterface ? m.pScriptedInterface->Script_ParseLine(this, pszCommandLine.c_str(), ScriptCmd) : 0;
// 			if( iReturn <= 0 )
// 			{
// 				//Owner entity didn't recognize command
// 				if( m.PrecacheOnly )
// 					ALERT(at_console, "Script: %s, Line: %i - Command \"%s\" NOT FOUND!\n", m.ScriptFile.c_str(), LineNum, testcmd);
// 				return 0;
// 			}
// 		}

// 		//Add all the command's parameters
// 		for (int p = 1; p < paramSize; p++)
// 		{
// 			//Log("Param: %s", params[p].c_str());
// 			ScriptCmd.m_Params.add(GetConst(params[p].c_str())); //Resolve constants, but not variables
// 		}

// 		//Add the command to the bunch
// 		CurrentCmds.m_Cmds.add(ScriptCmd);
// 	}

// 	return 1;
// }

int CScript::ParseLine(const char* pszCommandLine /*in*/, int LineNum /*in*/, SCRIPT_EVENT** pCurrentEvent /*in/out*/,
	scriptcmd_list** pCurrentCmds /*in/out*/, ::mslist<scriptcmd_list*>& ParentCmds /*in/out*/)
{
	startdbg;

	dbg("Begin");

	SCRIPT_EVENT* CurrentEvent = *pCurrentEvent;
	scriptcmd_list& CurrentCmds = **pCurrentCmds;

	char TestCommand[128]; TestCommand[0] = 0;
	char cBuffer[512];
	int LineOfs = 0, TmpLineOfs = 0;

#define CmdLine &pszCommandLine[LineOfs]
#define CmdLineTmp &pszCommandLine[TmpLineOfs]

	dbg(pszCommandLine);

	//Read the first word of the line
	if (sscanf(pszCommandLine, "%s", TestCommand) <= 0)
		return 0;

	LineOfs += strlen(TestCommand);

	//Read spaces
	if (sscanf(CmdLine, "%[ \t\n\r]", cBuffer) > 0)
		LineOfs += strlen(cBuffer);

	TmpLineOfs = LineOfs;
	msstring Line = CmdLineTmp;
	//ALERT( at_console, "CommandFound: %s\n", TestCommand );

	//Check if this word is a pre-command.
	if (!stricmp(TestCommand, "{"))
	{
		if (!*pCurrentEvent)
		{
			//Create a new Event
			eventscope_e esScope = m.DefaultScope;
			msstring Name;
			bool Override = false;

			msstring Param = Line.thru_char(SKIP_STR);

			//Read options and the event name (all of it is optional and can be in any order)
			while (Param.len())
			{
				if (Param == "[client]")		esScope = EVENTSCOPE_CLIENT;
				else if (Param == "[server]")	esScope = EVENTSCOPE_SERVER;
				else if (Param == "[shared]")	esScope = EVENTSCOPE_SHARED;
				else if (Param == "[override]") Override = true;
				else Name = Param;

				Line = Line.substr(Param.len()).skip(SKIP_STR);
				Param = Line.thru_char(SKIP_STR);
			}

			SCRIPT_EVENT Event;
			clrmem(Event);

			Event.fNextExecutionTime = -1;
			Event.fRepeatDelay = -1;

			if (Name.len())
			{
				//Named event.  Don't run until called
				Event.Name = Name;
			}
			//Unnamed event.  Run once and let repeatdelay decide if it gets run after that
			else Event.fNextExecutionTime = 0;		//Run at least once

			Event.Scope = esScope;

			if (Name.len() && Override)						//Delete all previous occurances of this event
				for (int i = 0; i < m.Events.size(); i++)
					if (Name == m.Events[i].Name)
					{
						m.Events.erase(i); i--;
					}

			m.Events.add(Event);
			*pCurrentEvent = &m.Events[m.Events.size() - 1];
			*pCurrentCmds = &m.Events[m.Events.size() - 1].Commands;
		}
		else
		{
			if (!ParentCmds.size())
			{
				MSErrorConsoleText(__FUNCTION__, UTIL_VarArgs("Script: %s, Line: %i - %s \"{\" following non conditional command!\n", m.ScriptFile.c_str(), LineNum, TestCommand, cBuffer));
				return 0;
			}

			//Entering the group of commands within a conditional statement
			CurrentCmds.m_SingleCmd = false;	//Allow multiple commands in this list
		}
		return 1;
	}
	else if (!stricmp(TestCommand, "#include"))
	{	//#include [scope] <name> [allowduplicate]
		//Include another script file
		msstring FileName = Line.thru_char(SKIP_STR);
		eventscope_e Scope = EVENTSCOPE_SHARED;
		bool Casual = false;

		if (FileName[0] == '[')	//#include [scope][casual] <name>
		{
			if (FileName.contains("[server]")) Scope = EVENTSCOPE_SERVER;
			else if (FileName.contains("[client]")) Scope = EVENTSCOPE_CLIENT;
			if (FileName.contains("[casual]"))
				Casual = true;

			Line = Line.substr(FileName.len()).skip(SKIP_STR);
			FileName = Line.thru_char(SKIP_STR);
		}

		FileName = GetConst(FileName);

		//Check the scope of this include.  Scope == EVENTSCOPE_SHARED means include the file on both
		if ((Scope == EVENTSCOPE_SHARED) || MSGlobals::IsServer == (Scope == EVENTSCOPE_SERVER))
		{
			msstring AllowDup = msstring(Line.substr(FileName.len())).skip(SKIP_STR).thru_char(SKIP_STR);

			string_i CurrentScriptFile = m.ScriptFile;
			bool AllowDupInclude = m.AllowDupInclude;
			m.AllowDupInclude = AllowDup.find("allowduplicate") != msstring_error;
			bool fSucces = Spawn(FileName, m.pScriptedEnt, m.pScriptedInterface, m.PrecacheOnly, Casual);
			m.ScriptFile = CurrentScriptFile;
			m.AllowDupInclude = AllowDupInclude;
			if (!fSucces && !Casual)
			{
#ifndef POSIX
				MessageBox(NULL, msstring("Script: ") + m.ScriptFile + " Tried to include non-existant script: " + FileName + "\r\n\r\nThis is a fatal error in the public build.", "FIX THIS QUICK!", MB_OK);
#endif
				ALERT(at_console, "Script: %s, Line: %i - %s \"%s\" failed!  Possible File Not Found.\n", m.ScriptFile.c_str(), LineNum, TestCommand, FileName.c_str());
			}
		}
		return 1;
	}
	else if (!stricmp(TestCommand, "#scope"))
	{
		msstring Scope = Line.thru_char(SKIP_STR);
		if (Scope == "client")		m.DefaultScope = EVENTSCOPE_CLIENT;
		else if (Scope == "server")	m.DefaultScope = EVENTSCOPE_SERVER;
		else if (Scope == "shared")	m.DefaultScope = EVENTSCOPE_SHARED;
		else ALERT(at_console, "Script: %s, Line: %i - %s \"%s\" - Not valid!.\n", m.ScriptFile.c_str(), LineNum, TestCommand, Scope.c_str());
		return 1;
	}

	if (!CurrentEvent)
	{
		ALERT(at_console, "Script: %s, Line: %i Missing {\n", m.ScriptFile.c_str(), LineNum); return 0;
	}

	//KeepCmd - This is for pre-commands that also function as normal commands
	//			The pre-commands below require that we be inside of an event
	bool KeepCmd = false;

	if (!stricmp(TestCommand, "}"))
	{
		if (!ParentCmds.size())
		{
			//Done with event
			*pCurrentEvent = NULL;
			*pCurrentCmds = NULL;

			//If Event wasn't in the right scope, delete it now
			if (m.Events.size() && m.Events[m.Events.size() - 1].Scope ==
#ifdef VALVE_DLL
				EVENTSCOPE_CLIENT
#else
				EVENTSCOPE_SERVER
#endif
				)
			{
				CurrentEvent = NULL;
				m.Events.erase(m.Events.size() - 1);
			}
		}
		else
		{
			//Done with conditional code block
			//return control to the parent command list
			*pCurrentCmds = ParentCmds[ParentCmds.size() - 1];
			ParentCmds.erase(ParentCmds.size() - 1);
		}
	}
	else if (!stricmp(msstring(TestCommand).substr(0, 2), "if"))
	{
		//This could be a new-style if or an old-style if
		if (!strstr(TestCommand, "(") && *CmdLineTmp != '(') //Old: if var == var
			KeepCmd = true;
		else
		{
			/*
				New:	if( var == var ) command
				if( var == var )
				{
					command1
					command2
				}
			*/

			//Create the command
			scriptcmd_t& ScriptCmd = CurrentCmds.m_Cmds.add(scriptcmd_t("if()", true));	//Change the command name to "if()
			ScriptCmd.m_NewConditional = true;

			//Add all the command's parameters
			if (*CmdLineTmp == '(') TmpLineOfs++;											//Go past the '('
			msstring ParamStr = msstring(CmdLineTmp).skip(SKIP_STR);					//Skip spaces
			for (int i = 0; i < 3; i++)
			{
				ScriptCmd.m_Params.add(GetConst(ParamStr.thru_char(SKIP_STR)));			//Save the next parameter - Resolve Contants but not variables
				ParamStr = msstring(ParamStr.findchar_str(SKIP_STR)).skip(SKIP_STR);	//Skip over the parameter's text and any spaces
				if (!i && ParamStr[0] == ')')
					break;																	//Compare parameter was ')' -- this if statement only has one parameter Ex: if( var ) command

			}

			if (ParamStr[0] == ')')
			{
				ParentCmds.add(*pCurrentCmds);											//Store the current commands list
				*pCurrentCmds = &ScriptCmd.m_IfCmds;										//Set the new parent command list to my true statment child list
				(*pCurrentCmds)->m_SingleCmd = true;										//Default to one command only.  If I hit a '{' first, then allow a block of commands

				//Check if there are any commands at the end of the if line and parse them under this if statement
				ParamStr = msstring(ParamStr.findchar_str(SKIP_STR)).skip(SKIP_STR);	//Skip the ')' and any spaces

				if (!ParamStr[0] || ParamStr[0] == ')')
					return 2;  //Return 2 so any parent command knows I'm not done yet
				else
				{
					//There is a command at the end of this line
					if (ParseLine(ParamStr, LineNum, pCurrentEvent, pCurrentCmds, ParentCmds) == 2)	//Parse the command at the end of the line
						return 2;	//I found a conditional at the end of this line that isn't done yet... so let it finish parsing
					//...I found a non-conditional or a conditional which only had one line... it drop out and escape to my parent
				}
			}
			else
				MSErrorConsoleText("SCript::ParseLine()", msstring("Script: ") + m.ScriptFile.c_str() + " Line: " + LineNum + " - if() statement missing ')'!\n");
		}
	}
	else if (!stricmp(TestCommand, "else"))
	{
		scriptcmd_t& ParentCmd = CurrentCmds.m_Cmds[CurrentCmds.m_Cmds.size() - 1];
		if (ParentCmd.m_Conditional)
		{
			ParentCmd.m_AddingElseCmds = true;											//If I reach a '{', I know its for else commands and not if commands

			ParentCmds.add(*pCurrentCmds);											//Store the current commands list
			*pCurrentCmds = &ParentCmd.m_ElseCmds.add(scriptcmd_list());				//Set the new parent command list to my new else child list
			(*pCurrentCmds)->m_SingleCmd = true;										//Default to one command only.  If I hit a '{' first, then allow a block of commands

			if (*CmdLineTmp && !isspace(*CmdLineTmp))
			{
				//There is a command at the end of this line
				if (ParseLine(CmdLineTmp, LineNum, pCurrentEvent, pCurrentCmds, ParentCmds) == 2)	//Parse the command at the end of the line
					return 2;	//I found a conditional at the end of this line that isn't done yet... so let it finish parsing
				//...I found a non-conditional or a conditional which only had one line... it drop out and escape to my parent
			}
			else
				return 2; //Return 2 so any parent command knows I'm not done yet
		}
		else MSErrorConsoleText("CSript::ParseLine", UTIL_VarArgs("Script: %s, Line: %i - %s \"else\" following non conditional command!\n", m.ScriptFile.c_str(), LineNum, TestCommand, cBuffer));
	}
	else if (!stricmp(TestCommand, "eventname"))
	{
		//Set a name for the current event
		sscanf(CmdLineTmp, "%s", cBuffer);
		CurrentEvent->Name = cBuffer;
		CurrentEvent->fNextExecutionTime = -1;
		CurrentEvent->fRepeatDelay = -1;
	}
	else if (!stricmp(TestCommand, "repeatdelay"))
	{
		//Set a delay timer for the current event
		sscanf(CmdLineTmp, "%s", cBuffer);
		CurrentEvent->fRepeatDelay = atof(SCRIPTCONST(cBuffer));
		CurrentEvent->fNextExecutionTime = gpGlobals->time + CurrentEvent->fRepeatDelay;
		KeepCmd = true;
	}
	else if (!stricmp(TestCommand, "setvar") ||
		!stricmp(TestCommand, "const"))
	{
		if (CurrentEvent->Scope !=
#ifdef VALVE_DLL
			EVENTSCOPE_CLIENT
#else
			EVENTSCOPE_SERVER
#endif
			)
		{

			//Set variable value.  "setvarg" sets global variable
			msstring VarName = Line.thru_char(SKIP_STR);

#if !TURN_OFF_ALERT
			//Thothie JUN2013_08 - check for conflicts in developer builds as we go
			msstring testvar = VarName;
			msstring testvar_type;
			msstring testvar_scope = "preload";
			if (!stricmp(TestCommand, "setvar")) testvar_type = "setvar";
			if (!stricmp(TestCommand, "const")) testvar_type = "const";
			conflict_check(testvar, testvar_type, testvar_scope, LineNum);
#endif

			Line = Line.substr(VarName.len()).skip(SKIP_STR);
			msstring VarValue;
			if (Line[0] == '"') VarValue = Line.substr(1).thru_char("\"");		//Starting quote found, read until the next quote
			else VarValue = Line.thru_char(SKIP_STR);				//No quotes

			VarValue = msstring(GETCONST_COMPATIBLE(VarValue));				//Resolve both constants and variables -- TODO: Remove the variables - should be constants only
			if (!stricmp(TestCommand, "setvar"))
			{
				SetVar(VarName, VarValue, !stricmp(TestCommand, "setvarg") ? true : false);
				KeepCmd = true;
			}
			else
			{
				bool AddConst = true;

				for (int i = 0; i < m_Constants.size(); i++)
				{
					if (m_Constants[i].Name == VarName)
					{
						AddConst = false;
						break;
					}
				}

				if (AddConst)
				{
					m_Constants.add(scriptvar_t(VarName, VarValue));//Create new constant
				}
				//else if( !stricmp(TestCommand, "const_ovrd") ) m_Constants[i].Value = VarValue; //NOV2014_18 - no good here, runs at load time
			}
		}
	}
	else if (!stricmp(TestCommand, "removeconst"))
	{
		msstring VarName = Line.thru_char(SKIP_STR);

		for (int i = 0; i < m_Constants.size(); i++)						//Update constant
			if (m_Constants[i].Name == VarName)
			{
				m_Constants.erase(i);
				break;
			}
	}
	else if (!stricmp(TestCommand, "setvard"))
	{
		//Really just a setvar that skips the loadtime execution
#if !TURN_OFF_ALERT
			//Thothie JUN2013_08 - check for conflicts in developer builds as we go
		msstring tVarName = Line.thru_char(SKIP_STR);
		msstring testvar = tVarName;
		msstring testvar_type = "setvard";
		msstring testvar_scope = "preload";
		conflict_check(testvar, testvar_type, testvar_scope, LineNum);
#endif
		strncpy(TestCommand, "setvar", 128);
		KeepCmd = true;
	}
	//this fails, as it sometimes runs at load time (not sure why)
	else if (!stricmp(TestCommand, "const_ovrd"))
	{
		//Thothie NOV2014_18 - fixing const_ovrd to only work during run time
		msstring VarName = Line.thru_char(SKIP_STR);

		Line = Line.substr(VarName.len()).skip(SKIP_STR);
		msstring VarValue;
		if (Line[0] == '"') VarValue = Line.substr(1).thru_char("\"");		//Starting quote found, read until the next quote
		else VarValue = Line.thru_char(SKIP_STR);				//No quotes

		VarValue = msstring(GETCONST_COMPATIBLE(VarValue));

		//keep command, but don't use until run time
		//HOW DO THIS!?
		if (!m.PrecacheOnly)
		{
			//Print("DEBUG: const_ovrd to replace %s with %s\n",VarName.c_str(),VarValue.c_str());
			for (int i = 0; i < m_Constants.size(); i++)
			{
				//Print("DEBUG: const_ovrd checking %s vs %s\n",m_Constants[i].Name.c_str(),VarName.c_str());
				if (m_Constants[i].Name == VarName)
				{
					m_Constants[i].Value = VarValue;
					//Print("DEBUG: const_ovrd found! %s is now %s\n",m_Constants[i].Name.c_str(),m_Constants[i].Value.c_str());
					break;
				}
			}
		}
		KeepCmd = true;
	}
	/*else if( !stricmp(TestCommand,	"precachefile"		) )
	{
		//precache another script file
		sscanf( CmdLineTmp, "%[^\t \r\n]", cBuffer );
		strcpy( cBuffer, SCRIPTCONST(cBuffer) );
		strcat( cBuffer, SCRIPT_EXT );
		string_t CurrentScriptFile = m.ScriptFile;
		CScript TempScript;
		bool fSucces = TempScript.Spawn( ALLOC_STRING(cBuffer), NULL, NULL, true );
		if( !fSucces )
			ALERT( at_console, "Script: %s, Line: %i - %s \"%s\" failed!  Possible File Not Found.\n", m.ScriptFile.c_str(), LineNum, TestCommand, cBuffer );
		return 1;
	}*/
	else if (!stricmp(TestCommand, "setmodel") ||
		!stricmp(TestCommand, "setviewmodel") ||
		!stricmp(TestCommand, "setworldmodel") ||
		!stricmp(TestCommand, "setpmodel") ||
		!stricmp(TestCommand, "setshield") ||
		!stricmp(TestCommand, "attachsprite") ||
		!stricmp(TestCommand, "svplaysound") ||
		//!stricmp(TestCommand,	"playsoundcl"		) ||  //never used
		!stricmp(TestCommand, "svplayrandomsound") ||
		!stricmp(TestCommand, "svsound.play3d") ||
		!stricmp(TestCommand, "precache") ||
		!stricmp(TestCommand, "say")
		)
	{
#ifdef VALVE_DLL
		char cSpaces[256] = "";
		msstringlist Resources;
		enum { pctype_model, pctype_sound, pctype_sprite } Precachetype;

		int ResourceIdx = 0;
		char* pSearchLine = "%s";
		cBuffer[0] = 0;
		int SndType = 0;
		if (!stricmp(TestCommand, "sound.play3d") || !stricmp(TestCommand, "svsound.play3d")) SndType = 1;	//Sound name is first parameter

		while (sscanf(CmdLineTmp, pSearchLine, cSpaces, cBuffer) > 0) //The first time preceding spaces aren't checked and the result is in cSpaces
		{
			TmpLineOfs += strlen(cSpaces) + strlen(cBuffer);
			if (!ResourceIdx)
				strncpy(cBuffer, cSpaces, sizeof(cBuffer));		//The first parameter ends up in cSpaces.... move it to cBuffer
			bool SkipFirst = (!SndType && strstr(TestCommand, "sound")) ? true : false;


			if (!SkipFirst || ResourceIdx)						//Sounds skip the first parameter
			{
				msstring Resolved = SCRIPTCONST(cBuffer);		//For now, resolve both consts and vars
				//Later, once the scripts have been updated remove vars
				if (!strcmp(TestCommand, "say"))
				{
					//Thothie JUN2007b - see thoth_pissed
					//- Well, this here explains why I couldn't stop it from parsing into waves
					Resolved = Resolved.thru_char("[");		//Special case for 'say'.  Get the sound separate from the delay
					if (Resolved == "*") { Resolved = ""; ALERT(at_console, "%s Warning: Old 'say' command using '*' as sound name\n", m.ScriptFile.c_str()); }
					if (Resolved.len())
					{
						if (!Resolved.contains("RND_SAY"))
							Resolved = msstring("npc/") + Resolved + ".wav"; //still total h4x, but there's no good way to fix this without changing how the command works
					}
				}
				if (Resolved.len() && strstr(Resolved, "."))
					Resources.add(Resolved);
			}

			ResourceIdx++;

			if (strstr(TestCommand, "sprite") || strstr(TestCommand, "model") || strstr(TestCommand, "setshield") || SndType == 1) break;	//If a sprite or model, only use the first parameter
			pSearchLine = "%[ \t\r\n]%s";
		}

		for (int i = 0; i < Resources.size(); i++)
		{
			msstring& FileName = Resources[i];

			if (FileName == "none") continue;

			if (!FileName.contains(".wav") && !FileName.contains(".mdl") && !FileName.contains(".spr")) continue; //NOV2015_14 Thothie - don't attempt to precache non-media, makes the check below redundant

			/*
			if( FileName.len() < 4 )
			{
				Print( "Found precache with name too small: %s in %s line %i\n", FileName.c_str(),m.ScriptFile.c_str(),LineNum );
				continue;
			}
			*/

			msstring Extension = &FileName[FileName.len() - 4];
			//Thothie - Fail
			/*if ( !FileName.contains("items") )
			{
				int thoth_pre = gpGlobals->PreCount;
				thoth_pre++;
				gpGlobals->PreCount = thoth_pre;
			}*/

			if (Extension == ".wav") Precachetype = pctype_sound;
			else if (Extension == ".mdl")
				Precachetype = pctype_model;
			else if (Extension == ".spr")
				Precachetype = pctype_sprite;
			else { /*Print( "Found unknown preache: %s\n", FileName.c_str() );*/ continue; }

			msstring Dirname;
			Dirname = (Precachetype == pctype_model) ? "models/" : (Precachetype == pctype_sprite) ? "sprites/" : "";
			msstring Fullpath = Dirname + FileName;

			//char *pszDLLString = (char *)STRING(ALLOC_STRING(Fullpath));
			//pszDLLString = Fullpath;
			static msstring Precaches[16384] = { "" };
			static int PrecachesTotal = 0;

			char* pszGlobalPointer = nullptr; //Precaches MUST be global pointers. Can't use stack memory
			for (int p = 0; p < PrecachesTotal; p++)
				if (Fullpath == Precaches[p])
				{
					pszGlobalPointer = Precaches[p];
					break;
				}
			if (!pszGlobalPointer)
			{
				pszGlobalPointer = (Precaches[PrecachesTotal++] = Fullpath).c_str();
			}

			//if( Fullpath.contains( "human" ) )
			//	int stop = 0;

			switch (Precachetype)
			{
			case pctype_sound:
				if (stricmp(TestCommand, "precache"))
					PRECACHE_SOUND(pszGlobalPointer); //Thothie MAR2012_27 - no longer precahing sounds from here, using client side sounds wherever possible
				break;
			case pctype_sprite:
				PRECACHE_MODEL(pszGlobalPointer);	break;
			case pctype_model:
				PRECACHE_MODEL(pszGlobalPointer); break;
			}
		}
#endif
		if (stricmp(TestCommand, "precache")) //Don't keep the precache command
			KeepCmd = true;
	}
	else KeepCmd = true; //Normal command

	if (CurrentEvent &&
		CurrentEvent->Scope ==		//Don't keep any commands if in the wrong scope.
#ifdef VALVE_DLL			//This also prevents 'not found' errors from server-only commands found on the client
		EVENTSCOPE_CLIENT
#else
		EVENTSCOPE_SERVER
#endif
		) KeepCmd = false;

	if (!KeepCmd || m.PrecacheOnly)
		goto DontKeepCommand;	//Don't keep this command

	{
		//Check if this word is a command
		scriptcmdname_t Command(TestCommand);
		msfunchash_t::iterator iFunc = m_GlobalCmdHash.find(msstring(TestCommand));
		bool fFoundCmd = iFunc != m_GlobalCmdHash.end();

		//Create the command
		scriptcmd_t ScriptCmd(msstring(TestCommand), fFoundCmd ? iFunc->second.GetConditional() : false);

		if (!fFoundCmd)
		{
			//First word was not a command

			//I have an owner, try his parseline function
			int iReturn = m.pScriptedInterface ? m.pScriptedInterface->Script_ParseLine(this, pszCommandLine, ScriptCmd) : 0;

			if (iReturn <= 0)
			{
				//Owner entity didn't recognize command
				if (m.PrecacheOnly)
					ALERT(at_console, "Script: %s, Line: %i - Command \"%s\" NOT FOUND!\n", m.ScriptFile.c_str(), LineNum, TestCommand);
				return 0;
			}
		}

		//Add all the command's parameters
		while (sscanf(CmdLine, "%s", cBuffer) > 0)
		{
			LineOfs += strlen(cBuffer);

			//Stop at end-of-line comments
			if (!strncmp(cBuffer, "//", 2))
				break;

			if (cBuffer[0] == '"')
			{
				LineOfs -= strlen(cBuffer);		//Bring the offset back to the quote
				LineOfs++;						//Set the offset to the next char
				if (sscanf(CmdLine, "%[^\"]", cBuffer) > 0) //Read until the next quote
				{
					LineOfs += strlen(cBuffer);	//Move up the offset to span the quoted parameter
					LineOfs++;					//Skip the closing quote
				}
				else	//Error: No closing quotes
					MSErrorConsoleText("", UTIL_VarArgs("Script: %s, Line: %i - \"%s\" Closing quotations NOT FOUND!\n", m.ScriptFile.c_str(), LineNum, cBuffer));
			}

			ScriptCmd.m_Params.add(GetConst(cBuffer));	//Resolve constants, but not variables

			if (sscanf(CmdLine, "%[ \t\r\n]", cBuffer) > 0) LineOfs += strlen(cBuffer);
		}

		//Add the command to the bunch
		CurrentCmds.m_Cmds.add(ScriptCmd);
	}

DontKeepCommand:
	if ((*pCurrentCmds) && (*pCurrentCmds)->m_SingleCmd)
	{
		//This is the first line after a conditional and we didn't encounter an opening brace.
		//The conditional only gets this one command... we now return control to the parent command list
		if (ParentCmds.size())
		{
			*pCurrentCmds = ParentCmds[ParentCmds.size() - 1];
			ParentCmds.erase(ParentCmds.size() - 1);
		}
		else MSErrorConsoleText("", UTIL_VarArgs("Script: %s, Line: %i - Conditional command returned to parent cmd list but the parent list wasn't found!\n", m.ScriptFile.c_str(), LineNum, cBuffer));
	}

	enddbg("CScript::ParseLine()");

	return 1;
}

// Executes a single script event
bool CScript::Script_ExecuteEvent(SCRIPT_EVENT& Event, msstringlist* Parameters)
{
	m.CurrentEvent = &Event;
	if (m.pScriptedEnt) m.pScriptedInterface->Script_ExecuteEvent(this, Event);

	//-1 means don't ever play again
	//Event.fNextExecutionTime = -1;

	//Auto-repeat if repeatdelay is set
	//if( Event.fRepeatDelay > -1 ) Event.fNextExecutionTime = gpGlobals->time + Event.fRepeatDelay;

	//Setup variables for the event
	Script_SetupEvent(Event, Parameters);

	//Execute the commands
	bool fReturn = Script_ExecuteCmds(Event, Event.Commands);
	Event.bFullStop = false; // MiB 07DEC_2014 - "exit" command

	Event.m_Variables.clearitems(); //Erase all local variables
	Event.Params = NULL;

	m.CurrentEvent = NULL;
	return fReturn;
}
bool CScript::Script_SetupEvent(SCRIPT_EVENT& Event, msstringlist* Parameters)
{
	Event.Params = Parameters;

	//Setup option parameter variables
	if (Parameters)
		for (int i = 0; i < (*Parameters).size(); i++)
			Event.SetLocal(msstring("PARAM") + int(i + 1), (*Parameters)[i]);

	return m.pScriptedInterface ? m.pScriptedInterface->Script_SetupEvent(this, Event) : true;
}
void CScript::ErrorPrintCommand(char* vsUniqueTag, SCRIPT_EVENT* pEvent, msstring& vsCmdName, msstringlist& vParams, int vParamStrt, char* vsText)
{
#if !TURN_OFF_ALERT
	msstring                            vsParams;
	for (int i = 0; i < vParams.size() - vParamStrt; i++)
		vsParams += msstring("\n  ") + i + ": " + vParams[i + vParamStrt];
	ErrorPrint(vsUniqueTag
		, ERRORPRINT_LOG | ERRORPRINT_CONSOLE
		, "Error with command '%s' %s for script file %s. Parameters:%s\n%s"
		, vsCmdName.c_str()
		, pEvent ? UTIL_VarArgs("event '%s' ", pEvent->Name.c_str())
		: ""
		, m.ScriptFile.c_str()
		, vsParams.c_str()
		, vsText
	);
	ErrorPrint(vsUniqueTag
		, ERRORPRINT_CVAR
		, "Script command error (%s) see log"
		, vsCmdName.c_str()
	);
#endif
}
bool CScript::Script_ExecuteCmds(SCRIPT_EVENT& Event, scriptcmd_list& Cmdlist)
{
	for (int c = 0; c < Cmdlist.m_Cmds.size(); c++)
	{
		scriptcmd_t& Cmd = Cmdlist.m_Cmds[c];

		//Convert the variable parameters
		msstringlist Params;
		for (int icmd = 0; icmd < Cmd.m_Params.size() - 1; icmd++)
			Params.add(GetVar(Event.GetLocal(Cmd.m_Params[icmd + 1])));

		// if Script_ExecuteCmd returns false, break execution of the event
		if (Script_ExecuteCmd(Event, Cmd, Params))
		{
			if (Cmd.m_Conditional)
				//This command was a conditional command and the conditions were met, execute the child commands
				Script_ExecuteCmds(Event, Cmd.m_IfCmds);
		}
		else if (Cmd.m_Conditional)
		{
			if (!Cmd.m_NewConditional)
				break; //Old if command.  Breaks event execution on failure

			for (int e = 0; e < Cmd.m_ElseCmds.size(); e++) // Parse all else statements
				if (Script_ExecuteCmds(Event, Cmd.m_ElseCmds[e]))
					break; //As soon as one returns true, don't parse any more

			if (Cmdlist.m_Cmds.size() == 1) //This is an event with only one conditional statment or
				return false;				//An else if statement.  This statment failed so return false to try the remaining else statements
		}

		if (Event.bFullStop)
			return false; // MiB 07DEC_2014 - "exit" command
	}

	return true;
}
void CScript::SendScript(scriptsendcmd_t& SendCmd)
{
#ifdef VALVE_DLL
	bool TargetOk = true;
	if (g_netmsg[NETMSG_CLDLLFUNC]) //g_netmsgs aren't initialized until the player is spawned... but this may be called earlier from the world.script
	{
		enum sendtype_e { ST_NEW = 0, ST_UPDATE, ST_REMOVE } Type = (SendCmd.MsgType == "remove") ? ST_REMOVE : SendCmd.MsgType == "new" ? ST_NEW : ST_UPDATE;
		if (SendCmd.MsgTarget == "all")
		{
			MESSAGE_BEGIN(MSG_ALL, g_netmsg[NETMSG_CLDLLFUNC], NULL);
		}
		else if (SendCmd.MsgTarget == "all_in_sight")
		{
			if (m.pScriptedEnt)
				MESSAGE_BEGIN(MSG_PVS, g_netmsg[NETMSG_CLDLLFUNC], m.pScriptedEnt->pev->origin);
			else TargetOk = false;
		}
		else
		{
			CBaseEntity* pEntity = m.pScriptedEnt ? m.pScriptedEnt->RetrieveEntity(SendCmd.MsgTarget) : NULL;
			if (pEntity && pEntity->IsNetClient())
				MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_CLDLLFUNC], NULL, pEntity->pev);
			else TargetOk = false;
		}

		if (TargetOk)
		{
			WRITE_BYTE(17);
			WRITE_BYTE(Type); //0 == Add | 1 == Update | 2 = Remove
			WRITE_LONG(SendCmd.UniqueID);
			if (Type == ST_NEW) WRITE_STRING(SendCmd.ScriptName);
			int Parameters = SendCmd.Params.size();
			WRITE_BYTE(Parameters);
			for (int i = 0; i < Parameters; i++)
				WRITE_STRING(SendCmd.Params[i]);
			MESSAGE_END();
		}
	}
#endif
}

CBaseEntity* CScript::RetrieveEntity(msstring_ref Name)
{
	return m.pScriptedEnt ? m.pScriptedEnt->RetrieveEntity(Name) : StringToEnt(Name);
}

Vector CScript::DetermineOrigin(msstring& vsOrigin)
{
	return m.pScriptedEnt ? m.pScriptedEnt->DetermineOrigin(vsOrigin) : StringToVec(vsOrigin);
}

void CScript::CallEventTimed(msstring_ref EventName, float Delay)
{
	float Time = gpGlobals->time + Delay;
	for (int e = 0; e < m.Events.size(); e++)
	{
		SCRIPT_EVENT& seEvent = m.Events[e];
		if (!seEvent.Name || seEvent.Name != EventName)
			continue;

		seEvent.TimedExecutions.add(Time);
	}
}


//IScripted
//*********

IScripted::IScripted()
{
	m_pScriptCommands = NULL;
}
void IScripted::Deactivate()
{
	for (int i = 0; i < m_Scripts.size(); i++)
	{
		Script_Remove(i); i--;
	}

	m_Scripts.clear();		//explicitly delete the list, to reclaim the memory

}
CScript* IScripted::Script_Add(string_i ScriptName, CBaseEntity* pEntity)
{
	//Adds a new script to the list
	CScript* Script = msnew CScript();

	bool fSuccess = Script->Spawn(ScriptName, pEntity, this);

	if (fSuccess)
		m_Scripts.add(Script);
	else
	{
		if (Script)
			delete Script;
		Script = NULL;
	}

	return Script;
}
CScript* IScripted::Script_Get(string_i ScriptName)
{
	for (int i = 0; i < m_Scripts.size(); i++)
		if (!strcmp(m_Scripts[i]->m.ScriptFile, ScriptName))
			return m_Scripts[i];
	return NULL;
}
void IScripted::Script_Remove(int idx)
{
	delete m_Scripts[idx];
	m_Scripts.erase(idx);
}

int IScripted::Script_ParseLine(CScript* Script, msstring_ref pszCommandLine, scriptcmd_t& Cmd)
{
	//Script is checking if MSMonster sees this line as a command
	char TestCommand[MSSTRING_SIZE];

	//Read the first word of the line
	if (sscanf((const char*)pszCommandLine, " %s", TestCommand) <= 0)
		return 0;

	if (m_pScriptCommands)
		for (int icommand = 0; icommand < m_pScriptCommands->size(); icommand++)
		{
			scriptcmdname_t& Checkcmd = (*m_pScriptCommands)[icommand];
			if (!stricmp(TestCommand, Checkcmd.m_Name.c_str()))
				return Checkcmd.m_Conditional ? 2 : 1;
		}

	return 0;
}

void IScripted::RunScriptEvents(bool fOnlyRunNamedEvents)
{
	for (int i = 0; i < m_Scripts.size(); i++)
	{
		CScript* Script = m_Scripts[i];

		if (!Script->m.RemoveNextFrame)
		{
			Script->RunScriptEvents(fOnlyRunNamedEvents);
		}
		else
		{
			delete Script;

			m_Scripts.erase(i);
			i--;
		}
	}
}

void IScripted::CallScriptEventTimed(msstring_ref EventName, float Delay)
{
	for (int i = 0; i < m_Scripts.size(); i++)
		m_Scripts[i]->CallEventTimed(EventName, Delay);
}

void IScripted::CallScriptEvent(msstring_ref EventName, msstringlist* Parameters)
{
	m_ReturnData = "";
	for (int i = 0; i < m_Scripts.size(); i++)
		m_Scripts[i]->RunScriptEventByName(EventName, Parameters);
}

void IScripted::Script_InitHUD(CBasePlayer* pPlayer)
{
	for (int i = 0; i < m_Scripts.size(); i++)
	{
		CScript* Script = m_Scripts[i];
		if (Script == NULL) continue;
		for (int e = 0; e < Script->m.PersistentSendCmds.size(); e++)
			Script->SendScript(Script->m.PersistentSendCmds[e]);
	}
}
msstring_ref IScripted::GetFirstScriptVar(msstring_ref VarName)
{
	//Only use the first script!
	if (!m_Scripts.size())
		return VarName;

	return m_Scripts[0]->GetVar(VarName);
}
void IScripted::SetScriptVar(msstring_ref VarName, msstring_ref Value) { if (m_Scripts.size()) m_Scripts[0]->SetVar(VarName, Value); }
void IScripted::SetScriptVar(msstring_ref VarName, int iValue) { if (m_Scripts.size()) m_Scripts[0]->SetVar(VarName, iValue); }
void IScripted::SetScriptVar(msstring_ref VarName, float flValue) { if (m_Scripts.size()) m_Scripts[0]->SetVar(VarName, flValue); }


msstring_ref SCRIPT_EVENT::GetLocal(msstring_ref Name)
{
	if (!stricmp(Name, "game.event.params"))
	{
		return Params ? UTIL_VarArgs("%i", Params->size()) : "0";
	}
	else if (!stricmp(Name, "game.event.params.token") || !stricmp(Name, "game.event.params.tokenize"))
	{
		//Thothie MAY2016_04 - so you can process a set of parameters as a token set
		//- some other command to dump to array would be good too, as sometimes parameters are long
		if (Params->size() > 1)
		{
			msstring stemp = "";
			for (int i = 0; i < Params->size(); i++)
			{
				msstring l_param = (*Params)[i];
				stemp.append(l_param);
				stemp.append(";");
			}
			return stemp.c_str();
		}
	}

	return GetVar(Name);
}

//Calls every scripted entity
//Also calls world.script
void CScript::CallScriptEventAll(msstring_ref EventName, msstringlist* Parameters)
{
#ifdef VALVE_DLL
	edict_t* pEdict = NULL;
	CBaseEntity* pEntity = NULL;
	IScripted* pScripted = NULL;

	for (int i = 1; i < gpGlobals->maxEntities; i++)
	{
		pEdict = g_engfuncs.pfnPEntityOfEntIndex(i);

		if (!pEdict || pEdict->free)	// Not in use
			continue;

		pEntity = MSInstance(pEdict);
		if (!pEntity) continue;		//No private data

		pScripted = pEntity->GetScripted();
		if (!pScripted) continue;		//Not scripted

		//Thothie DEC2017_19 - tracking down calleventall failure (blah, can't check from here)
		//Print("DEBUG: Sending to %s\n",pScripted->m_Scripts[0]->m.ScriptFile.c_str());

		pScripted->CallScriptEvent(EventName, Parameters);
	}
#else
	gHUD.m_HUDScript->CallScriptEvent(EventName, Parameters);
#endif

	if (MSGlobals::GameScript)
		MSGlobals::GameScript->CallScriptEvent(EventName, Parameters);
	}
//Thothie JUN2007a, allows callexternal on all players, via "callexternal players [delay] <event> <params...>"
void CScript::CallScriptPlayers(msstring_ref EventName, msstringlist* Parameters)
{
#ifdef VALVE_DLL
	//edict_t		*pEdict = NULL;
	//CBaseEntity *pEntity = NULL;

	IScripted* pScripted = NULL;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBaseEntity* pEntity = UTIL_PlayerByIndex(i);
		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;;
		if (!pPlayer) continue;

		pScripted = pPlayer->GetScripted();
		if (!pScripted) continue;		//Not scripted

		pScripted->CallScriptEvent(EventName, Parameters);
	}
	//#else
	//gHUD.m_HUDScript->CallScriptEvent( EventName, Parameters );*/
	/*if( MSGlobals::GameScript )
		MSGlobals::GameScript->CallScriptEvent( EventName, Parameters );*/
#endif
}

//Thothie MAR2012_27 - send a client event to all CL player scripts
//equiv of: clientevent update all const.localplayer.scriptID <event> [params...]
void CScript::ClCallScriptPlayers(msstring_ref EventName, msstringlist* Parameters)
{
#ifdef VALVE_DLL
	if (g_netmsg[NETMSG_CLDLLFUNC]) //g_netmsgs aren't initialized until the player is spawned... but this may be called earlier from the world.script
	{
		MESSAGE_BEGIN(MSG_ALL, g_netmsg[NETMSG_CLDLLFUNC], NULL);

		WRITE_BYTE(17);
		WRITE_BYTE(1); //0 == Add | 1 == Update | 2 = Remove
		WRITE_LONG(-2);
		int nParameters = Parameters->size();
		WRITE_BYTE(nParameters + 1);
		WRITE_STRING(EventName);
		for (int i = 0; i < nParameters; i++)
			WRITE_STRING((*Parameters)[i]);
		MESSAGE_END();
	}
#endif
}

//MAR2012_28 - Client Message Sound Function
//ClXPlaySoundAll( pszSound, m.pScriptedEnt->pev->origin, iChannel, m.pScriptedEnt->SndVolume, sAttn, sPitch );
//gEngfuncs.pEventAPI->EV_PlaySound( 0, *(Vector *)&Origin, CHAN_AUTO, Sound, Volume, Attn, 0, 100 );
void CScript::ClXPlaySoundAll(msstring_ref sSample, const Vector& Origin, int sChannel, float sVolume, float sAttn, int sPitch)
{
#ifdef VALVE_DLL
	if (g_netmsg[NETMSG_CLDLLFUNC]) //g_netmsgs aren't initialized until the player is spawned... but this may be called earlier from the world.script
	{
		MESSAGE_BEGIN(MSG_ALL, g_netmsg[NETMSG_CLXPLAY], NULL);
		WRITE_STRING_LIMIT(sSample, 128);
		WRITE_COORD(Origin.x);
		WRITE_COORD(Origin.y);
		WRITE_COORD(Origin.z);
		WRITE_BYTE(sChannel);
		WRITE_COORD(sVolume);
		WRITE_COORD(sAttn);
		WRITE_COORD(sPitch);
		MESSAGE_END();
	}
#endif
}


bool GetModelBounds(void* pModel, Vector Bounds[2])
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pModel;
	if (!pstudiohdr)
		return false;

	return true;
}

//we have to use our own getline because of the mixed line endings.
//credits to https://gist.github.com/josephwb/df09e3a71679461fc104
std::istream& getline(std::istream& is, std::string& t) {
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for (;;) {
		int c = sb->sbumpc();
		switch (c) {
		case '\n':
			return is;
		case '\r':
			if (sb->sgetc() == '\n') {
				sb->sbumpc();
			}
			return is;
		case EOF:
			// Also handle the case when the last line has no line ending
			if (t.empty()) {
				is.setstate(std::ios::eofbit);
			}
			return is;
		default:
			t += (char)c;
		}
	}
}

#if !TURN_OFF_ALERT
//Thothie JAN2013
void CScript::conflict_check(msstring testvar, msstring testvar_type, msstring testvar_scope, int linenum)
{
	bool cc_found = false;
	bool cc_check_against_const = false;
	bool cc_check_against_var = false;
	bool cc_check_against_global = false;
	msstring cc_conflict_rep = testvar_type;

	if (testvar_type == "setvar" || testvar_type == "setvard")
	{
		cc_check_against_const = true;
		cc_check_against_global = true;
	}
	else if (testvar_type == "const")
	{
		cc_check_against_var = true;
		cc_check_against_global = true;
	}
	else if (testvar_type == "setvarg")
	{
		cc_check_against_var = true;
		cc_check_against_const = true;
	}
	else if (testvar_type == "local")
	{
		cc_check_against_var = true;
		cc_check_against_const = true;
		cc_check_against_global = true;
	}

	if (cc_check_against_const)
	{
		for (int i = 0; i < m_Constants.size(); i++)
		{
			if (m_Constants[i].Name == testvar)
			{
				cc_conflict_rep.append("==const");
				cc_found = true;
			}
		}
	}
	if (cc_check_against_var)
	{
		for (int i = 0; i < m_Variables.size(); i++)
		{
			if (m_Variables[i].Name == testvar)
			{
				cc_conflict_rep.append("==setvar");
				cc_found = true;
			}
		}
	}
	if (cc_check_against_global)
	{
		for (int i = 0; i < m_gVariables.size(); i++)
		{
			if (m_gVariables[i].Name == testvar)
			{
				cc_conflict_rep.append("==setvarg");
				cc_found = true;
			}

		}
	}

	if (cc_found)
	{
		msstring out_error;
		if (linenum == 0)
			out_error = UTIL_VarArgs("CONFLICT_ERROR! [%s:%s]:(%s) %s %s\n", testvar_scope.c_str(), m.ScriptFile.c_str(), cc_conflict_rep.c_str(), testvar_type.c_str(), testvar.c_str());
		else
			out_error = UTIL_VarArgs("CONFLICT_ERROR! [%s:%s]:(%s) %s %s at %d\n", testvar_scope.c_str(), m.ScriptFile.c_str(), cc_conflict_rep.c_str(), testvar_type.c_str(), testvar.c_str(), linenum);

		logfile << Logger::LOG_WARN << out_error.c_str();
		//be nice to be able to return the top script here, but buggers up if I try to pull the ent to do so
		Print("%s", out_error.c_str());
		MSErrorConsoleText("", out_error.c_str());
		/*
		if ( m.pScriptedEnt )
		{
			CBaseEntity *pEntity = m.pScriptedEnt;
			IScripted *pScripted = (pEntity) ? pEntity->GetScripted() : NULL;
			if ( pScripted )
			{
				cc_error_text.append( UTIL_VarArgs( "[%s]", pScripted->m_Scripts[0]->m.ScriptFile.c_str() ) );
			}
			else
			{
				cc_error_text.append( UTIL_VarArgs( "[*%s]", m.ScriptFile.c_str() ) );
			}
		}
		else
		{
			cc_error_text.append( UTIL_VarArgs( "[*%s]", m.ScriptFile.c_str() ) );
		}
		logfile << "Conflict_error*: " << cc_error_text.c_str() << " : " << VarName.c_str() << "\n";
		//Print("Conflict_error: %s with %s in %s:\n",cc_error_text.c_str(),VarName.c_str(), m.ScriptFile.c_str() );
		//MSErrorConsoleText( "", UTIL_VarArgs("Conflict_error: %s with %s in %s:\n",cc_error_text.c_str(),VarName.c_str(), m.ScriptFile.c_str()) );
		*/
	}
}
#endif
