/***
*
*	Copyright (c) 2000, Kenneth "Dogg" Early.
*	
*	Email kene@maverickdev.com or
*		  kearly@crosswinds.net
*
****/
//////////////////////////////////
//	Special armor behavior      //
//////////////////////////////////

#include "inc_weapondefs.h"

struct armordata_t
{
	int SwapBodyParts; //Bodyparts to make invisible when this armor is worn
	float Protection;
	string_i Type;
	mslist<int> m_ProtectionAreas; //Bodyparts that the armor protects
};

#define TypeCheck   \
	if (!ArmorData) \
	return

void CGenericItem::RegisterArmor()
{
	if (ArmorData)
		delete ArmorData;

	ArmorData = msnew(armordata_t);
	ZeroMemory(ArmorData, sizeof(armordata_t));

	ArmorData->Type = GetFirstScriptVar("ARMOR_TYPE");

	//JAN2010_13 - Thothie - Changing armor registration system for use with new reference

	ArmorData->Protection = atof(GetFirstScriptVar("ARMOR_PROTECTION"));

	msstring_ref ProtectionArea = GetFirstScriptVar("ARMOR_PROTECTION_AREA");
	if (strstr(ProtectionArea, "head"))
		ArmorData->m_ProtectionAreas.add(HBP_HEAD);
	if (strstr(ProtectionArea, "chest"))
		ArmorData->m_ProtectionAreas.add(HBP_CHEST);
	if (strstr(ProtectionArea, "arms"))
		ArmorData->m_ProtectionAreas.add(HBP_ARMS);
	if (strstr(ProtectionArea, "legs"))
		ArmorData->m_ProtectionAreas.add(HBP_LEGS);

	msstring_ref ReplaceBodyParts = GetFirstScriptVar("ARMOR_REPLACE_BODYPARTS");
	if (strstr(ReplaceBodyParts, "head"))
		m_WearModelPositions.add(HBP_HEAD);
	if (strstr(ReplaceBodyParts, "chest"))
		m_WearModelPositions.add(HBP_CHEST);
	if (strstr(ReplaceBodyParts, "arms"))
		m_WearModelPositions.add(HBP_ARMS);
	if (strstr(ReplaceBodyParts, "legs"))
		m_WearModelPositions.add(HBP_LEGS);

	/*
	//Print("ARMOR REGISTERED: %s", GetFirstScriptVar("ARMOR_TYPE"));
	ArmorData->Protection = 0.0;
	if ( GetFirstScriptVar("ARMOR_TYPE") == "platemail" )
	{
		m_WearModelPositions.add( HBP_CHEST );
		m_WearModelPositions.add( HBP_ARMS );
		m_WearModelPositions.add( HBP_LEGS );
		ArmorData->m_ProtectionAreas.add( HBP_CHEST );
		ArmorData->m_ProtectionAreas.add( HBP_ARMS );
		ArmorData->m_ProtectionAreas.add( HBP_LEGS );
	}

	if ( GetFirstScriptVar("ARMOR_TYPE") == "leather" )
	{
		m_WearModelPositions.add( HBP_CHEST );
		ArmorData->m_ProtectionAreas.add( HBP_CHEST );
	}
	*/

	SetBits(Properties, ITEM_ARMOR);
}
#ifdef VALVE_DLL
float CGenericItem::Armor_Protect(damage_t Damage)
{
	if (!ArmorData)
		return Damage.flDamage;

	//Some armors might protect more than one body part and others
	//can't protect certain types of damage (slash/stab/arrows/etc)

	//Does the armor protect this area?
	//Thothie - attempt armor display fix by unremarking line below
	//no go BodyPartIdx udeclared identifier; ProtectionArea not a memeber of ArmorData
	//fldamage undeclared ID
	//if( !FBitSet( (1<<BodyPartIdx), ArmorData->ProtectionArea ) ) return flDamage;

	//Thothie JAN2010_13 - not used, commetning out to save cycles (not related to Char screen display bug)
	/*
	bool ShouldProtect = false;
	 for (int i = 0; i < m_WearModelPositions.size(); i++) 
		if( Damage.iHitGroup == m_WearModelPositions[i] )
		{
			ShouldProtect = true;
			break;
		}

	if( !ShouldProtect ) return Damage.flDamage;
	*/

	m_ReturnData[0] = 0;

	msstringlist Parameters;
	Parameters.add(EntToString(Damage.pAttacker));
	Parameters.add(UTIL_VarArgs("%.2f", Damage.flDamage));
	Parameters.add(Damage.sDamageType);
	CallScriptEvent("game_protect", &Parameters);

	//Reduce damage by a percentage
	float flDamage = Damage.flDamage * ((100 - ArmorData->Protection) * 0.01);

	//Override damage with the damage set in script
	if (m_ReturnData[0])
		flDamage = atof(m_ReturnData);

	if (Damage.pInflictor && Damage.pInflictor->GetScripted())
	{
		IScripted *pScripted = Damage.pInflictor->GetScripted();
		Parameters.clearitems();
		Parameters.add(EntToString(m_pOwner));
		Parameters.add(ItemName);
		Parameters.add(UTIL_VarArgs("%.2f", Damage.flDamage));
		pScripted->CallScriptEvent("game_hitarmor", &Parameters);
	}

	/*if( pAttacker ) {
		char *Name;
		if( pAttacker->IsPlayer() ) Name = (char *)STRING(pAttacker->pev->netname);
		else Name = (char *)STRING(pAttacker->pev->classname);
		Name = (char *)STRING(pAttacker->DisplayName);
		m_pPlayer->SendInfoMsg( "Your %s softened %s's blow\n", STRING(DisplayName), Name );
	}*/

	return flDamage;
}
#endif
int CGenericItem::Armor_GetBody(int Bodypart)
{
	//THOTHIE: attempts to fix armor display by unremarking remainder of function
	//failed: e:\hldev\msc_src\dlls\Weapons\GIArmor.cpp(111): error C2039:
	//'ProtectionArea' is not a member of 'armordata_t'  Weapons\GIArmor.cpp(15)

	/*
	if( !ArmorData ) 
		return 0;

	if( FBitSet( ArmorData->ProtectionArea, (1<<Bodypart) ) ) 
		return ArmorData->ModelBody;
	*/

	return 0;
}