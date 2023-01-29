#ifndef _HUDMISC
#define _HUDMISC
#include "hudbase.h"

//typedef struct cl_entity_s cl_entity_t;
#define MAX_ARROWS 5

class CHudMisc : public CHudBase
{
public:
	int Init(void);
	void InitHUDData(void); // called every time a server is connected to
	int Draw(float flTime);
	void Think(void);
	void Reset(void);
	void FreeMemory(void);
	void UserCmd_ChangeSayType(void);
	void UserCmd_RemovePack(void);
	void UserCmd_Offer(void);
	void UserCmd_Accept(void);
	//void UserCmd_ListSkills( void );
	void SelectMenuItem(int menu_item, TCallbackMenu *pcbMenu);

	entinfo_t m_OfferTarget;

private:
	// easiest place to set the crosshair lol
	HLSPRITE m_hCrosshairList[9];
	HLSPRITE m_hCrosshair;
	wrect_t m_rcCrosshair;

	HLSPRITE GetCrosshairSprite(int type);
#define MAX_GOLD_OFFER 1000000

	struct offerinfo_t
	{
		bool GoldScreen; //Whether I'm currently determining an amount of gold to offer
		int GoldAmt;
		int OfferItem[MAX_PLAYER_HANDS];
	} m_OfferInfo;
	mslist<ulong> m_RemoveList;
};
#endif //_HUDMISC
