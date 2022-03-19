/***
*
*	Copyright (c) 2000, Kenneth "Dogg" Early.
*	
*	Email kene@maverickdev.com or
*		  l33tdogg@hotmail.com
*
****/
//////////////////////////////////
//	Special drink behavior      //
//////////////////////////////////

#include "inc_weapondefs.h"

#ifndef VALVE_DLL
	#include "../cl_dll/hud.h"
	#include "../cl_dll/cl_util.h"
#endif

struct drinkdata_t {
	int Type, Intensity, IdleAnim;
	float flEffectDelay, flDrinkDelay;
	//Dynamic Data
	bool fDrinking;
	bool fGulped;
	float TimeDrinkStart;
};

#define DrinkableCheck if( !DrinkData ) return

void CGenericItem::RegisterDrinkable( )
{
	if( DrinkData ) delete DrinkData;

	DrinkData = msnew(drinkdata_t);
	ZeroMemory( DrinkData, sizeof(drinkdata_t) );
	//if( !stricmp(GetFirstScriptVar("DRINK_TYPE"),"givehealth") )
	//	DrinkData->Type = DRINK_GIVEHEALTH;

	DrinkData->Intensity = atof(GetFirstScriptVar("DRINK_EFFECTAMT"));
	Quality = atoi(GetFirstScriptVar("DRINK_AMOUNT"));
	DrinkData->IdleAnim = atoi(GetFirstScriptVar("DRINK_IDLE_ANIM"));
	DrinkData->flEffectDelay = atof(GetFirstScriptVar("DRINK_GULP_DELAY"));
	DrinkData->flDrinkDelay = atof(GetFirstScriptVar("DRINK_TIME"));

	SetBits( Properties, ITEM_DRINKABLE );
}
void CGenericItem::Drink( )
{
	DrinkableCheck;

	//The server *only* calls this when told, from the client
	if( DrinkData->fDrinking || !m_pOwner )
		return;

	#ifndef VALVE_DLL
		if( Quality <= 0 || !ActivatedByOwner() )
			return;
	#endif

	DrinkData->fDrinking = true;
	DrinkData->fGulped = false;
	DrinkData->TimeDrinkStart = gpGlobals->time;
	
	#ifndef VALVE_DLL
		ClientCmd( UTIL_VarArgs("drink %i\n",m_Hand) );	//Make sure the server is synced
	#endif

	CallScriptEvent( "game_start_drink" );
}
void CGenericItem::DrinkThink( )
{
	DrinkableCheck;

	if( !DrinkData->fDrinking ) return;

	if( !DrinkData->fGulped && gpGlobals->time >= DrinkData->TimeDrinkStart + DrinkData->flEffectDelay )
	{
		/*switch( DrinkData->Type )
		{
			case DRINK_GIVEHEALTH:
			#ifdef VALVE_DLL
				if( m_pOwner ) m_pOwner->GiveHP( DrinkData->Intensity );
			#endif
				break;
		}*/
		CallScriptEvent( "game_drink" );
		DrinkData->fGulped = true;
		Quality--;
	}
	if( gpGlobals->time >= DrinkData->TimeDrinkStart + DrinkData->flDrinkDelay )
	{
		DrinkData->fDrinking = false;
		CallScriptEvent( "game_drink_done" );
		if( Quality <= 0 )
		{
			#ifdef VALVE_DLL
				DelayedRemove( );
			#endif
		}

		return;
	}
}
void CGenericItem::DrinkCancel( )
{
	DrinkableCheck;

	DrinkData->fDrinking = false;
}

void CGenericItem::DrinkSetAmt( int iAmount )
{
	DrinkableCheck;

	Quality = iAmount;
}
int CGenericItem::DrinkGetAmt( )
{
	if( !DrinkData ) return 0;

	return Quality;
}