/*
	Manages netcode for receieving MS characters
*/

#ifndef NET_SERVER_H
#define NET_SERVER_H

#include "msnetcode.h"

class CNetCodeServer : public CNetCode
{
public:
	CNetCodeServer();
	bool Init();											 //Init
	void Think();											 //Checks the listening socket for incoming connections by clients
	void Shutdown();										 //Shutdown
	msstring_ref GetServerIPForPlayer(CBasePlayer *pPlayer); //Returns the IP the client should use to connect to the server [lan ip vs internet ip]
	//static CBasePlayer *CNetCode::GetPlayerByAddress( const char *Address );	//Address must be IP:Port
	static CBasePlayer *GetPlayerByFileID(ulong ID); //Finds client from ID.  The client first sends this ID to server via the "savefileid" cmd

	struct props
	{
		vector<INTERFACE_INFO> Interfaces; //All detected server interfaces
		SOCKET FileSock;				   //Socket for receiving files
		ulong FilePort;					   //Port for receiving files
	} s;
	static CNetCodeServer NetCode;
};
extern SOCKET g_PingSock;

#endif
