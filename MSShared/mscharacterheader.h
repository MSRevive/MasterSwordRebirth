#ifndef MSCHARACTERHEADER_H
#define MSCHARACTERHEADER_H

#include <pshpack4.h>
struct savedata_t
{
	//Player info
	int Version;
	char Name[32],
		Race[16],
		MapName[16], 
		NextMap[32], 
		OldTrans[32], 
		NewTrans[32],
		SteamID[32],
		Party[12];
	byte IsElite;
	int Gold;
	short MaxHP, MaxMP, HP, MP;
	Vector Origin, Angles;
	byte Gender;
	ulong PartyID; //Unique ID for party.  If I join a server that has a party with the same name, I won't
		//automatically join their party

	short PlayerKills;
	float TimeWaitedToForgetKill;  //Counts up... when reachs a certain number, decrement PlayerKills
	float TimeWaitedToForgetSteal; //Counts up... when reachs a certain number, player is not considered a thief
};
#include <poppack.h>

#endif //MSCHARACTERHEADER_H
