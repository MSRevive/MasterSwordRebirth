/*
	Manages netcode for receieving MS characters
*/

#include "msdllheaders.h"
#include "msnetcode.h"
#include "player/player.h"
#include "logger.h"

#ifndef VALVE_DLL
#include "hud.h"
#include "cl_util.h"
#endif

void SetSocketBlocking(SOCKET Socket, bool Enabled)
{
	//Disable socket blocking
#ifdef _WIN32
	u_long arg = !Enabled ? 1 : 0;
	ioctlsocket(Socket, FIONBIO, &arg);
#else
	fcntl(Socket, F_SETFL, (Enabled ? 0 : O_NONBLOCK));
#endif
}

CNetCode *CNetCode::pNetCode;

CNetCode::CNetCode() { m.HostIP = "127.0.0.1"; }

bool CNetCode::Init()
{
#ifdef _WIN32
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData))
	{
		MSErrorConsoleText("MSGlobalInit", "WSAStartup failed");
		return false;
	}
#endif

	return true;
}
bool CNetCode::ResolveAddress(const char *pszAddress, ushort DefaultPort, sockaddr_in &outSockAddr)
{
	msstring FullAddress = pszAddress;
	msstring Addr = FullAddress.thru_char(":");
	msstring PortStr = FullAddress.substr(Addr.len()).skip(":");

	ushort Port = DefaultPort;
	if (PortStr.len())
		Port = atoi(PortStr);

	ulong ulAddr = inet_addr(Addr);
	if (ulAddr != INADDR_NONE) //Is this an IP?
	{
		memcpy(&outSockAddr.sin_addr, &ulAddr, sizeof(ulAddr));
	}
	else
	{
		hostent *Host = gethostbyname(Addr); //Not an IP.  Try to resolve the hostname
		if (!Host)
		{
			//Not an IP or resolvable hostname
			return false;
		}
		memcpy(&outSockAddr.sin_addr, Host->h_addr, Host->h_length);
	}

	outSockAddr.sin_port = htons(Port);
	outSockAddr.sin_family = AF_INET;
	clrmem(outSockAddr.sin_zero);

	return true;
}

//Checks the listening socket for incoming connections by clients
void CNetCode::Think()
{
	//Use Iterator because the transactions might delete themselves during think

	mslist<CNetFileTransaction *> Transations = m.Transactons;

	for (int i = 0; i < Transations.size(); i++)
		Transations[i]->Think();
}
void CNetCode::RemoveTransaction(CNetFileTransaction *pTrans)
{
	for (int f = 0; f < m.Transactons.size(); f++)
		if (m.Transactons[f] == pTrans)
		{
			m.Transactons.erase(f);
			break;
		}
}

void CNetCode::Shutdown()
{
	for (int i = m.Transactons.size() - 1; i > 0; i--)
		m.Transactons[i]->Shutdown();

	WSACleanup();
}

// File Transaction
// ---- -----------

CNetFileTransaction::CNetFileTransaction(SOCKET sock, TransationCallbackFunc pTransCallback)
{
	clrmem(m);
	m.socket = sock;
	m.fRecv = true;
	m.TransCallback = pTransCallback;
}
CNetFileTransaction::CNetFileTransaction(const msstring_ref sDestAddr, byte *pData, ulong lDataSize, TransationCallbackFunc pTransCallback)
{
	clrmem(m);

	m.DestAddr = (const char *)sDestAddr;
	m.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	m.DataSize = lDataSize;
	m.Data = msnew byte[m.DataSize]; //Copy data locally
	memcpy(m.Data, pData, m.DataSize);
	m.FileID = RANDOM_LONG(200, 66666);	

#ifdef _WIN32
	u_long arg = 1;
	ioctlsocket(m.socket, FIONBIO, &arg); // Turn off socket blocking
#else
	fcntl(m.socket, F_SETFL, O_NONBLOCK);
#endif

	m.fRecv = false;
	m.TransCallback = pTransCallback;
	m.TimeTimeout = gpGlobals->time + NET_CONNECTTIMEOUT; // Connection time out
}

void CNetFileTransaction::Think()
{

	if (m.TimeTimeout && gpGlobals->time >= m.TimeTimeout)
	{
		logfile << Logger::LOG_INFO << "[Net] File TIMED OUT\r\n";
		Shutdown(); //Timed out
		return;
	}

	if (m.fRecv)
	{
		char Buffer[1024];

		//See how much data is waiting
		//If not initialized, only read up the size of our header
		int iAmt = recv(m.socket, Buffer, m.Connected ? sizeof(Buffer) : NET_FILEHEADERSIZE, 0);
		if (iAmt == SOCKET_ERROR)
			return;

		byte SendByte = 1;

		send(m.socket, (char *)&SendByte, 1, 0);

		if (!m.Connected)
		{
			if (iAmt < NET_FILEHEADERSIZE)
			{
				logfile << Logger::LOG_WARN << "[Net] Received bad header for incoming file (header too small). Rejecting file...\r\n";
				Shutdown();
				return;
			}

			fileheader_t FileHeader;
			memcpy(&FileHeader, Buffer, iAmt);

			//Sanity check.  If size is too large, this is a bogus file
			if (FileHeader.DataSize >= (1 << 16))
			{
				logfile << Logger::LOG_WARN << "[Net] Received bad header for incoming file (DataSize too big). Rejecting file...\r\n";
				Shutdown();
				return;
			}

			(*(fileheader_t *)&m) = FileHeader;
			m.Connected = true;
			m.Data = msnew(byte[m.DataSize]);
		}
		else
		{
			memcpy(&m.Data[m.Ofs], Buffer, iAmt);
			m.Ofs += iAmt;

			/*	if( iAmt > 0 )			//Print out every byte receieved
			 for (int i = 0; i < iAmt; i++) 
			{
				hex( logfile );
				logfile <<  "[" << (unsigned char)Buffer[i] << "]";
				dec( logfile );
			}*/

			if (m.Ofs >= m.DataSize)
			{
				//Done, parse file
				if (m.TransCallback)
					m.TransCallback(this);
				Shutdown();
				return;
			}
		}

		m.TimeTimeout = gpGlobals->time + NET_TIMEOUT; //Time out
	}
	else
	{

		//See how much data is waiting
		byte Buffer = 0;
		int iAmt = recv(m.socket, (char *)&Buffer, 1, 0);
		if (iAmt != SOCKET_ERROR)
			m.Connected = true; //Received a packet, I'm connected now

		if (!m.Connected)
		{
			if (gpGlobals->time < m.TimeRetryConnect)
				return;

#ifndef VALVE_DLL
			if (m.TimeRetryConnect)
				ConsolePrint(UTIL_VarArgs("Retrying data connect to %s...\n", m.DestAddr.c_str()));
#endif

			//Set up server addr
			sockaddr_in m_AddrIn;
			msstring_ref Port = "27015";
			msstring_ref Addr = "127.0.0.1";
			int idx = m.DestAddr.find(":");
			if (idx != msstring_error)
			{
				Addr = m.DestAddr.substr(0, idx);
				Port = m.DestAddr.substr(idx + 1);
			}

			logfile << Logger::LOG_INFO << "[Net] Connecting to server IP: " << Addr << ":" << Port << "....\r\n";

			m_AddrIn.sin_family = AF_INET;
			m_AddrIn.sin_port = htons((unsigned short)atoi(Port));
			ulong tmp = inet_addr(Addr);
			memcpy(&m_AddrIn.sin_addr, &tmp, sizeof(in_addr));
			memset(&m_AddrIn.sin_zero, 0, 8);

			connect(m.socket, (sockaddr *)&m_AddrIn, sizeof(sockaddr));

			m.TimeRetryConnect = gpGlobals->time + NET_RETRYDELAY; //Retry after delay
			return;
		}

		if (iAmt == SOCKET_ERROR)
			return;

		//Received ack of last packet, I'm done
		if (m.Ofs >= m.DataSize)
		{
			logfile << Logger::LOG_INFO << "Done sending char " << m.Ofs << "/" << m.DataSize << "\r\n";
			Shutdown();
			return;
		}

		if (!m.Ofs)
		{
			//Send the file header first
			CNetFileTransaction::fileheader_t FileHeader = m;
			send(m.socket, (const char *)&FileHeader, sizeof(FileHeader), 0);
		}

		int iSendSize = 128;
		if (m.Ofs + iSendSize > m.DataSize)
			iSendSize = (m.DataSize - m.Ofs);

		send(m.socket, (const char *)&m.Data[m.Ofs], iSendSize, 0);
		//logfile << "Send " << iSendSize << " bytes \r\n";

		m.Ofs += iSendSize;
		m.TimeTimeout = gpGlobals->time + NET_TIMEOUT; //Time out
	}
}
void CNetFileTransaction::Shutdown()
{
	closesocket(m.socket);
	if (m.Data)
	{
		delete m.Data;
		m.Data = NULL;
	}
	g_NetCode.RemoveTransaction(this);
	delete this;
}