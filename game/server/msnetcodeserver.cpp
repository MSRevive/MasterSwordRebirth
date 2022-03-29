/*
	Manages netcode for receieving MS characters
*/

#include "msdllheaders.h"
#include "weapons/genericitem.h"
#include "msitemdefs.h"
#include "msnetcodeserver.h"
#include "stats/stats.h"
#include "mscharacter.h"
#include "svglobals.h"
#include "teams.h"
#include "logfile.h"

//Callback for save file transaction
void ReceivedSaveFile(CNetFileTransaction *pTransaction);
CGenericItem *ReadItem(CPlayer_DataBuffer &Data, bool fStripItems, Vector &vOrigin);

//Initailizes the network
void CNetCode::InitNetCode()
{
	pNetCode = msnew CNetCodeServer();
	pNetCode->Init();
}

CNetCodeServer::CNetCodeServer() : CNetCode() { m.HostIP = "127.0.0.1"; }

//Finds the server IP and enumerates the network interfaces
bool CNetCodeServer::Init()
{
	if (!CNetCode::Init())
		return false;

	//GetHostName
	/*char cHostName[64];
	if( gethostname( cHostName, 64 ) == SOCKET_ERROR )
	{
		MSErrorConsoleText( "CNetCode::Init", "gethostname failed." );
		return false;
	}
	logfile << "[Net] Local hostname: " << cHostName << "\r\n";

	//GetHostByName
	hostent *Host = gethostbyname( cHostName );
	if( !Host )
	{
		MSErrorConsoleText( "CNetCode::Init", msstring("gethostbyname(") + cHostName + ") failed." );
		return false;
	}

	//Create the socket
	s.FileSock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	//Get Interface list (Winsock 2.0 required)
	INTERFACE_INFO InterfaceList[32];
	int iNumInterfaces;
	unsigned long nBytesReturned = 0;
	int ret;

#ifdef _WIN32
	ret = WSAIoctl( s.FileSock, SIO_GET_INTERFACE_LIST, NULL, NULL, InterfaceList, sizeof(InterfaceList), &nBytesReturned, NULL, NULL );
#else
	ret = ioctlsocket( s.FileSock, SIO_GET_INTERFACE_LIST, NULL, NULL, InterfaceList, sizeof(InterfaceList), &nBytesReturned, NULL, NULL );
#endif
	if( ret != SOCKET_ERROR )
	{
		iNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);
		s.Interfaces.reserve( iNumInterfaces );

		logfile << "[Net] Interfaces: " << iNumInterfaces << "\r\n";
		for( int i = 0; i < iNumInterfaces; i++ )
		{
			logfile << "[Net] Address: " << inet_ntoa(((SOCKADDR_IN*)&InterfaceList[i].iiAddress)->sin_addr) << (InterfaceList[i].iiFlags&IFF_LOOPBACK ? " loopback\r\n" : "\r\n");
			s.Interfaces.push_back( InterfaceList[i] );
		}
	}
	else {
		MSErrorConsoleText( "CNetCode::Init", "WSAIoctl failed." );
		//Don't make this fatal - Linux version doesn't work
	}

	//Use LAST host addr as the internet one
	//LAN IPs are used after any internet ones, and 0.0.0.0 is never used 
	//(If 0.0.0.0 is the only one, then the default, 127.0.0.1 is used)
	int i = 0;
	while( Host->h_addr_list[i] )
	{
		char cTestIP[32];
		strncpy( cTestIP, inet_ntoa( *(in_addr *)Host->h_addr_list[i] ), sizeof(cTestIP) );
		if( i > 0 || m.HostIP == "0.0.0.0" )
		{
			//if( strstr(CVAR_GET_STRING("net_address"),cTestIP) )
			//	continue;
			char cFirstNum[4];
			strncpy( cFirstNum, cTestIP, 3 );
			cFirstNum[3] = 0;
			if( !stricmp(cFirstNum,"192") ||
				!stricmp(cFirstNum,"169") ||
				!stricmp(cFirstNum,"10.") ||
				!stricmp(cFirstNum,"127") )
			{
				i++;
				continue;
			}
		}
		

		m.HostIP = cTestIP;
		i++;
	}

	logfile << "[Net] Server IP: " << m.HostIP.c_str() << "\r\n";

	CVAR_SET_STRING( "ms_ip", m.HostIP.c_str() );

	unsigned long BindAddr = inet_addr("0.0.0.0");
	sockaddr_in m_AddrIn;
	memset( &m_AddrIn.sin_zero, 0, 8 );
	m_AddrIn.sin_family = AF_INET;

	//Set up SaveFile recv port (STREAM socket)
	//Same port as server, or port 27015 by default
	s.FilePort = CVAR_GET_FLOAT("port");
	if( s.FilePort <= 0 ) s.FilePort = 27015;

	m_AddrIn.sin_port = htons(s.FilePort);
	m_AddrIn.sin_addr.s_addr = htonl(INADDR_ANY);

	//Create the socket
	s.FileSock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	ret = bind( s.FileSock, (sockaddr *)&m_AddrIn, sizeof(sockaddr) );
	if( ret == SOCKET_ERROR )
	{
		MSErrorConsoleText( "CNetCode::Init", "bind failed." );
		return false;
	}

	//Disable socket blocking
	SetSocketBlocking( s.FileSock, false );

	//Listen
	ret = listen( s.FileSock, 5 );
	if( ret == SOCKET_ERROR ) 
	{
		MSErrorConsoleText( "CNetCode::Init", "listen failed." );
		return false;
	}*/

	return true;
}

msstring_ref CNetCodeServer::GetServerIPForPlayer(CBasePlayer *pPlayer)
{
	//Finds the server address that this particular client should use.
	//This is different for lan clients than internet clients (unless for some reason your lan uses your internet IP)
	char cPlayerIP[32];
	 strncpy(cPlayerIP,  pPlayer->m_ClientAddress, sizeof(cPlayerIP) );
	*strstr(cPlayerIP, ":") = 0;
	ULONG plyrAddr = inet_addr(cPlayerIP);

	for (int i = 0; i < s.Interfaces.size(); i++)
	{
		//Find an interface that masks correctly with the client IP address
		//If a client is coming from lan, we'll select the proper lan IP the client should use to contact the server
		//If not, it'll fall through to MS_IP
		INTERFACE_INFO &Interface = s.Interfaces[i];
		ulong InterfaceAddr = *(ULONG *)&Interface.iiAddress.AddressIn.sin_addr;
		ulong InterfaceMask = *(ULONG *)&Interface.iiNetmask.AddressIn.sin_addr;
		ulong addr_result = InterfaceAddr & InterfaceMask;
		ulong plyr_result = plyrAddr & InterfaceMask;
		if (addr_result != INADDR_ANY && addr_result == plyr_result)
			return inet_ntoa(Interface.iiAddress.AddressIn.sin_addr);
	}

	return CVAR_GET_STRING("ms_ip");
}

/*CBasePlayer *CNetCode::GetPlayerByAddress( const char *Address )
{
	//The format for Address is IP:Port
	//Don't call this with just the IP!
	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( i );
		if( !pPlayer ) continue;
		
		if( !strcmp(pPlayer->m_ClientAddress, Address) ) 
			return pPlayer;
	}

	return NULL;
}*/
CBasePlayer *CNetCodeServer::GetPlayerByFileID(ulong ID)
{
	//The format for Address is IP:Port
	//Don't call this with just the IP!
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (!pPlayer)
			continue;

		if (pPlayer->m_SaveFileID == ID)
			return pPlayer;
	}

	return NULL;
}

//Checks the listening socket for incoming connections by clients
void CNetCodeServer::Think()
{
	sockaddr_in addr;
	int addrlen = sizeof(sockaddr);
	memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));
	addrlen = sizeof(sockaddr);

	//Attempt to accept a client connection for receiving a file

	SOCKET newsock = accept(s.FileSock, (sockaddr *)&addr, &addrlen);

	if (newsock != INVALID_SOCKET)
	{
		m.Transactons.push_back(msnew CNetFileTransaction(newsock, ReceivedSaveFile)); //Create incoming file transaction
		byte SendByte = 1;															   //Send the first ack packet
		send(newsock, (char *)&SendByte, 1, 0);										   //

		char Address[32];
		_snprintf(Address, sizeof(Address), "%s:%i", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

		logfile << "[Net] Incoming savefile connection from: " << Address << "\r\n";
	}

	CNetCode::Think();
}

void CNetCodeServer::Shutdown()
{
	if (s.FileSock)
		closesocket(s.FileSock);
	CNetCode::Shutdown();
}

void ReceivedSaveFile(CNetFileTransaction *pTransaction)
{
	//Finished receiving file
	CBasePlayer *pPlayer = CNetCodeServer::GetPlayerByFileID(pTransaction->m.FileID);
	pPlayer->RestoreAllServer(pTransaction->m.Data, pTransaction->m.DataSize);
}

//Called when the server has received the entire save file
#define TAMPER_ERROR ALERT(at_console, "%s may be tampering with their save file!!\n", STRING(pev->netname))
//#include "monsters/bodyparts/bodyparts_Human.h"
bool CBasePlayer::RestoreAllServer(void *pData, ulong Size)
{
	startdbg;
	dbg("Begin");

	Log("Load Character: %s", DisplayName());

	//Thothie JAN2010_10 - flag to tell "char" function character is loaded, so no clickie
	m_CharacterState = CHARSTATE_LOADING;

	//Thothie JUL2007 - prevent loading of STEAM_ID_PENDING chars
	msstring thoth_displayname = DisplayName();
	if (thoth_displayname.starts_with("LOAD_FAILED-RECONNECT"))
	{
		KickPlayer("\nNo clicky means no clicky. Please reconnect.\n");
		return false;
	}

	RemoveAllItems(false, true);				  //Remove all items
	CallScriptEvent("game_reset_wear_positions"); //Re-initialize all the wearable positions for items

	chardata_t Data;
	if (!MSChar_Interface::ReadCharData(pData, Size, &Data))
	{
		KickPlayer("\nTampering with the save file results in a ban!\n");
		return false;
	}

	//Read Maps Visited

	m_Maps = Data.m_VisitedMaps;

	m_MapStatus = INVALID_MAP;
	char cCurrentMap[32];
	 strncpy(cCurrentMap,  STRING(gpGlobals->mapname), sizeof(cCurrentMap) );
	strncpy(m_NextMap, cCurrentMap, 32);

	//Determine whether a transition took place and set the spawn transition accordingly
	if (FStrEq(Data.MapName, cCurrentMap))
	{
		m_MapStatus = OLD_MAP;
		strncpy(m_OldTransition, Data.OldTrans, 32); //Copy transition names to savable memory
	}
	else if (FStrEq(Data.NextMap, cCurrentMap))
	{
		m_MapStatus = NEW_MAP;
		strncpy(m_OldTransition, Data.NewTrans, 32); //The new transition becomes the old transition
	}

	m_SpawnTransition = m_OldTransition;
	m_NextTransition[0] = 0;
	m_NextMap[0] = 0;

	//Copy the data

	m_DisplayName = Data.Name;
	pev->netname = 0;
	//msstring String = msstring("\"name ") + Data.Name + "\"\n";

	//Force the client to do the name command
	//This is so the engine will run its name check routine and append (x) if people have the same names
	//I need that check because if two people were to forecefully get assigned the same name (with g_engfuncs.pfnSetClientKeyValue),
	//Then the whole server does the "5 minute delayed messages" bug
	CLIENT_COMMAND(edict(), "name %s\n", Data.Name);
	//g_engfuncs.pfnSetClientKeyValue( entindex(), g_engfuncs.pfnGetInfoKeyBuffer( edict() ), "name", (char *)Data.Name );

	dbg("Read Stats");
	m_OldGold = m_Gold = Data.Gold;

	//MiB JAN2010_15 Gold Change on Spawn.rtf
	
	MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_SETSTAT], NULL, pev);
	WRITE_BYTE(3);
	WRITE_BYTE(0);
	WRITE_LONG(m_Gold);
	MESSAGE_END();

	if (Data.HP > 0)
	{
		float CappedHP = max(Data.HP, 0);
		float CappedMP = max(Data.MP, 0);
		pev->health = m_HP = CappedHP;
		m_MP = CappedMP;
		m_MaxHP = Data.MaxHP;
		m_MaxMP = Data.MaxMP;
	}
	//Player saved while dead
	else
		pev->deadflag = DEAD_DEAD;

	 strncpy(m_cEnterMap,  Data.MapName, sizeof(m_cEnterMap) );

	SetTeam(CTeam::CreateTeam(Data.Party, Data.PartyID));
	m_Gender = Data.Gender;
	m_fIsElite = Data.IsElite ? true : false;
	m_PlayersKilled = Data.PlayerKills;
	m_TimeWaitedToForgetKill = Data.TimeWaitedToForgetKill;
	m_TimeWaitedToForgetSteal = Data.TimeWaitedToForgetSteal;

	//MiB JAN2010 - Player Kill Stickiness.rtf
	MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_CLDLLFUNC], NULL, pev);
	WRITE_BYTE(8);
	WRITE_SHORT(m_PlayersKilled);
	MESSAGE_END();

	m_JoinType = MSChar_Interface::CanJoinThisMap(Data, m_Maps);

	//Create our Human body -- Must be done here
	if (Body)
		Body->Delete(); //Moved to SUB_Remove
	Body = msnew CHumanBody;
	//Body->Initialize( this );

	//Read skills
	m_Stats = Data.m_Stats;

	//MiB Aug 2008a (JAN2010_15) - Reset tomes on client
	MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_SETPROP], NULL, pev);
	WRITE_BYTE(PROP_SPELL);
	WRITE_BYTE(-1);
	MESSAGE_END();

	//Read Magic spells
	for (int s = 0; s < Data.m_Spells.size(); s++)
		LearnSpell(Data.m_Spells[s]);

	mslist<CGenericItem *> Items; //Keep track of ALL items, for quickslot assignment later

	//Read Items
	dbg("Read Items");
	for (int i = 0; i < Data.m_Items.size(); i++)
	{
		CGenericItem *pItem = Data.m_Items[i].operator CGenericItem *();

		if (pItem->m_Location == ITEMPOS_HANDS)
			AddItem(pItem, true, false, pItem->m_Hand);
		else
			pItem->AddToOwner(this);

		Items.add(pItem);
		for (int c = 0; c < pItem->Container_ItemCount(); c++)
			Items.add(pItem->Container_GetItem(c));

		//if( FBitSet(pItem->MSProperties(), ITEM_WEARABLE) && pItem->IsWorn() )	//Wear the wearable items
		//	pItem->WearItem( );
	}

	//Read storage items
	m_Storages = Data.m_Storages;
	Storage_Send(); //Send all the items in storage

	//Read Companions
	dbg("Read Companions");

	m_Companions = Data.m_Companions;
	//Thothie JUN2008a - just read in companions, save the summoning until the script command "summonpets"
	//- scratch the above, if the player saves while his pet is not present, it corrupts the pet and character
	for (int c = 0; c < m_Companions.size(); c++)
	{
		companion_t &Companion = m_Companions[c];

		CMSMonster *pCompanion = (CMSMonster *)CREATE_ENT("ms_npc");
		if (!pCompanion)
			continue;
		Companion.Active = true;
		Companion.Entity = pCompanion;

		pCompanion->StoreEntity(this, ENT_OWNER);
		edict_t *pEdict = pCompanion->edict();
		pCompanion->Spawn(Companion.ScriptName);
		if (pEdict->free)
			continue;

		pCompanion->pev->origin = pev->origin + Vector(0, 0, 128);

		IScripted *pScripted = pCompanion->GetScripted();
		if (!pScripted || !pScripted->m_Scripts.size())
			continue;
		for (int v = 0; v < Companion.SaveVarName.size(); v++)
			pScripted->SetScriptVar(Companion.SaveVarName[v].c_str(), Companion.SaveVarValue[v].c_str());

		pScripted->CallScriptEvent("game_companion_restore");
	}

	//Read Help tips
	m_ViewedHelpTips = Data.m_ViewedHelpTips;

	//Read Quests
	m_Quests = Data.m_Quests;

	//Read QuickSlots
	for (int q = 0; q < Data.m_QuickSlots.size(); q++)
	{
		quickslot_t &QuickSlot = Data.m_QuickSlots[q];
		if (QuickSlot.Active && QuickSlot.Type == QS_ITEM)
			for (unsigned int i = 0; i < Items.size(); i++)
				if (Items[i]->m_OldID == QuickSlot.ID)
				{
					QuickSlot.ID = Items[i]->m_iId;
					break;
				}
		m_QuickSlots[q] = QuickSlot;
	}

	//if( !fSuccess ) TAMPER_ERROR;
	//Print( "Loaded character. (%s)(%i/%i)\n", Data.Name, FileSize, sizeof(savedata_t) );

	//Make sure an update is sent from UpdateClientData ASAP
	for (int i = 0; i < m_Stats.size(); i++)
		m_Stats[i].OutDate();

	m_CharacterState = CHARSTATE_LOADED;

	//Send the character name down to client
	MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_CLDLLFUNC], NULL, pev);
	WRITE_BYTE(4);
	WRITE_STRING_LIMIT(Data.Name, 32);
	MESSAGE_END();

	dbg("Call CBasePlayer::Spawn()");
	Spawn();

	enddbg;

	return true;
}
