class CMSMonster;

class SPEECH
{
public:
	static const char *ItemName(CGenericItem *pItem, bool fCapital = false);
	static const char *NPCName(CMSMonster *pMonster, bool fCapital = false);
	static const char *HandName(int iHand, bool fCapital = false);
};

#define SPEECH_GetItemName SPEECH::ItemName
#define SPEECH_IntToHand SPEECH::HandName
//int SPEECH_HandToInt( char *Hand );
//void SPEECH_GetItemList ( CBasePlayerItem *FirstItem, char *ReturnString );
