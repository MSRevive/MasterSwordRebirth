//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "tri.h"
#include "inc_weapondefs.h"
#include "hud.h"
#include "cl_util.h"
#include "render/clenv.h"
#include "hudscript.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "Exports.h"

#include "particleman/particleman.h"
extern IParticleMan* g_pParticleMan;

//Half-life callback
#define DLLEXPORT EXPORT
extern "C"
{
	void DLLEXPORT HUD_DrawNormalTriangles(void);
	void DLLEXPORT HUD_DrawTransparentTriangles(void);
};

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles()
{
	//	RecClDrawNormalTriangles();

	CEnvMgr::RenderFog(true);

	//gHUD.m_Spectator.DrawOverview();
}


/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles()
{
	//	RecClDrawTransparentTriangles();

	CEnvMgr::PushHLStates();

	CEnvMgr::RenderFog(false);
	CEnvMgr::Think_DrawTransparentTriangles();
	gHUD.m_HUDScript->Effects_DrawTransPararentTriangles();

	CEnvMgr::PopHLStates();

	if (g_pParticleMan)
		g_pParticleMan->Update();
}
