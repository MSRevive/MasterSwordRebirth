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
// cl_util.h
//

#include "sharedutil.h"
#include "msdebug.h"

#include "cvardef.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// ScreenHeight returns the height of the screen, in pixels
#define ScreenHeight (gHUD.m_scrinfo.iHeight)
// ScreenWidth returns the width of the screen, in pixels
#define ScreenWidth (gHUD.m_scrinfo.iWidth)

//these must stay macros, or we need to make a msg function for every command.
// Macros to hook function calls into the HUD object
#define HOOK_MESSAGE(x) gEngfuncs.pfnHookUserMsg(#x, __MsgFunc_##x);

#define DECLARE_MESSAGE(y, x)                                     \
	int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf) \
	{                                                             \
		int ret = 0;                                              \
		ret = gHUD.y.MsgFunc_##x(pszName, iSize, pbuf);           \
		return ret;                                               \
	}

#define HOOK_COMMAND(x, y) gEngfuncs.pfnAddCommand(x, __CmdFunc_##y);
#define DECLARE_COMMAND(y, x)   \
	void __CmdFunc_##x(void)    \
	{                           \
		gHUD.y.UserCmd_##x();   \
	}

//------------ Master Sword ----------------
#define MS_DECLARE_MESSAGE(y, x)                                  \
	int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf) \
	{                                                             \
		int ret = 0;                                              \
		ret = gHUD.y->MsgFunc_##x(pszName, iSize, pbuf);          \
		return ret;                                               \
	}
#define MS_DECLARE_COMMAND(y, x) \
	void __CmdFunc_##x(void)     \
	{                            \
		gHUD.y->UserCmd_##x();   \
	}
//------------------------------------------
//end required macros
//Master Sword
#undef CVAR_GET_FLOAT
#undef CVAR_GET_STRING
#undef CVAR_CREATE
inline float CVAR_GET_FLOAT(const char *x) { return gEngfuncs.pfnGetCvarFloat(x); }
inline const char *CVAR_GET_STRING(const char *x) { return gEngfuncs.pfnGetCvarString(x); }
inline struct cvar_s *CVAR_CREATE(const char *cv, const char *val, const int flags) { return gEngfuncs.pfnRegisterVariable(cv, val, flags); }

//#define SPR_Load (gEngfuncs.pfnSPR_Load(
//#define SPR_Set (*gEngfuncs.pfnSPR_Set)
//#define SPR_Frames (*gEngfuncs.pfnSPR_Frames)
//#define SPR_GetList (*gEngfuncs.pfnSPR_GetList)

// SPR_Draw  draws a the current sprite as solid
//#define SPR_Draw (*gEngfuncs.pfnSPR_Draw)

//#define SPR_DrawHoles (*gEngfuncs.pfnSPR_DrawHoles)
// SPR_DrawAdditive  adds the sprites RGB values to the background  (additive transulency)
//#define SPR_DrawAdditive (*gEngfuncs.pfnSPR_DrawAdditive)

// SPR_EnableScissor  sets a clipping rect for HUD sprites.  (0,0) is the top-left hand corner of the screen.
//#define SPR_EnableScissor (*gEngfuncs.pfnSPR_EnableScissor)

// SPR_DisableScissor  disables the clipping rect
//#define SPR_DisableScissor (*gEngfuncs.pfnSPR_DisableScissor)
//#define FillRGBA (*gEngfuncs.pfnFillRGBA)

// Use this to set any co-ords in 640x480 space
//#define XRES(x) ((int)(float(x) * ((float)ScreenWidth / 640.0f) + 0.5f))
//#define YRES(y) ((int)(float(y) * ((float)ScreenHeight / 480.0f) + 0.5f))

// use this to project world coordinates to screen coordinates
//#define XPROJECT(x) ((1.0f + (x)) * ScreenWidth * 0.5f)
//#define YPROJECT(y) ((1.0f - (y)) * ScreenHeight * 0.5f)

//#define GetScreenInfo (*gEngfuncs.pfnGetScreenInfo)
//#define ServerCmd (*gEngfuncs.pfnServerCmd)
//#define ClientCmd (*gEngfuncs.pfnClientCmd)
//#define SetCrosshair (*gEngfuncs.pfnSetCrosshair)
//#define AngleVectors (*gEngfuncs.pfnAngleVectors)


inline HLSPRITE SPR_Load(const char* sprName) {
	return gEngfuncs.pfnSPR_Load(sprName);
};

inline void SPR_Set(HLSPRITE hpic, const int r, const int g, const int b) {
	gEngfuncs.pfnSPR_Set(hpic, r, g, b);
};

inline int SPR_Frames(HLSPRITE hpic) {
	return gEngfuncs.pfnSPR_Frames(hpic);
};

inline client_sprite_s* SPR_GetList(const char* psz, int* piCount) {
	return gEngfuncs.pfnSPR_GetList(psz, piCount);
};

//SPR_Draw draws the current sprite as solid
inline void SPR_Draw(const int frame, const int x, const int y, const Rect* pRect) {
	gEngfuncs.pfnSPR_Draw(frame, x, y, pRect);
};

// SPR_DrawHoles  draws the current sprites,  with color index255 not drawn (transparent)
inline void SPR_DrawHoles(const int frame, const int x, const int y, const Rect* pRect) {
	gEngfuncs.pfnSPR_DrawHoles(frame, x, y, pRect);
}

inline void SPR_DrawAdditive(const int frame, const int x, const int y, const Rect* pRect) {
	gEngfuncs.pfnSPR_DrawAdditive(frame, x, y, pRect);
}

inline void SPR_EnableScissor(const int x, const int y, const int width, const int height) {
	gEngfuncs.pfnSPR_EnableScissor(x, y, width, height);
}

inline void SPR_DisableScissor() {
	gEngfuncs.pfnSPR_DisableScissor();
}

inline void FillRGBA(const int x, const int y, const int width, const int height, const int r, const int b, const int g, const int a) {
	gEngfuncs.pfnFillRGBA(x, y, width, height, r, g, b, a);
}


inline int XRES(const int x) {
	return (int)(float(x) * ((float)ScreenWidth / 640.0f) + 0.5f);
}

inline int YRES(const int y) {
	return (int)(float(y) * ((float)ScreenHeight / 480.0f) + 0.5f);
}

inline float XPROJECT(const float x) {
	return (float)(1.0f + x) * ScreenWidth * 0.5f;
}

inline float YPROJECT(const float y) {
	return (float)(1.0f - y) * ScreenHeight * 0.5f;
}

inline int GetScreenInfo(SCREENINFO_s * psScreenInfo) {
	return gEngfuncs.pfnGetScreenInfo(psScreenInfo);
}

inline int ServerCmd(const char* pszCmdString) {
	return gEngfuncs.pfnServerCmd(pszCmdString);
}

inline int ClientCmd(const char* pszCmdString) {
	return gEngfuncs.pfnClientCmd(pszCmdString);
}

inline void SetCrosshair(const HLSPRITE hSprite, const Rect rt, const int r, const int g, const int b) {
	gEngfuncs.pfnSetCrosshair(hSprite, rt, r, g, b);
}

// Gets the height & width of a sprite,  at the specified frame
inline int SPR_Height(HLSPRITE x, int f) { return gEngfuncs.pfnSPR_Height(x, f); }
inline int SPR_Width(HLSPRITE x, int f) { return gEngfuncs.pfnSPR_Width(x, f); }

inline client_textmessage_t *TextMessageGet(const char *pName) { return gEngfuncs.pfnTextMessageGet(pName); }
inline int TextMessageDrawChar(int x, int y, int number, int r, int g, int b)
{
	return gEngfuncs.pfnDrawCharacter(x, y, number, r, g, b);
}

inline int DrawConsoleString(int x, int y, const char *string)
{
	return gEngfuncs.pfnDrawConsoleString(x, y, (char *)string);
}

inline void GetConsoleStringSize(const char *string, int *width, int *height)
{
	gEngfuncs.pfnDrawConsoleStringLen(string, width, height);
}

inline int ConsoleStringLen(const char *string)
{
	int _width, _height;
	GetConsoleStringSize(string, &_width, &_height);
	return _width;
}

inline void ConsolePrint(const char *string)
{
	if (gEngfuncs.pfnConsolePrint)
		gEngfuncs.pfnConsolePrint(string);
}

inline void CenterPrint(const char *string)
{
	gEngfuncs.pfnCenterPrint(string);
}

inline void GetPlayerInfo(const int enum_num, hud_player_info_s* pInfo ) {
	gEngfuncs.pfnGetPlayerInfo(enum_num, pInfo);
}

// returnsthe players name of entity no.
//#define GetPlayerInfo (*gEngfuncs.pfnGetPlayerInfo)

// sound functions
inline void PlaySound(const char *szSound, float vol) { gEngfuncs.pfnPlaySoundByName(szSound, vol); }
inline void PlaySound(int iSound, float vol) { gEngfuncs.pfnPlaySoundByIndex(iSound, vol); }
inline void PlayHUDSound(const char *Sound, float vol) { PlaySound((char *)Sound, vol); }

void ScaleColors(int &r, int &g, int &b, int a);

template <typename Type>
float DotProduct(Type* x, Type* y) {
	return (x[0] * y[0]) + (x[1] * y[1]) + (x[2] + y[2]);
};

//#define DotProduct(x, y) ((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2])

// ugh, some bits of the client code expect a float[3] while others expect a Vector, and there's a macro that pretends they're the same
// handle the extern with the correct types in the places where it's required
// extern Vector  vec3_origin;

// disable 'possible loss of data converting float to int' warning message
//#pragma warning(disable : 4244)
// disable 'truncation from 'const double' to 'float' warning message
//#pragma warning(disable : 4305)

inline void UnpackRGB(int &r, int &g, int &b, unsigned long ulRGB)
{
	r = (ulRGB & 0xFF0000) >> 16;
	g = (ulRGB & 0xFF00) >> 8;
	b = ulRGB & 0xFF;
}

HLSPRITE LoadSprite(const char *pszName);
