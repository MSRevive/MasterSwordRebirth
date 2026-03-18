/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

enum {

	DMG_IMAGE_POISON = 0,
	DMG_IMAGE_ACID = 1,
	DMG_IMAGE_COLD = 2,
	DMG_IMAGE_LIFE = 2,// seconds that image is up
	DMG_IMAGE_DROWN = 3,
	DMG_IMAGE_BURN = 4,
	DMG_IMAGE_NERVE = 5,
	DMG_IMAGE_RAD = 6,
	DMG_IMAGE_SHOCK = 7,
	//tf defines
	DMG_IMAGE_CALTROP = 8,
	DMG_IMAGE_TRANQ = 9,
	DMG_IMAGE_CONCUSS = 10,
	DMG_IMAGE_HALLUC = 11
};

constexpr int NUM_DMG_TYPES = 12;
// instant damage

typedef struct
{
	float fExpire;
	float fBaseline;
	int x, y;
} DAMAGE_IMAGE;

//
//-----------------------------------------------------
//
class CHudHealth : public CHudBase
{
public:
	virtual int Init(void);
	virtual int VidInit(void);
	virtual int Draw(float fTime);
	virtual void Reset(void);
	int MsgFunc_CLDllFunc(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_HP(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_MP(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_Damage(const char *pszName, int iSize, void *pbuf);
	int m_iTempHP, m_iTempMP; //HP/MP Displayed at the moment.. might not be real HP/MP
	int m_HUD_dmg_bio;
	int m_HUD_cross;
	float flChangeTime;

private:
	HLSPRITE m_hSprite;
	HLSPRITE m_hDamage;

	DAMAGE_IMAGE m_dmg[NUM_DMG_TYPES];
	int m_bitsDamage;
	float m_fFade;
	void GetPainColor(int &r, int &g, int &b);
	int DrawPain(float fTime);
	int DrawDamage(float fTime);
	float m_fAttackFront, m_fAttackRear, m_fAttackLeft, m_fAttackRight;
	void CalcDamageDirection(Vector  vecFrom);
	void UpdateTiles(float fTime, long bits);
};
