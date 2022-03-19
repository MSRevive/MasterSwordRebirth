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
//
// Health.cpp
//
// implementation of CHudHealth class
//

//Entity Stuff
#include "inc_weapondefs.h"
#include "pm_defs.h"
#include "event_api.h"
#include "clglobal.h"
#include "logfile.h"
#include "mscharacter.h"
#include "script.h"

//HUD stuff
#include "..\inc_huditem.h"
#include "../vgui_teamfortressviewport.h"

#include "health.h"
#include "hudmisc.h"
#include "menu.h"

MS_DECLARE_MESSAGE(m_Health, HP)
MS_DECLARE_MESSAGE(m_Health, MP)
MS_DECLARE_MESSAGE(m_Health, Damage)

#define MOVE_INC 3
#define PAIN_NAME "sprites/%d_pain.spr"
#define DAMAGE_NAME "sprites/%d_dmg.spr"

int giDmgHeight, giDmgWidth;

int giDmgFlags[NUM_DMG_TYPES] =
	{
		DMG_POISON,
		DMG_ACID,
		DMG_FREEZE | DMG_SLOWFREEZE,
		DMG_DROWN,
		DMG_BURN | DMG_SLOWBURN,
		DMG_NERVEGAS,
		DMG_RADIATION,
		DMG_SHOCK,
		DMG_CALTROP,
		DMG_TRANQ,
		DMG_CONCUSS,
		DMG_HALLUC};

int CHudHealth::Init(void)
{
	HOOK_MESSAGE(HP);
	HOOK_MESSAGE(MP);
	HOOK_MESSAGE(Damage);
	player.m_HP = 100.0f;
	m_fFade = 0;
	m_iFlags = HUD_ACTIVE;
	m_bitsDamage = 0;
	m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 0;
	giDmgHeight = 0;
	giDmgWidth = 0;
	//g_flHealth = player.m_HP;
	m_iTempHP = m_iTempMP = 0;

	memset(m_dmg, 0, sizeof(DAMAGE_IMAGE) * NUM_DMG_TYPES);

	/*	if( !CVAR_GET_FLOAT("gl_polyoffset") ) {
		char a[64];
		 _snprintf(a, sizeof(a),  "Software mode detected!  Disabling Health/Mana Flasks!.\n",  CVAR_GET_FLOAT("gl_polyoffset") ); 
		ConsolePrint( a );
		m_iFlags = 0;
	}*/

	gHUD.AddHudElem(this);
	return 1;
}

void CHudHealth::Reset(void)
{
	// make sure the pain compass is cleared when the player respawns
	m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 0;

	// force all the flashing damage icons to expire
	m_bitsDamage = 0;
	for (int i = 0; i < NUM_DMG_TYPES; i++)
	{
		m_dmg[i].fExpire = 0;
	}
	flChangeTime = 0;
}

int CHudHealth::VidInit(void)
{
	m_hSprite = 0;

	m_HUD_dmg_bio = gHUD.GetSpriteIndex("dmg_bio") + 1;
	m_HUD_cross = gHUD.GetSpriteIndex("cross");

	giDmgHeight = gHUD.GetSpriteRect(m_HUD_dmg_bio).right - gHUD.GetSpriteRect(m_HUD_dmg_bio).left;
	giDmgWidth = gHUD.GetSpriteRect(m_HUD_dmg_bio).bottom - gHUD.GetSpriteRect(m_HUD_dmg_bio).top;
	return 1;
}

int CHudHealth::MsgFunc_HP(const char *pszName, int iSize, void *pbuf)
{
	startdbg;
	dbg("Begin");

	BEGIN_READ(pbuf, iSize);
	int x = READ_SHORT(), iIsMaxHP = READ_BYTE();

	//thothie attempting to undo scrolling health effect
	if (player.m_HP < 0)
		player.m_HP = player.pev->health = 0;
	if (player.m_HP > 5000)
		player.m_HP = player.pev->health = 5000;

	if (!iIsMaxHP)
		player.m_HP = player.pev->health = x;
	else
		player.m_MaxHP = x;

	enddbg;
	return 1;
}
int CHudHealth::MsgFunc_MP(const char *pszName, int iSize, void *pbuf)
{
	startdbg;
	dbg("Begin");

	BEGIN_READ(pbuf, iSize);
	int x = READ_SHORT(), iIsMaxMP = READ_BYTE();

	//thothie attempting to undo scrolling mana effect
	if (player.m_MaxMP < 0)
		player.m_MP = 0;
	if (player.m_MaxMP > 5000)
		player.m_MP = 5000;

	if (!iIsMaxMP)
		player.m_MP = x;
	else
		player.m_MaxMP = x;

	enddbg;
	return 1;
}

int CHudHealth::MsgFunc_Damage(const char *pszName, int iSize, void *pbuf)
{
	startdbg;
	dbg("Begin");

	BEGIN_READ(pbuf, iSize);

	int armor = READ_BYTE();	   // armor
	int damageTaken = READ_BYTE(); // health
	long bitsDamage = READ_LONG(); // damage bits

	vec3_t vecFrom;

	for (int i = 0; i < 3; i++)
		vecFrom[i] = READ_COORD();

	UpdateTiles(gHUD.m_flTime, bitsDamage);

	// Actually took damage?
	if (damageTaken > 0 || armor > 0)
		CalcDamageDirection(vecFrom);

	enddbg;
	return 1;
}

// Returns back a color from the
// Green <-> Yellow <-> Red ramp
void CHudHealth::GetPainColor(int &r, int &g, int &b)
{
	int iHealth = m_iTempHP;

	if (iHealth > 25)
		iHealth -= 25;
	else if (iHealth < 0)
		iHealth = 0;
#if 0
	g = iHealth * 255 / 100;
	r = 255 - g;
	b = 0;
#else
	if (m_iTempHP > (player.m_MaxHP / 4))
	{
		//UnpackRGB(r,g,b, RGB_YELLOWISH);
		r = 255;
		g = 255;
		b = 255;
	}
	else
	{
		r = 250;
		g = 0;
		b = 0;
	}
#endif
}

int CHudHealth::Draw(float flTime)
{
	return 1;
	/*	int HPr, HPg, HPb;
	int x, y;
	int xFlask, yFlask;
	int iHealth = player.m_HP,
		iMaxHP = player.m_MaxHP,
		iMana = player.m_MP,
		iMaxMP = player.m_MaxMP;

	if( player.m_HP < 1.0f && player.m_HP > 0 ) iHealth = 1;	//Cap integers at 1
	if( player.m_MP < 1.0f && player.m_MP > 0 ) iMana = 1;

	if ( gHUD.m_iHideHUDDisplay & (HIDEHUD_HEALTH|HIDEHUD_ALL) || !(m_iFlags&HUD_ACTIVE) )
		return 1;

	if ( !m_hSprite )
		m_hSprite = LoadSprite(PAIN_NAME);
	

	// If health is getting low, make it bright red
	//if (m_iHealth <= 15) ScaleColors(r, g, b, a );
		
	if( (gHUD.m_flTime-flChangeTime) >= .007) {
		float inc = (100/MOVE_INC);
		if( m_iTempHP > iHealth ) { m_iTempHP -= (iHealth/inc)+1; if(m_iTempHP<iHealth) m_iTempHP = iHealth; }
		else if( m_iTempHP < iHealth ) { m_iTempHP += (iHealth/inc)+1; if(m_iTempHP>iHealth) m_iTempHP = iHealth; }
		if( m_iTempMP > iMana ) { m_iTempMP -= (iMana/inc)+1; if(m_iTempMP<iMana) m_iTempMP = iMana; }
		else if( m_iTempMP < iMana ) { m_iTempMP += (iMana/inc)+1; if(m_iTempMP>iMana) m_iTempMP = iMana; }
		flChangeTime = gHUD.m_flTime;
	}

//	if( m_iTempHP < iHealth ) m_iTempHP = iHealth;
//	if( m_iTempMP < iMana ) m_iTempMP = iMana;

//	ScaleColors(r, g, b, a );

	int iHealthFlask = gHUD.GetSpriteIndex("healthflask"),
		iManaFlask = gHUD.GetSpriteIndex("manaflask"),
		iframe;
	int SMLNumWidth = gHUD.GetSpriteRect(gHUD.m_HUD_numberSML_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_numberSML_0).left,
		SMLSlashWidth = gHUD.GetSpriteRect(gHUD.m_HUD_char_slashSML).right - gHUD.GetSpriteRect(gHUD.m_HUD_char_slashSML).left,
		HFlaskWidth = gHUD.GetSpriteRect(iHealthFlask).right - gHUD.GetSpriteRect(iHealthFlask).left,
		MFlaskWidth = gHUD.GetSpriteRect(iManaFlask).right - gHUD.GetSpriteRect(iManaFlask).left;
	int xStart, yStart;

	//y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;
	yFlask = ScreenHeight - 110; xFlask = XRES(15);

	xStart = (xFlask+(HFlaskWidth/2))-(((numofdigits(m_iTempHP)*SMLNumWidth)+(numofdigits(iMaxHP)*SMLNumWidth)+SMLSlashWidth)/2); yStart = ScreenHeight - 40;

	y = yStart; x = xStart;


	float fltemp; int maxframes;
	maxframes = (SPR_Frames(gHUD.GetSprite(iHealthFlask))-1);
	fltemp = (float)((float)m_iTempHP/(float)iMaxHP) * maxframes;
	iframe = min(fltemp,maxframes); //round up if not at integer
	SPR_Set(gHUD.GetSprite(iHealthFlask), 128, 128, 128 );
	SPR_DrawHoles( iframe, xFlask, yFlask, &gHUD.GetSpriteRect(iHealthFlask));

	GetPainColor( HPr, HPg, HPb );
	x = gHUD.DrawHudNumberSML(x, y, NULL, m_iTempHP, HPr, HPg, HPb);

	x = gHUD.DrawHudStringSML( x, y, "/", 255, 255, 255 );
	
	x = gHUD.DrawHudNumberSML(x, y, NULL, iMaxHP, 255, 255, 255);

	xFlask += (gHUD.GetSpriteRect(iHealthFlask).right - gHUD.GetSpriteRect(iHealthFlask).left) + XRES(10);
	
	maxframes = (SPR_Frames(gHUD.GetSprite(iManaFlask))-1);
	fltemp = (float)((float)m_iTempMP/(float)iMaxMP) * maxframes;
	iframe = min(fltemp,maxframes); //round up if not at integer
	SPR_Set(gHUD.GetSprite(iManaFlask), 128, 128, 128 );
	SPR_DrawHoles( iframe, xFlask, yFlask, &gHUD.GetSpriteRect(iManaFlask));

	x = (xFlask+(MFlaskWidth/2))-(((numofdigits(m_iTempMP)*SMLNumWidth)+(numofdigits(iMaxMP)*SMLNumWidth)+SMLSlashWidth)/2); yStart = ScreenHeight - 40;
	
	x = gHUD.DrawHudNumberSML(x, y, NULL, m_iTempMP, 255, 255, 255);

	x = gHUD.DrawHudStringSML( x, y, "/", 255, 255, 255 );
	
	x = gHUD.DrawHudNumberSML(x, y, NULL, iMaxMP, 255, 255, 255);

	DrawDamage(flTime);
	return DrawPain(flTime);*/
}

void CHudHealth::CalcDamageDirection(vec3_t vecFrom)
{
	vec3_t forward, right, up;
	float side, front;
	vec3_t vecOrigin, vecAngles;

	if (!vecFrom[0] && !vecFrom[1] && !vecFrom[2])
	{
		m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 0;
		return;
	}

	memcpy(vecOrigin, gHUD.m_vecOrigin, sizeof(vec3_t));
	memcpy(vecAngles, gHUD.m_vecAngles, sizeof(vec3_t));

	VectorSubtract(vecFrom, vecOrigin, vecFrom);

	float flDistToTarget = vecFrom.Length();

	vecFrom = vecFrom.Normalize();
	AngleVectors(vecAngles, forward, right, up);

	front = DotProduct(vecFrom, right);
	side = DotProduct(vecFrom, forward);

	if (flDistToTarget <= 50)
	{
		m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 1;
	}
	else
	{
		if (side > 0)
		{
			if (side > 0.3)
				m_fAttackFront = max(m_fAttackFront, side);
		}
		else
		{
			float f = fabs(side);
			if (f > 0.3)
				m_fAttackRear = max(m_fAttackRear, f);
		}

		if (front > 0)
		{
			if (front > 0.3)
				m_fAttackRight = max(m_fAttackRight, front);
		}
		else
		{
			float f = fabs(front);
			if (f > 0.3)
				m_fAttackLeft = max(m_fAttackLeft, f);
		}
	}
}

int CHudHealth::DrawPain(float flTime)
{
	if (!(m_fAttackFront || m_fAttackRear || m_fAttackLeft || m_fAttackRight))
		return 1;

	int r, g, b;
	int x, y, a, shade;

	// TODO:  get the shift value of the health
	a = 255; // max brightness until then

	float fFade = gHUD.m_flTimeDelta * 2;

	// SPR_Draw top
	if (m_fAttackFront > 0.4)
	{
		GetPainColor(r, g, b);
		shade = a * max(m_fAttackFront, 0.5);
		ScaleColors(r, g, b, shade);
		SPR_Set(m_hSprite, r, g, b);

		x = ScreenWidth / 2 - SPR_Width(m_hSprite, 0) / 2;
		y = ScreenHeight / 2 - SPR_Height(m_hSprite, 0) * 3;
		SPR_DrawAdditive(0, x, y, NULL);
		m_fAttackFront = max(0, m_fAttackFront - fFade);
	}
	else
		m_fAttackFront = 0;

	if (m_fAttackRight > 0.4)
	{
		GetPainColor(r, g, b);
		shade = a * max(m_fAttackRight, 0.5);
		ScaleColors(r, g, b, shade);
		SPR_Set(m_hSprite, r, g, b);

		x = ScreenWidth / 2 + SPR_Width(m_hSprite, 1) * 2;
		y = ScreenHeight / 2 - SPR_Height(m_hSprite, 1) / 2;
		SPR_DrawAdditive(1, x, y, NULL);
		m_fAttackRight = max(0, m_fAttackRight - fFade);
	}
	else
		m_fAttackRight = 0;

	if (m_fAttackRear > 0.4)
	{
		GetPainColor(r, g, b);
		shade = a * max(m_fAttackRear, 0.5);
		ScaleColors(r, g, b, shade);
		SPR_Set(m_hSprite, r, g, b);

		x = ScreenWidth / 2 - SPR_Width(m_hSprite, 2) / 2;
		y = ScreenHeight / 2 + SPR_Height(m_hSprite, 2) * 2;
		SPR_DrawAdditive(2, x, y, NULL);
		m_fAttackRear = max(0, m_fAttackRear - fFade);
	}
	else
		m_fAttackRear = 0;

	if (m_fAttackLeft > 0.4)
	{
		GetPainColor(r, g, b);
		shade = a * max(m_fAttackLeft, 0.5);
		ScaleColors(r, g, b, shade);
		SPR_Set(m_hSprite, r, g, b);

		x = ScreenWidth / 2 - SPR_Width(m_hSprite, 3) * 3;
		y = ScreenHeight / 2 - SPR_Height(m_hSprite, 3) / 2;
		SPR_DrawAdditive(3, x, y, NULL);

		m_fAttackLeft = max(0, m_fAttackLeft - fFade);
	}
	else
		m_fAttackLeft = 0;

	return 1;
}

/*int CHudHealth::DrawDamage(float flTime)
{
	int r, g, b, a;
	DAMAGE_IMAGE *pdmg;

	if (!m_bitsDamage)
		return 1;

	UnpackRGB(r,g,b, RGB_YELLOWISH);
	
	a = (int)( fabs(sin(flTime*2)) * 256.0);

	ScaleColors(r, g, b, a);

	// Draw all the items
	for (int i = 0; i < NUM_DMG_TYPES; i++)
	{
		if (m_bitsDamage & giDmgFlags[i])
		{
			pdmg = &m_dmg[i];
			SPR_Set(gHUD.GetSprite(m_HUD_dmg_bio + i), r, g, b );
			SPR_DrawAdditive(0, pdmg->x, pdmg->y, &gHUD.GetSpriteRect(m_HUD_dmg_bio + i));
		}
	}


	// check for bits that should be expired
	for ( i = 0; i < NUM_DMG_TYPES; i++ )
	{
		DAMAGE_IMAGE *pdmg = &m_dmg[i];

		if ( m_bitsDamage & giDmgFlags[i] )
		{
			pdmg->fExpire = min( flTime + DMG_IMAGE_LIFE, pdmg->fExpire );

			if ( pdmg->fExpire <= flTime		// when the time has expired
				&& a < 40 )						// and the flash is at the low point of the cycle
			{
				pdmg->fExpire = 0;

				int y = pdmg->y;
				pdmg->x = pdmg->y = 0;

				// move everyone above down
				for (int j = 0; j < NUM_DMG_TYPES; j++)
				{
					pdmg = &m_dmg[j];
					if ((pdmg->y) && (pdmg->y < y))
						pdmg->y += giDmgHeight;

				}

				m_bitsDamage &= ~giDmgFlags[i];  // clear the bits
			}
		}
	}

	return 1;
}*/

void CHudHealth::UpdateTiles(float flTime, long bitsDamage)
{
	DAMAGE_IMAGE *pdmg;

	// Which types are new?
	long bitsOn = ~m_bitsDamage & bitsDamage;

	for (int i = 0; i < NUM_DMG_TYPES; i++)
	{
		pdmg = &m_dmg[i];

		// Is this one already on?
		if (m_bitsDamage & giDmgFlags[i])
		{
			pdmg->fExpire = flTime + DMG_IMAGE_LIFE; // extend the duration
			if (!pdmg->fBaseline)
				pdmg->fBaseline = flTime;
		}

		// Are we just turning it on?
		if (bitsOn & giDmgFlags[i])
		{
			// put this one at the bottom
			pdmg->x = giDmgWidth / 8;
			pdmg->y = ScreenHeight - giDmgHeight * 2;
			pdmg->fExpire = flTime + DMG_IMAGE_LIFE;

			// move everyone else up
			for (int j = 0; j < NUM_DMG_TYPES; j++)
			{
				if (j == i)
					continue;

				pdmg = &m_dmg[j];
				if (pdmg->y)
					pdmg->y -= giDmgHeight;
			}
			pdmg = &m_dmg[i];
		}
	}

	// damage bits are only turned on here;  they are turned off when the draw time has expired (in DrawDamage())
	m_bitsDamage |= bitsDamage;
}
