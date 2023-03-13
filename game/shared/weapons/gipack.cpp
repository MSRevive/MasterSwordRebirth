/***
*
*	Copyright (c) 2000, Kenneth "Dogg" Early.
*	
*	Email kene@maverickdev.com or
*		  l33tdogg@hotmail.com
*
****/
//////////////////////////////////
//	Special container behavior  //
//////////////////////////////////
#pragma warning(disable : 4800) // forcing int to bool

#include "inc_weapondefs.h"
#include "vgui_menudefsshared.h"
#include "logger.h"
#include "strutil.h"

//#define LOG_EXTRA

#ifdef LOG_EXTRA
#define logfileopt logfile
#else
#define logfileopt NullFile
#endif

#ifndef VALVE_DLL
void ShowVGUIMenu(int iMenu);
void ContainerWindowUpdate();
#endif

struct packdata_t
{
	bool fClosed, fCanClose, fClientUpdated;
	int /*Type, */ iLockStrength, MaxItems;
	CItemList ItemList;
	float Volume;
	//Not setting these means accept all items
	msstringlist AcceptItemsTypes;
	msstringlist RejectItemsTypes;
	~packdata_t()
	{
		for (int i = ItemList.size() - 1; i >= 0; i--) //Go backwards - sure to delete each item
		{
			ItemList[i]->RemoveFromContainer();
			ItemList[i]->SUB_Remove();
		}
	}
};

void CGenericItem::RegisterContainer()
{
	if (PackData)
		delete PackData;

	PackData = new (packdata_t);

	/*	if( !stricmp(GetFirstScriptVar("CONTAINER_TYPE"),"quiver") ) PackData->Type = PACK_QUIVER;
	else if( !stricmp(GetFirstScriptVar("CONTAINER_TYPE"),"sheath") ) PackData->Type = PACK_SHEATH;
	else PackData->Type = PACK_NORMAL;
	PackData->Volume = atof(GetFirstScriptVar("reg.container.space"));*/
	PackData->fCanClose = (bool)atoi(GetFirstScriptVar("reg.container.canclose"));
	PackData->iLockStrength = atof(GetFirstScriptVar("reg.container.lock_str"));
	PackData->MaxItems = atof(GetFirstScriptVar("reg.container.maxitem"));

#define CONTAINER_ITEM_ACCEPT "reg.container.accept_mask"
#define CONTAINER_ITEM_REJECT "reg.container.reject_mask"

	const char *pszItemList = "";
	if (strcmp(pszItemList = GetFirstScriptVar(CONTAINER_ITEM_ACCEPT), CONTAINER_ITEM_ACCEPT))
		TokenizeString(pszItemList, PackData->AcceptItemsTypes);
	if (strcmp(pszItemList = GetFirstScriptVar(CONTAINER_ITEM_REJECT), CONTAINER_ITEM_REJECT))
		TokenizeString(pszItemList, PackData->RejectItemsTypes);

	PackData->fClosed = true;

	SetBits(Properties, ITEM_CONTAINER);
}
/*int CGenericItem::Container_Type( )
{
	if( !PackData ) return 0;

	return PackData->Type;
}*/
int CGenericItem::Container_ItemCount()
{
	if (!PackData)
		return 0;

	/*
	//Trying to find a good place to dump inventory data to log for checking
	if ( PackData->iLockStrength != 44 )
	{
		//logfile << "======= PACK: " << m_Name << "======== \n";
		 for (int i = 0; i < PackData->ItemList.size(); i++) 
		{
			//THOTHIEDEBUG
			Print("Checking %i in %s", i, m_Name);
			CGenericItem *pItem = PackData->ItemList[ i ];
			if ( pItem )
			{
				logfile << "PACK_DATA: " << pItem->m_Name << "\n";
				PackData->iLockStrength = 44;
			}
		}
	}
	*/

	return PackData->ItemList.size();
}

float CGenericItem::Container_Weight()
{
	if (!PackData)
		return 0;

	//MIB JAN2010_27 - Bag of Holding Fix
	if (atoi(GetFirstScriptVar("CONTAINER_BOH")) == 1)
		return 0;

	return PackData->ItemList.FilledVolume();
}

void CGenericItem::SetLockStrength(int iNewStrength)
{
	PackData->iLockStrength = iNewStrength;
}

void CGenericItem::Container_Open()
{
	if (!PackData)
		return;

		//Thothie NOTE: This only goes off for the first container in inventory when inventory is activated
		//Thothie FEB2008a - attempting to make use of locked containers
#ifdef VALVE_DLL
	if (PackData->iLockStrength != 0)
	{
		static msstringlist Params;
		Params.add(EntToString(Owner()));
		CallScriptEvent("game_attempt_unlock", &Params);
	}
#endif

	bool bOpened = !PackData->fClosed;
	if (!bOpened) //Closed, try to open it
	{
		//Locked ?
		if (PackData->iLockStrength == 0)
			bOpened = true;
	}

	if (bOpened)
	{
		CallScriptEvent("game_opencontainer");
		PackData->fClosed = false;
	}
}
bool CGenericItem::Container_IsOpen()
{
	return !PackData->fClosed;
}

CGenericItem *CGenericItem::Container_GetItem(int iIndex)
{
	if (!PackData)
		return NULL;

	CGenericItem *pItem = PackData->ItemList[iIndex];

	return pItem;
}

void CGenericItem::Container_UnListContents()
{
	if (!PackData || !m_pPlayer)
		return;

#ifndef VALVE_DLL
	m_pPlayer->ClearConditions(MONSTER_OPENCONTAINER);
	ContainerWindowUpdate();
#endif
}
void CGenericItem::Wearable_RemoveFromOwner()
{
}
void CGenericItem::Wearable_WearOnOwner()
{
	//This function should be moved because its used for both
	//packs and armor (any wearables)
	/*#ifdef VALVE_DLL
	if( !m_pPlayer ) return;

	bool fAddOrRemove = m_Location ? true : false;

	if( fAddOrRemove )
	{
		//If this is a sheath, show all models inside (there should only be one...)
		if( Container_Type() == PACK_SHEATH )
		{
			int i = 0;
			CGenericItem *pItem;
			while( pItem = Container_GetItem( i ) )
			{
				if( m_pOwner )
				{
					ClearBits( pItem->pev->effects, EF_NODRAW );
					pItem->pev->movetype = MOVETYPE_FOLLOW;
					pItem->pev->aiment = m_pOwner->edict();
					pItem->Script->RunScriptEventByName( "game_sheathed" );
				}
				i++;
			}
		}
	}
#endif*/
}
void CGenericItem::Container_SendContents()
{
	//Send all items down to the client
	for (int i = 0; i < Container_ItemCount(); i++)
		Container_SendItem(Container_GetItem(i), true);
}
void CGenericItem::Wearable_ResetClientUpdate()
{
	//This function should be moved because its used for both
	//packs and armor (any wearables)
#ifdef VALVE_DLL
	if (!PackData)
		return;

		/*if( !m_Location )
	{
		//If this is a sheath, hide all models inside (there should only be one...)
		if( Container_Type() == PACK_SHEATH )
		{
			int i = 0;
			CGenericItem *pItem;
			while( pItem = Container_GetItem( i++ ) )
				SetBits( pItem->pev->effects, EF_NODRAW );
		}
	}*/
#endif
}
/*
	Check if this container can accept the specified item
*/
bool CGenericItem::Container_CanAcceptItem(CGenericItem *pItem)
{
	if (!PackData || !pItem || pItem == this)
		return false;

#ifdef VALVE_DLL
	//Some packs (like sheathes) can only hold a certain amount of items
	if (PackData->MaxItems && Container_ItemCount() + 1 > PackData->MaxItems)
	{
		//MiB FEB2010_13 - Making this into an if-block - If the pack is "full" we check to see
		//		if the item is groupable and if it can be fully grouped into another.
		if (FBitSet(pItem->Properties, ITEM_GROUPABLE))
		{
			for (int i = 0; i < PackData->ItemList.size(); i++)
			{
				CGenericItem *pCur = Container_GetItem(i);

				//Thothie FEB2011_03 - don't stack with items in hands
				/*
				if ( pItem->m_Location == ITEMPOS_HANDS )
				{
					continue;
				}
				*/

				if (msstring(pCur->m_Name) != msstring(pItem->m_Name)) //If it has the same script name
					continue;

				if (!FBitSet(pCur->Properties, ITEM_GROUPABLE)) //If it's groupable (Paranoia)
					continue;

				//Thothie - No stack size limit
				//if ( pCur->iQuantity + pItem->iQuantity > pCur->m_MaxGroupable ) //If it won't over-fill the stack
				//	continue;

				return true; //It's ok to put the item in here. It will all get absorbed into the stack.
			}
		}
		return false; //Wasn't groupable or couldn't find a suitable group.
	}

	//Does this pack accept this type of item?
	bool fAccepted = true; //Default to true, in case if AcceptItemTypes has no members

	if (PackData->RejectItemsTypes.size())
	{
		if (PackData->RejectItemsTypes[0] == "all")
			fAccepted = false;
		else
		{
			for (int i = 0; i < PackData->RejectItemsTypes.size(); i++)
			{
				if (strstr(pItem->ItemName, PackData->RejectItemsTypes[i]))
				{
					fAccepted = false;
					break;
				}
			}
		}
	}

	//This accept overrules a reject
	if (PackData->AcceptItemsTypes.size())
		fAccepted = false;

	for (int i = 0; i < PackData->AcceptItemsTypes.size(); i++)
	{
		//fAccepted = false;
		if (strstr(pItem->ItemName, PackData->AcceptItemsTypes[i]))
		{
			fAccepted = true;
			break;
		}
	}

	if (!fAccepted)
		return false;

	if (!PackData->ItemList.CanAddItem(pItem))
		return false;

#endif

	return true;
}
int CGenericItem::Container_AddItem(CGenericItem *pItem)
{
	if (!Container_CanAcceptItem(pItem))
		return 0;

	startdbg;

	/*
	//Thothie MAR2010_15 - trying to restore stackable stacks sans char corruption
	if ( FBitSet( pItem->Properties, ITEM_GROUPABLE ) )
	{
		dbg("Stack Attempt");
		CBasePlayer	*pOwner = (CBasePlayer *)m_pOwner;
		if ( pOwner )
		{
			if ( pOwner->m_CharacterState == CHARSTATE_LOADED )
			{
				//Only if I r loaded and in world
				 for (int i = 0; i < PackData->ItemList.size(); i++) 
				{
					CGenericItem *pCur = Container_GetItem( i );
					if ( msstring(pCur->m_Name) != msstring( pItem->m_Name ) ) //If it has the same script name
						continue;

					if ( !FBitSet( pCur->Properties , ITEM_GROUPABLE ) ) //If it's groupable (Paranoia)
						continue;

					pCur->iQuantity += pItem->iQuantity;
					Container_SendItem( pCur , true );
					return -1;
					break;
				}
			}
		}
	}
	dbg("Post Stack Attempt");
	*/

	PackData->ItemList.AddItem(pItem);

	pItem->m_pParentContainer = this;

	//Update client
	Container_SendItem(pItem, true);

	//Thothie FEB20080a
	//- This event goes off for every item the player in a container has at spawn
	//- making much lag on connect
	//- It'd be good if we could rig this so it'd only do so post-spawn (ie. when a player actually adds an item to a container)
	//- at the moment I'm commenting it out as doing so doesn't seem to mess with anything
	//- but it maybe a useful function in the future if we can get around the lag on connect issue
	//static msstringlist Params;
	//Params.clearitems( );
	//ifdef VALVE_DLL
	//	Params.add( EntToString(pItem) );
	//endif
	//CallScriptEvent( "game_container_addeditem", &Params );

	enddbg;

	return 1;
}
void CGenericItem::Container_SendItem(CGenericItem *pItem, bool fAddItem)
{
	//Thothie FEB2008a - might be nice to have a way of knowing what item player took from chest/shop?
	//- except this isn't a chest or shop >< (may also lag on spawn)
	/*#ifdef VALVE_DLL
		static msstringlist Params;
		msstring thoth_item_name = pItem->DisplayName();
		Params.add( EntToString(pItem) );
		Params.add( thoth_item_name.c_str() );
		CallScriptEvent( "game_container_gaveitem", &Params);
	#endif*/

#ifdef VALVE_DLL
	if (!m_pPlayer)
		return;

	//MIB SEP2007 - attempts to stop overflow
	//- fail, problem may lie somewhere else
	//for( int x = 0; x < 1000; x++ );
	//Print("Allowing item %s to pass now",pItem->m_Name);

	MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_ITEM], NULL, m_pPlayer->pev);
	WRITE_BYTE(fAddItem ? 3 : 4); //3 == Add to container | 4 == Remove from container
	WRITE_LONG(m_iId);
	if (fAddItem)
		SendGenericItem(m_pPlayer, pItem, false);
	else
		WRITE_LONG(pItem->m_iId);
	MESSAGE_END();
#endif
}

bool CGenericItem::Container_RemoveItem(CGenericItem *pItem)
{
	if (!PackData || !pItem || !PackData->ItemList.ItemExists(pItem))
		return false;

	if (!PackData->ItemList.RemoveItem(pItem))
		return false;

	//Update the client
	Container_SendItem(pItem, false);

	if (pItem->pev)
	{
		ClearBits(pItem->pev->effects, EF_NODRAW);
		if (pItem->pev->owner == edict())
			pItem->pev->owner = NULL;
	}

	if (pItem->m_pParentContainer == this)
		pItem->m_pParentContainer = NULL;

	static msstringlist Params;
	Params.clearitems();

#ifdef VALVE_DLL
	Params.add(EntToString(pItem));
#endif

	CallScriptEvent("game_container_removeditem", &Params);

	//pItem->CallScriptEvent( "removefrompack" );//old
	//pItem->CallScriptEvent( "game_removefrompack" );
	return true;
}
//Remove and destroy all items in the container
void CGenericItem::Container_RemoveAllItems()
{
	if (!PackData)
		return;

	while (PackData->ItemList.size())
	{
		CGenericItem *pItem = Container_GetItem(0);
		Container_RemoveItem(pItem);
		pItem->SUB_Remove();
	}
}
//Dallocate memory the container is using
void CGenericItem::Container_Deactivate()
{
	if (!PackData)
		return;
	PackData->ItemList.clear(); //Let my itemlist deallocate collection pointers
	delete PackData;			//Delete the packdata
	PackData = NULL;
}

//Thothie FEB2011_16 - Seperate container stack func
void CGenericItem::Container_StackItems()
{
	if (!PackData)
		return;

	for (int i1 = 0; i1 < PackData->ItemList.size(); i1++)
	{
		CGenericItem *pItem = Container_GetItem(i1);
		if (FBitSet(pItem->Properties, ITEM_GROUPABLE))
		{
			CBasePlayer *pOwner = (CBasePlayer *)m_pOwner;
			if (pOwner)
			{
				if (pOwner->m_CharacterState == CHARSTATE_LOADED)
				{
					//Only if I r loaded and in world
					for (int i2 = 0; i2 < PackData->ItemList.size(); i2++)
					{
						CGenericItem *pCur = Container_GetItem(i2);
						if (pCur == pItem)
							continue;

						if (msstring(pCur->m_Name) != msstring(pItem->m_Name)) //If it has the same script name
							continue;

						if (!FBitSet(pCur->Properties, ITEM_GROUPABLE)) //If it's groupable (Paranoia)
							continue;

						//if (pCur->iQuantity == 0)
						//{
						//	//PackData->ItemList.RemoveItem( pCur );
						//	Container_RemoveItem(pCur);
						//	pCur->RemoveFromOwner();
						//	continue;
						//}

						//Thothie FEB2010_13 - MiB says try this other way around
						pItem->iQuantity += pCur->iQuantity;
						if (pItem->iQuantity > 1500)
							pItem->iQuantity = 1500; //Thothie FEB2011_22 - cap stax at 1500

						//PackData->ItemList.RemoveItem( pCur );
						pCur->iQuantity = 0;
						Container_RemoveItem(pCur);
						pCur->RemoveFromOwner();
						pCur->DelayedRemove();
						Container_SendItem(pItem, true);
						//pCur->iQuantity += pItem->iQuantity;
						//Container_SendItem( pCur , true );
						//PackData->ItemList.RemoveItem( pItem );
					}
				}
			}
		}
	}
}