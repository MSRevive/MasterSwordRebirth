/*
	Manages netcode for receieving MS characters
*/
#ifdef _WIN32
#include "winsock2.h"
#include "ws2tcpip.h"
#else
#include "sys/socket.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

typedef unsigned short WORD;
#define WSADESCRIPTION_LEN 256
#define WSASYS_STATUS_LEN 128
#define WSACleanup()

typedef struct WSAData
{
	WORD wVersion;
	WORD wHighVersion;
	char szDescription[WSADESCRIPTION_LEN + 1];
	char szSystemStatus[WSASYS_STATUS_LEN + 1];
	unsigned short iMaxSockets;
	unsigned short iMaxUdpDg;
	char *lpVendorInfo;
} WSADATA;

#define IOCPARM_MASK 0x7f  /* parameters must be < 128 bytes */
#define SOCKET_ERROR            (-1)
#define INVALID_SOCKET			(-1)
#define closesocket close

typedef unsigned long u_long;

#define SIO_GET_INTERFACE_LIST _IOR('t', 127, u_long)

typedef struct sockaddr_in SOCKADDR_IN;
typedef int SOCKET;

typedef union sockaddr_gen
{
	struct sockaddr Address;
	struct sockaddr_in AddressIn;
	struct sockaddr_in AddressIn6;
} sockaddr_gen;

typedef struct _INTERFACE_INFO
{
	u_long iiFlags;					 /* Interface flags */
	sockaddr_gen iiAddress;			 /* Interface address */
	sockaddr_gen iiBroadcastAddress; /* Broadcast address */
	sockaddr_gen iiNetmask;			 /* Network mask */
} INTERFACE_INFO;
#endif

const char *GetWSAErrorCodeString();
void SetSocketBlocking(SOCKET Socket, bool Enabled);

#include "sharedutil.h" //MS generic utility functions
typedef void TransationCallbackFunc(class CNetFileTransaction *pTransaction);

class CNetFileTransaction
{
public:
	struct fileheader_t //File Header  - First thing sent
	{
		ulong FileID;	//Player to associate this file with (recv)
		ulong DataSize; //Data size
	};
#define NET_FILEHEADERSIZE sizeof(CNetFileTransaction::fileheader_t)
#define NET_CONNECTTIMEOUT 8 //8 second timeout for file connection
#define NET_TIMEOUT 60		 //60 second timeout for connections
#define NET_RETRYDELAY 2	 //2 second delay between connect retries

	struct props : public fileheader_t
	{
		bool fRecv;							   //True == Receive				| False = Send
		bool Connected;						   //Recv: Received header yet?	| Send: Received first ack packet?
		ulong Ofs;							   //Current offset & amount sent/received
		byte *Data;							   //File data
		SOCKET socket;						   //File's socket
		float TimeTimeout;					   //When this connection times out
		TransationCallbackFunc *TransCallback; //Callback when operation is finished
		msstring DestAddr;					   //Send: Destination address for connection.  Format: Host:Port
		float TimeRetryConnect;				   //Send: When another connection attempt should be made
	} m;

	CNetFileTransaction(SOCKET sock, TransationCallbackFunc pTransCallback = NULL);												   //Receiving file.  Use the socket returned by accept
	CNetFileTransaction(const msstring_ref sDestAddr, byte *pData, ulong lDataSize, TransationCallbackFunc pTransCallback = NULL); //Sending file
	void Think();
	void Shutdown();
};

class CNetCode
{
public:
	CNetCode();
	virtual bool Init();										 //Init
	virtual void Think();										 //Checks the listening socket for incoming connections by clients
	virtual void Shutdown();									 //Shutdown
	virtual void RemoveTransaction(CNetFileTransaction *pTrans); //File is done

	struct props
	{
		msstring HostIP;						   //Detected server IP
		mslist<CNetFileTransaction *> Transactons; //Collection of all currently incoming files.  Once they are finished sending, they are removed from this list
	} m;
	static CNetCode *pNetCode;
	static void InitNetCode();
	//Resolves address:port string to a sockaddr_in struct
	static bool ResolveAddress(const char *pszAddress, ushort DefaultPort, sockaddr_in &outSockAddr);
};

#ifdef VALVE_DLL
#ifdef NET_SERVER_H
#define g_NetCode (*(CNetCodeServer *)CNetCode::pNetCode)
#else
#define g_NetCode (*CNetCode::pNetCode)
#endif
#else
#define g_NetCode (*CNetCode::pNetCode)
#endif