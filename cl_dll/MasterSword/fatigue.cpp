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
// Fatigue.cpp - Control how your own fatigue is shown/heard
//
#include "..\inc_huditem.h"

//Player functionality
#include "inc_weapondefs.h"
#include "Stats/statdefs.h"
//----------------

#include "fatigue.h"

float LastGainStaminaTime = 0;

MS_DECLARE_MESSAGE(m_Fatigue, Fatigue);
//MS_DECLARE_COMMAND( m_Fatigue, ToggleFatigue );

int CHudFatigue::Init(void)
{
	m_DrawFatigue = FALSE;

	HOOK_MESSAGE(Fatigue);
	//HOOK_COMMAND("fatigue", ToggleFatigue);

	Reset();

	gHUD.AddHudElem(this);

	return 1;
}

int CHudFatigue::Draw(float flTime)
{
	if (m_DrawFatigue)
	{
		gHUD.DrawHudNumberSML(ScreenWidth - 40, ScreenHeight - 40, NULL, player.Stamina, 255, 255, 255);
		gHUD.DrawHudNumberSML(ScreenWidth - 20, ScreenHeight - 40, NULL, player.MaxStamina(), 255, 255, 255);
	}
	return 1;
}

int CHudFatigue::VidInit(void) { return 1; }

void CHudFatigue::Reset(void)
{
	//	m_iFlags &= ~HUD_ACTIVE;
	m_iFlags |= HUD_ACTIVE;
}
void CHudFatigue::InitHUDData(void)
{
	fBreatheTime = 0;
	LastGainStaminaTime = 0;
}

// Think about your fatigue
void CHudFatigue::DoThink()
{
	float MaxStamina = player.MaxStamina();

	if (player.Stamina > MaxStamina)
		player.Stamina = MaxStamina;

	else if (player.Stamina < player.MaxStamina() &&
			 !FBitSet(player.m_StatusFlags, PLAYER_MOVE_RUNNING) &&
			 !(player.IsActing()))
	{
		float flFrameIncrement, flExtraFatigue = 0;
		flFrameIncrement = gpGlobals->time - LastGainStaminaTime; // Standard stamina gain...

		//Stamina gain bonus +1 for every 10 str
		flExtraFatigue = 0.6;										   //3
		flExtraFatigue += (1 / 10.0) * player.GetNatStat(NATURAL_STR); // 30.0

		player.Stamina += flFrameIncrement * flExtraFatigue;
		player.Stamina = max(min(player.Stamina, MaxStamina), 0);
	}

	LastGainStaminaTime = gpGlobals->time;
	//Breathe( );
}

// Message handler for Fatigue message
// accepts five values:
//		byte   : 0 = Next byte specifies iFatigue | 1 = Next byte specifies iMaxFatigue
//		byte   : iFatigue = Current amount of Fatigue
int CHudFatigue::MsgFunc_Fatigue(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	if (!READ_BYTE())
		player.Stamina = READ_BYTE();

	//Lose a percentage of total stamina
	else if (READ_BYTE() == 1)
		player.Stamina *= (1 - (READ_BYTE() / 100.0));

	player.Stamina = max(min(player.Stamina, player.MaxStamina()), 0);
	return 1;
}
void CHudFatigue::UserCmd_ToggleFatigue(void)
{
	m_DrawFatigue = !m_DrawFatigue;
}
/*void CHudFatigue :: Breathe( )
{
	float flStaminaPercent = player.Stamina / player.MaxStamina();

	if( fBreatheTime < gHUD.m_flTime ) {
		if( flStaminaPercent < 0.25 ) {
			player.PlaySound( CHAN_VOICE, "player/breathe_fast3.wav", 1.0, true );
			fBreatheTime = gHUD.m_flTime + 3.54;
		}
		else if( flStaminaPercent < 0.35 ) {
			player.PlaySound( CHAN_VOICE, "player/breathe_fast2.wav", 1.0, true );
			fBreatheTime = gHUD.m_flTime + 2.7;
		}
		else if( flStaminaPercent < 0.65 ) {
			player.PlaySound( CHAN_VOICE, "player/breathe_fast1.wav", 1.0, true );
			fBreatheTime = gHUD.m_flTime + 7.1;
		}
		else if( Stamina < MaxStamina*0.95 ) {
			iRandom = rand() % 2;
			if( iRandom > 0 ) {
				player.PlaySound( CHAN_VOICE, "player/breathe_slow1a.wav", 1.0, true );
				fBreatheTime = gHUD.m_flTime + 9.75;
			}
			else {
				player.PlaySound( CHAN_VOICE, "player/breathe_slow1b.wav", 1.0, true );
				fBreatheTime = gHUD.m_flTime + 11;
			}
		
	}
}*/
