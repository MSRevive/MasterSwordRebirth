#include "msdllheaders.h"
#include "msitemdefs.h"
#include "player/player.h"
#include "monsters/corpse.h"
#include "stats/stats.h"
#include "stats/statdefs.h"
#include "syntax/syntax.h"
#include "weapons/weapons.h"
#include "weapons/genericitem.h"
#include "titles.h"
#include "magic.h"
#include "scriptedeffects.h"
#include "mscharacter.h"
#include "script.h"
#include "global.h"
#include "logfile.h"

#ifndef VALVE_DLL
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#endif

#define UUENC_OFS 'A'

//Send char to client or to server (client-side characters only)
//Send to server at client connect
//send to client every few intervals (for client-side storage)
void CBasePlayer::SendChar(charinfo_base_t &CharBase)
{
	charsendinfo_t &SendInfo = m_CharSend;

	m_TimeCharLastSent = gpGlobals->time;
	SendInfo.Status = CSS_SENDING;
	SendInfo.Index = CharBase.Index;
	SendInfo.TimeDataLastSent = 0;
	SendInfo.DataSent = 0;
	if (SendInfo.Data)
		delete SendInfo.Data;

//Starting to send new char
#ifdef VALVE_DLL
	SendInfo.Data = msnew char[CharBase.DataLen];
	memcpy(SendInfo.Data, CharBase.Data, CharBase.DataLen);
	SendInfo.DataLen = CharBase.DataLen;

	//Notify client the char file is about to be sent
	MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_CLDLLFUNC], NULL, pev);
	WRITE_BYTE(18);
	WRITE_BYTE(SendInfo.Index);
	WRITE_LONG(SendInfo.DataLen);
	MESSAGE_END();

	hudtextparms_t htp;
	clrmem(htp);
	htp.x = 0.02;
	htp.y = 0.6;
	htp.effect = 0;
	htp.r1 = 90;
	htp.g1 = 90;
	htp.b1 = 90;
	htp.fadeinTime = 0.1;
	htp.fadeoutTime = 10.0;
	htp.holdTime = 10.0;
	htp.fxTime = 0.6;
	msstring Message = (!CurrentTransArea) ? "Saving..." : "Saving (transition)...";

	UTIL_HudMessage(this, htp, Message);

#else

	SendInfo.DataLen = CharBase.DataLen * 2;
	SendInfo.Data = msnew char[SendInfo.DataLen];

	for (int i = 0; i < CharBase.DataLen; i++)
	{
		byte Value = CharBase.Data[i];
		char Num1 = (Value / (uint)20) + UUENC_OFS;
		char Num2 = (Value % 20) + UUENC_OFS;

		SendInfo.Data[i * 2] = Num1;
		SendInfo.Data[(i * 2) + 1] = Num2;
	}

	//Notify server the char file is about to be sent
	msstring SendStr = msstring("ul new ") + SendInfo.Index + " " + SendInfo.DataLen + " " + CharBase.DataLen + "\n";
	ClientCmd(SendStr);
	ChooseChar_Interface::UpdateCharScreen();
#endif
}

void MSChar_Interface::Think_SendChar(CBasePlayer *pPlayer)
{
	startdbg;
	dbg("Begin");

	if (MSGlobals::ServerSideChar)
		return;

#ifdef VALVE_DLL
	if (pPlayer->m_CharacterState == CHARSTATE_UNLOADED)
		return;
	if (!pPlayer->IsAlive())
		return;
	//Thothie NOV2014_07 - work around for new character delete bug
	//...in the wrong spot - moved to MSChar_Interface::SaveChar
	/*
		if( pPlayer->MaxHP() < 15 )
		{
			//character didn't load proper, don't save
			return;
		}
		*/
	if (pPlayer->m_CharSend.Status == CSS_DORMANT)
	{
		//Not sending a char file.  Check if one should be start sending
		if (gpGlobals->time - pPlayer->m_TimeCharLastSent >= 30)
		{
			pPlayer->SendChar(pPlayer->m_CharInfo[pPlayer->m_CharacterNum]);
		}
	}
#endif

#ifdef VALVE_DLL
#define MAX_UL_SIZE 150
#define SEND_DELAY 0.8f
#else
#define MAX_UL_SIZE 64
#define SEND_DELAY 0.4f
#endif

	charsendinfo_t &SendInfo = pPlayer->m_CharSend;

	if (SendInfo.Status != CSS_SENDING)
		return;

	if (gpGlobals->time - SendInfo.TimeDataLastSent < SEND_DELAY)
		return;

	int DataLeft = SendInfo.DataLen - SendInfo.DataSent;
	if (DataLeft <= 0)
	{
		//Done sending
		//Print( "Sent char #%i to %s\n", SendInfo.Index, pPlayer->DisplayName() );
		delete SendInfo.Data;
		SendInfo.Data = NULL;
		SendInfo.Status = CSS_DORMANT;

#ifdef VALVE_DLL
		hudtextparms_t htp;
		clrmem(htp);
		htp.x = 0.02;
		htp.y = 0.6;
		htp.effect = 0;
		htp.r1 = 90;
		htp.g1 = 90;
		htp.b1 = 90;
		htp.fadeinTime = 0.1;
		htp.fadeoutTime = 3.0;
		htp.holdTime = 2.0;
		htp.fxTime = 0.6;
		UTIL_HudMessage(pPlayer, htp, "Save Complete");
#endif

		return;
	}

	int PacketDataSize = min(DataLeft, MAX_UL_SIZE);

#ifdef VALVE_DLL
	//Sending sequential data for a char
	MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_CLDLLFUNC], NULL, pPlayer->pev);
	WRITE_BYTE(11);
	WRITE_BYTE(PacketDataSize);
	for (int i = 0; i < PacketDataSize; i++)
		WRITE_BYTE(SendInfo.Data[SendInfo.DataSent++]);
	MESSAGE_END();
#else
	char Buffer[10 + MAX_UL_SIZE * 2];

	 strncpy(Buffer,  "ul ", sizeof(Buffer) );

	for (int i = 0; i < PacketDataSize; i++)
	{
		strncat(Buffer, &SendInfo.Data[SendInfo.DataSent++], 1);
	}

	strncat(Buffer, "\n", 1);
	ClientCmd(Buffer);

	if ((signed)SendInfo.DataSent >= SendInfo.DataLen)
	{
		//Done sending UUEncoded char to server
		//Now the server must acknowledge by sending the char info in a msg before Status goes back to dormant
		delete SendInfo.Data;
		SendInfo.Data = NULL;
		player.m_CharSend.Status = CSS_SENT;
	}
	ChooseChar_Interface::UpdateCharScreenUpload();
#endif

	SendInfo.TimeDataLastSent = gpGlobals->time;

	enddbg;
}

#ifdef VALVE_DLL
void MSChar_Interface::HL_SVNewIncomingChar(CBasePlayer *pPlayer, int CharIdx, uint UUEncodeLen, uint DataLen)
{
	charsendinfo_t &SendInfo = pPlayer->m_CharSend;

	SendInfo.Status = CSS_RECEIVING;
	if (SendInfo.Data)
		delete SendInfo.Data;
	SendInfo.DataLen = DataLen;
	SendInfo.Data = msnew char[SendInfo.DataLen];
	SendInfo.DataSent = 0;
	SendInfo.Index = CharIdx;
	SendInfo.TimeDataLastSent = 0;
}
void MSChar_Interface::HL_SVReadCharData(CBasePlayer *pPlayer, const char *UUEncodedData)
{
	startdbg;
	dbg("Begin");

	charsendinfo_t &SendInfo = pPlayer->m_CharSend;

	if (SendInfo.Status != CSS_RECEIVING)
		return;

	int Bytes = strlen(UUEncodedData) / 2;
	for (int i = 0; i < Bytes; i++)
	{
		char Num1 = UUEncodedData[i * 2] - UUENC_OFS;
		char Num2 = UUEncodedData[(i * 2) + 1] - UUENC_OFS;

		SendInfo.Data[SendInfo.DataSent++] = Num1 * 20 + Num2;
	}

	if ((signed)SendInfo.DataSent >= SendInfo.DataLen)
	{
		//Finished receieving file from client
		pPlayer->m_CharInfo[SendInfo.Index].AssignChar(SendInfo.Index, LOC_CLIENT, SendInfo.Data, SendInfo.DataLen, pPlayer);
		//Delete buffer
		delete SendInfo.Data;
		SendInfo.Data = NULL;
		SendInfo.Status = CSS_DORMANT;
	}

	enddbg;
}
#endif

#ifndef VALVE_DLL
void MSChar_Interface::CLInit()
{
	//If using client-side chars, load characters from client's computer
	if (!MSGlobals::ServerSideChar)
	{
		player.m_CanJoin = false;
		player.PreLoadChars();

		//If at least one char can join this map, notify the server... to ensure that it doesn't auto-switch maps on me
		if (player.m_CanJoin)
			ClientCmd("char canjoin");
	}
}
void MSChar_Interface::HL_CLNewIncomingChar(int CharIdx, uint DataLen)
{
	charsendinfo_t &SendInfo = player.m_CharSend;

	SendInfo.Status = CSS_RECEIVING;
	if (SendInfo.Data)
		delete SendInfo.Data;
	SendInfo.DataLen = DataLen;
	SendInfo.Data = msnew char[SendInfo.DataLen];
	SendInfo.DataSent = 0;
	SendInfo.Index = CharIdx;
	SendInfo.TimeDataLastSent = 0;
}

void MSChar_Interface::HL_CLReadCharData()
{
	startdbg;
	dbg("Begin");

	charsendinfo_t &SendInfo = player.m_CharSend;

	if (SendInfo.Status != CSS_RECEIVING)
		return;

	int Bytes = READ_BYTE();
	for (int i = 0; i < Bytes; i++)
		SendInfo.Data[SendInfo.DataSent++] = READ_BYTE();

	if ((signed)SendInfo.DataSent >= SendInfo.DataLen)
	{
		//Finished receiving file from server

		//Write to file
		CGameFile SaveFile;
		//Print("[NETCODE_SHARED] %i", SendInfo.Index); //MAR2010_08
		SaveFile.OpenWrite(GetSaveFileName(SendInfo.Index, &player));
		SaveFile.Write(SendInfo.Data, SendInfo.DataLen);
		SaveFile.Close();

		//Delete buffer
		delete SendInfo.Data;
		SendInfo.Data = NULL;
		SendInfo.Status = CSS_DORMANT;
	}

	enddbg;
}
#endif