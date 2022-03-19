//Master Sword-specific structs for transations between client and server

//Tells client to start recieving a new script file
#include <pshpack4.h>

struct scriptinfo_t
{
	byte MsgID;
	char Name[32];
	byte ItemType;
	int FileSize;
	byte Percent;
};
#define SCRIPT_SEND_AMT 1024 //255 //500 at max
struct scriptdata_t
{
	byte MsgID;
	short MsgSize;
	int DataOfs;
	byte Data[SCRIPT_SEND_AMT];
};
#include <poppack.h>
