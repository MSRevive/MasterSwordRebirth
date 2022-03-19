/*
	Manages netcode for sending MS characters
*/

#include "../inc_huditem.h"
#include "inc_weapondefs.h"
#include "msnetcodeclient.h"
#include "vgui_menudefsshared.h"

void CNetCode::InitNetCode()
{
	pNetCode = msnew CNetCodeClient();
	pNetCode->Init();
}

void UpdateStatusVGUI();

void SaveCharSend(bool fResetSend)
{
	if (fResetSend)
	{
		//Read the character file to memory
		CPlayer_DataBuffer gFile;
		//gFile.Open( );
		if (!gFile.ReadFromFile(GetSaveFileName(player.m_CharacterNum), "rb", true))
		{
			ClientPrint(NULL, HUD_PRINTNOTIFY, "\nYour character could not be loaded!\n\n");
			ShowVGUIMenu(MENU_NEWCHARACTER);
			return;
		}

		CNetFileTransaction &SendFile = *msnew CNetFileTransaction(g_NetCode.m.HostIP, gFile.m_Buffer, gFile.m_BufferSize);
		g_NetCode.m.Transactons.push_back(&SendFile);
		ClientCmd(UTIL_VarArgs("savefileid %i\n", SendFile.m.FileID)); //Send File ID to server

		ShowVGUIMenu(MENU_SPAWN);
		gFile.Close();
	}

	UpdateStatusVGUI();
	g_NetCode.Think();
}
