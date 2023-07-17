//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SERVER_ZPS_ANGELSCRIPT_SCRIPTAPI_CASUTIL_H
#define GAME_SERVER_ZPS_ANGELSCRIPT_SCRIPTAPI_CASUTIL_H

#include "CASColor.h"

class asIScriptEngine;
class CScriptArray;

class HudTextParams final
{
public:
	HudTextParams() = default;
	~HudTextParams() = default;

	float		x;
	float		y;
	int			effect;
	int			r1, g1, b1, a1;
	int			r2, g2, b2, a2;
	float		fadeinTime;
	float		fadeoutTime;
	float		holdTime;
	float		fxTime;
	int			channel;

	void		SetColor( Color& color );
	void		SetColor2( Color& color );
	
private:
	// No copy constructors allowed
	HudTextParams(const HudTextParams& vOther);
};

class CASUtil final
{
public:
	CASUtil() = default;
	~CASUtil() = default;

	void ColorToHex(Color& clr, int& iHex);

private:
	CASUtil(const CASUtil&) = delete;
	CASUtil& operator=(const CASUtil&) = delete;
};

void RegisterScriptUtils( asIScriptEngine &engine );

#endif // GAME_SERVER_ZPS_ANGELSCRIPT_SCRIPTAPI_CASUTIL_H
