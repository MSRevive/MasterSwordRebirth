//
// Music - Client-side playback of midi files
//

#include <windows.h>
#include <mmsystem.h>
#include "inc_huditem.h"
#include "sharedutil.h"
#include "hudmusic.h"

HMIDIOUT* midiOutChannel;
HMIDIOUT* midiOutChannelList[256];
UINT wDeviceID;

#define MIDI_MAX_VOLUME 0xFFFF //0x7FFF //0x0000 to 0xFFFF
#define MIDI_VOLUME (MIDI_MAX_VOLUME * max(min(CVAR_GET_FLOAT("ms_music_volume"), 1.0f), 0.0f))
#define PPQN_DIV (5.0 + 1.0 / 3.0)

MS_DECLARE_MESSAGE(m_Music, Music);

int CHudMusic::Init(void)
{
	m_CurrentSong = 0;
	m_SongsPlayed = m_FailedSongs = 0;
	m_TimePlayNextSong = 0;

	//memset( midiOutChannelList, 0, sizeof(midiOutChannelList) );

	CVAR_CREATE("ms_music", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
	CVAR_CREATE("ms_music_volume", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);

	HOOK_MESSAGE(Music);

	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem(this);

	return 1;
}

void CHudMusic::Think()
{
	if (!m_Songs.size())
		return;

	if (gHUD.m_iHideHUDDisplay & HIDEHUD_ALL || !(m_iFlags & HUD_ACTIVE))
		return;

	if (m_TimePlayNextSong && gHUD.m_flTime >= m_TimePlayNextSong)
	{
		PlayMusic();
	}

	if (!CVAR_GET_FLOAT("ms_music") && m_TimePlayNextSong)
		StopMusic();
}

void CHudMusic::InitHUDData(void)
{
	StopMusic();
	m_Songs.clear();
}

// Message handler for Music message
int CHudMusic::MsgFunc_Music(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iCmd = READ_BYTE();
	if (iCmd == 0)
	{
		songplaylist NewPlayList;

		int RecvSongNum = READ_BYTE();
		bool SameList = RecvSongNum == m_Songs.size() ? true : false;

		for (int i = 0; i < RecvSongNum; i++)
		{
			song_t Song;
			Song.Name = READ_STRING();
			Song.Length = READ_FLOAT();
			NewPlayList.add(Song);
			if (SameList && Song.Name != m_Songs[i].Name)
				SameList = false;
		}

		if (SameList)
			return 1; //The list of songs was the same. Do nothing (Currently this happens in Edana temple)

		m_Songs.clear();
		for (int i = 0; i < NewPlayList.size(); i++)
			m_Songs.add(NewPlayList[i]);

		m_SongsPlayed = m_FailedSongs = 0;
		PlayMusic();
	}
	else if (iCmd == 1)
	{
		m_Songs.clear();
		StopMusic();
	}

	return 1;
}

void CHudMusic::PlayMusic()
{
	if (!CVAR_GET_FLOAT("ms_music"))
	{
		m_TimePlayNextSong = gHUD.m_flTime + 10.0;
		return;
	}

	if (!m_Songs.size())
	{
		//There were no songs on the list or all songs failed playing
		m_TimePlayNextSong = 0;
		return;
	}

	StopMusic();
	m_TimePlayNextSong = gHUD.m_flTime + 5.0; //If play fails, try again in 5 seconds

	//Determine how many songs we've already played
	int SongsPlayed = 0;
	for (int i = 0; i < m_Songs.size(); i++)
		if (m_SongsPlayed & (1 << i))
			m_SongsPlayed++;

	//If we've played all the songs, reset the playlist
	if (m_SongsPlayed >= m_Songs.size())
		m_SongsPlayed = 0;

	//Pick a song to play
	//Choose a random song from the list, ignoring songs I've already played
	int ValidSongs = 0, ValidSong[MAX_SONGS];

	for (int n = 0; n < m_Songs.size(); n++)
	{
		if ((m_SongsPlayed & (1 << n)) || (m_FailedSongs & (1 << n)))
			continue;

		ValidSong[ValidSongs++] = n;
	}

	if (!ValidSongs)
		return;

	m_CurrentSong = ValidSong[rand() % ValidSongs];
	m_SongsPlayed |= (1 << m_CurrentSong); //Remove this song from the playlist

	msstring SongPath;
	if (!m_Songs[m_CurrentSong].Name.contains("/"))
		SongPath = /*msstring(EngineFunc::GetGameDir()) +*/ msstring(MUSIC_PATH) + m_Songs[m_CurrentSong].Name;
	else
		SongPath = m_Songs[m_CurrentSong].Name;

	if (SongPath.len() < 4)
	{
		Print("Music Error: Song name %s is too short \n", SongPath.c_str());
		m_FailedSongs |= (1 << m_CurrentSong);
		return;
	}

	msstring extension = SongPath.substr(SongPath.len() - 4, 4);
	if (extension == ".mp3")
	{
		m_CurrentSongMp3 = true;
		msstring Command = msstring("mp3 play ") + SongPath + "\n";

		//AUG2013_13 Thothie - less jarring music stops
		//I'd really like to rig it to fade out and wait a second before playing another song, regardless, but meh
		if (Command.contains("stop.mp3"))
		{
			StopMusic();
		}
		else
		{
			ClientCmd(Command);
		}
	}
	else if (extension == ".mid")
	{
		m_CurrentSongMp3 = false;

		DWORD dwReturn;
		MCI_OPEN_PARMS mciOpenParms;
		MCI_PLAY_PARMS mciPlayParms;
		MCI_STATUS_PARMS mciStatusParms;
		char ErrString[256];
		clrmem(mciOpenParms);
		clrmem(mciPlayParms);

		SongPath = msstring(EngineFunc::GetGameDir()) + "/" + SongPath;

		//if( !midiOutChannelList[0] )
		//	foreach( i, midiOutGetNumDevs() )
		//		midiOutChannelList[i] = new(HMIDIOUT);

		// Open the device by specifying the device and filename.
		// MCI will attempt to choose the MIDI mapper as the output port.

		// Loop through all MIDI output channels to find one that can play this midi
		bool fError = true;

		//for( uint i = 0; i < midiOutGetNumDevs(); i++ )
		{
			//Open the midi device
			//if( midiOutOpen(midiOutChannelList[i], i, 0, 0, NULL) )
			//	Print( "Music: Open Error (%i)\n", i );

			//midiOutChannel = midiOutChannelList[i];
			mciOpenParms.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_SEQUENCER;
			mciOpenParms.lpstrElementName = SongPath.c_str();

			if (dwReturn = mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID, (DWORD)(LPVOID)&mciOpenParms))
			{
				// Failed to open device, see if closing the current one helps -- UNDONE
				Print("Tried to play %s.....Failed\n", SongPath.c_str());
				//mciGetErrorString( dwReturn, ErrString, 256 );
				//Print( "[%i] Open failed: %s\n", i, ErrString );
				//continue;
			}
			else
			{
				//Print( "Used Device: %i\n", i );

				fError = false;
				//break;
			}
		}

		if (fError)
		{
			Print("Music Error: Cannot play back %s\n", SongPath.c_str());
			m_FailedSongs |= (1 << m_CurrentSong);
			return;
		}

		//Looping through the devices didn't help play wierd songs by Lanethan,
		//so just play on the first device
		/*midiOutChannel = midiOutChannelList[0];
		mciOpenParms.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_SEQUENCER;
		mciOpenParms.lpstrElementName = cFileName;
		if (dwReturn = mciSendCommand((MCIDEVICEID)(*midiOutChannel), MCI_OPEN,
			MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID|MCI_OPEN_ELEMENT|MCI_WAIT, (DWORD)(LPVOID)&mciOpenParms))
		{
			// Failed to open device, see if closing the current one helps -- UNDONE
			Print( "Tried to play %s.....\n", cSongList[CurrentSong] );
			mciGetErrorString( dwReturn, ErrString, 256 );
			Print( "Open failed: %s\n", ErrString );
			midiOutClose( *midiOutChannel );
			return;
		}*/

		// The device opened successfully; get the device ID.
		wDeviceID = mciOpenParms.wDeviceID;
		//Print( "Device: %i\n", wDeviceID );

		//Get the time the midi will last
		//-------------------------------

		//Get total length in either Beats Per Minute or Frames Per Second,
		//depending on the midi type
		mciStatusParms.dwItem = MCI_STATUS_LENGTH;
		mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&mciStatusParms);

		//Print( "Length: %i\n", mciStatusParms.dwReturn );
		DWORD dwLen = mciStatusParms.dwReturn;

		//Get tempo in either Beats Per Minute or Frames Per Second,
		//depending on the midi type
		mciStatusParms.dwItem = MCI_SEQ_STATUS_TEMPO;
		mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&mciStatusParms);

		//Print( "Tempo: %i\n", mciStatusParms.dwReturn );
		DWORD dwTempo = mciStatusParms.dwReturn;

		//Get the midi type
		mciStatusParms.dwItem = MCI_SEQ_STATUS_DIVTYPE;
		mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&mciStatusParms);

		//Print( "Divtype: %i\n", mciStatusParms.dwReturn );
		DWORD divType = mciStatusParms.dwReturn;

		DWORD SongLen = 60;
		switch (divType)
		{
		case MCI_SEQ_DIV_PPQN:
			//I've got Beats Per Minute
			//Important: There's 4 parts per beat
			SongLen = (dwLen / (dwTempo * 4.0)) * 60.0;
			break;
		case MCI_SEQ_DIV_SMPTE_24:
			//I've got Frames Per Second
			//SongLen = dwLen / (dwTempo * 240);
			break;
		}
		//Print( "Total Length: %i\n", SongLen );
		SongLen += 2;
		m_Songs[m_CurrentSong].Length = SongLen;

		// Check if the output port is the MIDI mapper.
		mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT;
		if (dwReturn = mciSendCommand(wDeviceID, MCI_STATUS,
			MCI_STATUS_ITEM, (DWORD)(LPVOID)&mciStatusParms))
		{
			mciSendCommand(wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
			midiOutClose(*midiOutChannel);
			Print("Music Error 2\n");
			m_FailedSongs |= (1 << m_CurrentSong);
			return;
		}

		//	Midi Mapper device verified.

		//Set the volume to a percentage of ms_music_volume
		midiOutSetVolume((HMIDIOUT)wDeviceID, MAKELONG(MIDI_VOLUME, MIDI_VOLUME));

		// Begin playback.
		memset(&mciPlayParms, 0, sizeof(MCI_PLAY_PARMS));
		if (dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, NULL,
			(DWORD)(LPVOID)&mciPlayParms))
		{
			mciGetErrorString(dwReturn, ErrString, 256);
			Print("Play Failed: %s\n", ErrString);

			mciSendCommand(wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
			midiOutClose(*midiOutChannel);
			m_FailedSongs |= (1 << m_CurrentSong);
			return;
		}
	}
	else
	{
		Print("Music Error: Unrecognized song extension for %s\n", SongPath.c_str());
		m_FailedSongs |= (1 << m_CurrentSong);
		return;
	}

	m_TimePlayNextSong = gHUD.m_flTime + m_Songs[m_CurrentSong].Length;
	Print("Playing %s.....\n", SongPath.c_str());
	m_PlayingSong = true;
}

void CHudMusic::StopMusic()
{
	if (!m_PlayingSong)
		return;

	if (m_CurrentSongMp3)
	{
		//ClientCmd( "Mp3FadeTime 2" );
		ClientCmd("cd fadeout\n"); //AUG2013_13 Thothie - Someone forgot his slash n!
	}
	else
		mciSendCommand(MCI_ALL_DEVICE_ID, MCI_CLOSE, MCI_WAIT, NULL);

	/*if( !midiOutChannel ) return;
	MCI_GENERIC_PARMS mgp;
	mciSendCommand( wDeviceID, MCI_STOP, MCI_WAIT,
		(DWORD)(LPVOID)&mgp );
	mciSendCommand( wDeviceID, MCI_CLOSE, MCI_WAIT, NULL );
	midiOutClose( *midiOutChannel );*/
	m_TimePlayNextSong = 0; //AUG2013_13 Thothie (comment only) - wondering if we can use this to spin down the current song before playing another
	//  mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
	//ConsolePrint( "Stop playing!\n" );
	m_PlayingSong = false;
}