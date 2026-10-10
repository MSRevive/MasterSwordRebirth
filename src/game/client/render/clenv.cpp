#include "inc_weapondefs.h"
#include "render/clrender.h"
#include "clglobal.h"

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "com_model.h"
#include "mslogger.h"
#include <SDL2/SDL_video.h>

#define MS_GL_ATTRIBUTES GL_ALL_ATTRIB_BITS

#ifdef _WIN32
static PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = NULL;
#else // _WIN32
#define glActiveTextureARB glActiveTexture
#endif

float CEnvMgr::m_LightGamma = 2.5;
float CEnvMgr::m_MaxViewDistance = 4096;
CEnvMgr::fog_t CEnvMgr::m_Fog = {false, Vector(0, 0, 0), 0.01, 0.01, 10000, GL_EXP};

int CEnvMgr::m_OldHLTexture[10] = {0};
bool CEnvMgr::m_OldMultiTextureEnabled = false;

extern float v_ViewDist;
void VGUIImages_NewLevel();

struct skyface_t
{
	CParticle Face;

	//Vector Offset;
};

#define SKYFILENAME_PREFIX "gfx/env/"
struct
{
	Vector Dir;
	const char* FileNameSuffix;
	bool Rotate;
} g_SkyBoxInfo[6] =
	{
		{Vector(0, 180, 0), "ft", false},
		{Vector(0, 90, 0), "lf", false},
		{Vector(0, 0, 0), "bk", false},
		{Vector(0, 270, 0), "rt", false},
		{Vector(-90, 0, 0), "dn", false},
		{Vector(90, 0, 0), "up", true},
};

class CSkyBox
{
public:
	msstring m_SkyName;
	mslist<skyface_t> Faces;
	int m_uiNextTexIdx;

	CSkyBox()
	{
		m_uiNextTexIdx = 1;
		m_SkyName = "g_morning";

		for (int i = 0; i < 6; i++)
		{
			Faces.add(skyface_t());

			CParticle &Face = Faces[i].Face;
			Face.m_Color = Color4F(1, 1, 1, 1);
			Face.m_Brightness = 1.0f;
		}
	}

	//Called once each level load
	void Setup()
	{
		for (int i = 0; i < Faces.size(); i++)
		{
			CParticle &Face = Faces[i].Face;
			Face.SetAngles(g_SkyBoxInfo[i].Dir);
			if (g_SkyBoxInfo[i].Rotate)
			{
				Vector2D Tmp = Face.m_TexCoords[0];
				Face.m_TexCoords[0] = Vector2D(0, 0);
				Face.m_TexCoords[1] = Vector2D(1, 0);
				Face.m_TexCoords[2] = Vector2D(1, 1);
				Face.m_TexCoords[3] = Vector2D(0, 1);
			}
		}

		ChangeTexture(m_SkyName);
	}

	//Dynamic.  Can change textures anytime
	void ChangeTexture(const char* NewTexture)
	{
		m_SkyName = NewTexture;
		for (int i = 0; i < Faces.size(); i++)
		{
			msstring FileName = /*msstring(EngineFunc::GetGameDir()) + "/" +*/ msstring(SKYFILENAME_PREFIX) + NewTexture + g_SkyBoxInfo[i].FileNameSuffix + ".tga";
			Faces[i].Face.m_GLTex = 0;
			CEnvMgr::LoadGLTexture(FileName, Faces[i].Face.m_GLTex);
		}
	}

	void Render()
	{
		//if( MSGlobals::GameScript )
		//	MSGlobals::GameScript->CallScriptEvent( "game_render_sky" );

		for (int i = 0; i < Faces.size(); i++)
		{
			CParticle &Face = Faces[i].Face;
			Face.m_Width = v_ViewDist - 1;

			Face.m_Origin = ViewMgr.Origin - Face.m_DirForward * Face.m_Width / 2.0f;
			Face.m_ContinuedParticle = false;
			Face.Render();
		}
	}
};

CSkyBox g_CustomSkyBox;
CParticle g_Tint;

//The renderamt the server sent for an entity, and what ApplyFogFade replaced it with
#define FOGFADE_MAX_ENTS 4096
struct fogfade_t
{
	int ModelIndex;
	int Original, Written;
};
static fogfade_t g_FogFade[FOGFADE_MAX_ENTS];

void CEnvMgr::Init()
{
	m_MaxViewDistance = EngineFunc::CVAR_GetFloat("sv_zmax");
	g_CustomSkyBox.Setup();
	g_Tint.m_Color = Color4F(0, 0, 0, 0);
}

void CEnvMgr::InitNewLevel()
{
	InitGL();
	memset(g_FogFade, 0, sizeof(g_FogFade));
	VGUIImages_NewLevel();
	MS_INFO("[InitNewLevel Complete]");
}

void CEnvMgr::ChangeSkyTexture(const char* NewTexture)
{
	g_CustomSkyBox.ChangeTexture(NewTexture);
	g_CustomSkyBox.Setup();	 //Thothie DEC2014_02 - trying to make setenv sky.texture work.
	g_CustomSkyBox.Render(); //Thothie DEC2014_02 - trying to make setenv sky.texture work.
}

void CEnvMgr::RenderSky()
{
	g_CustomSkyBox.Render();
}

void Surface_ResetLighting(msurface_t *pSurface)
{
	pSurface->cached_dlight = 1;
}

//Traverses all nodes and leafs, calling Func on each surface found
void TraverseAllNodes(mnode_t *pNode, void *Func)
{
	if (!pNode)
		return;

	mnode_t &Node = *pNode;

	if (Node.contents == CONTENTS_SOLID)
		return;

	if (Node.contents < 0)
	{
		//Call Function on Leaf
		mleaf_t &Leaf = *(mleaf_t *)&Node;
		for (int s = 0; s < Leaf.nummarksurfaces; s++)
			(*(ParseAllSurfacesFunc *)Func)(Leaf.firstmarksurface[s]);
		return;
	}

	for (int i = 0; i < 2; i++)
		TraverseAllNodes(Node.children[i], Func);
}

void CEnvMgr::SetLightGamma(float Value)
{
	m_LightGamma = Value;
	EngineFunc::CVAR_SetFloat("lightgamma", m_LightGamma);

	TraverseAllNodes(gEngfuncs.GetEntityByIndex(0)->model->nodes, Surface_ResetLighting);
}

void CEnvMgr::ChangeTint(const Color4F &Color)
{
	g_Tint.m_ContinuedParticle = false;
	g_Tint.m_DoubleSided = false;
	g_Tint.m_Color = Color;
	g_Tint.m_Brightness = 0.0f;
	g_Tint.m_Width = 500;
	g_Tint.m_RenderMode = kRenderTransAlpha;
}

//Draw transparent stuff
void CEnvMgr::Think_DrawTransparentTriangles()
{
	bool HideTint = MSCLGlobals::CharPanelActive ||	//Hide while choosing character
					!g_Tint.m_Color.a;				//Hide if alpha is zero

	if (!HideTint)
	{
		g_Tint.m_Origin = ViewMgr.Params->vieworg + ViewMgr.Params->forward * 4.2;
		g_Tint.BillBoard();
		g_Tint.Render();
	}
}

void CEnvMgr::Cleanup()
{
	//Unload all OGL Textures (skybox)
	DeleteGLTextures(); //Thothie JAN2011_03, restore texture cleanup
}

void CEnvMgr::RenderFog( bool bRender )
{
	if ( CEnvMgr::m_Fog.Enabled )
	{
		glEnable( GL_FOG );
		if ( bRender )
			glFogfv( GL_FOG_COLOR, CEnvMgr::m_Fog.Color );
		else
			glFogfv( GL_FOG_COLOR, Vector( 0, 0, 0 ) );
		glFogi( GL_FOG_MODE, CEnvMgr::m_Fog.Type ); //GL_EXP, GL_LINEAR
		glFogf( GL_FOG_DENSITY, CEnvMgr::m_Fog.Density );
		glFogf( GL_FOG_START, CEnvMgr::m_Fog.Start );
		glFogf( GL_FOG_END, CEnvMgr::m_Fog.End );
	}
	else
		glDisable( GL_FOG );

	// Required for transparent crap, else they refuse to apply fog. Thank you engine.
	gEngfuncs.pTriAPI->Fog( bRender ? CEnvMgr::m_Fog.Color : Vector( 0, 0, 0 ), CEnvMgr::m_Fog.Start, CEnvMgr::m_Fog.End, CEnvMgr::m_Fog.Enabled );
}

//How much of something at Origin is left after fog. Same math OpenGL uses for each fog type
float CEnvMgr::GetFogFactor(const Vector &Origin)
{
	if (!m_Fog.Enabled)
		return 1.0f;

	float Dist = (Origin - ViewMgr.Origin).Length();
	float Factor = 1.0f;

	if (m_Fog.Type == GL_EXP)
		Factor = exp(-m_Fog.Density * Dist);
	else if (m_Fog.Type == GL_EXP2)
		Factor = exp(-(m_Fog.Density * Dist) * (m_Fog.Density * Dist));
	else if (m_Fog.End != m_Fog.Start) //GL_LINEAR
		Factor = (m_Fog.End - Dist) / (m_Fog.End - m_Fog.Start);

	if (Factor < 0.0f)
		return 0.0f;
	if (Factor > 1.0f)
		return 1.0f;
	return Factor;
}

//The engine never fogs sprites or additive/glow entities, so fade them out by distance instead
bool CEnvMgr::ApplyFogFade(cl_entity_s *pEnt)
{
	cl_entity_t &Ent = *pEnt;

	if (!Ent.model || Ent.curstate.rendermode == kRenderNormal)
		return true;

	if (Ent.model->type != mod_sprite &&
		Ent.curstate.rendermode != kRenderTransAdd &&
		Ent.curstate.rendermode != kRenderGlow)
		return true;

	if (Ent.index < 0 || Ent.index >= FOGFADE_MAX_ENTS)
		return true;

	//curstate is only refreshed when a server update arrives, so scale from the saved value
	//or the fade would compound every frame
	fogfade_t &Fade = g_FogFade[Ent.index];
	if (Fade.ModelIndex != Ent.curstate.modelindex || Fade.Written != Ent.curstate.renderamt)
	{
		//The server changed it, or this is a different entity
		Fade.ModelIndex = Ent.curstate.modelindex;
		Fade.Original = Ent.curstate.renderamt;
	}

	Fade.Written = (int)(Fade.Original * GetFogFactor(Ent.origin));
	Ent.curstate.renderamt = Fade.Written;

	return Fade.Written > 0 || Fade.Original <= 0;
}

//MS OGL extention stuff
void CEnvMgr::InitGL()
{
#ifdef _WIN32
	glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glActiveTextureARB");
#endif
}

void CEnvMgr::PushHLStates()
{
	if (!glActiveTextureARB)
		return;

	glPushAttrib(MS_GL_ATTRIBUTES);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glGetIntegerv(0x8069, &m_OldHLTexture[0]); //GL_TEXTURE_2D_BINDING = 0x8069

	//Workaround -
	//Half-life sometimes uses Texture1 to draw normal textures and other times Texture2
	//It just depends on where you're standing.  Here I check whether HL is using Texture2,
	//disable it, use texture1, then set it back to Texture2 when done
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glGetIntegerv(0x8069, &m_OldHLTexture[1]); //GL_TEXTURE_2D_BINDING = 0x8069

	m_OldMultiTextureEnabled = glIsEnabled(GL_TEXTURE_2D) ? true : false;
	glDisable(GL_TEXTURE_2D);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);

	/*glEnable( GL_COLOR_MATERIAL );
 	glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
	glColorMaterial( GL_FRONT, GL_EMISSION );
	glColorMaterial( GL_FRONT, GL_SPECULAR );*/

	//---------------

	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);*/
}

void CEnvMgr::PopHLStates()
{
	if (!glActiveTextureARB)
		return;

	//Part of the Texture1/Texture2 Workaround
	//Set the active texture back to what HL was using before
	//----------------------------------------
	glPopAttrib();

	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D, m_OldHLTexture[1]);
	if (m_OldMultiTextureEnabled)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, m_OldHLTexture[0]);

	if (m_OldMultiTextureEnabled)
		glActiveTextureARB(GL_TEXTURE1_ARB);

	//static float color[4] = { 0, 0, 0, 0 };
	//int Param = GL_MODULATE;
	//glTexEnviv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &Param );
	//glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color );

	//----------------------------------------
}

struct gltexture_t : loadtex_t
{
	msstring Name;
};
static mslist<gltexture_t> g_TextureList;

bool CEnvMgr::LoadGLTexture(const char *FileName, loadtex_t &LoadTex)
{
	bool Loaded = false;

	for (int i = 0; i < g_TextureList.size(); i++)
	{
		if (g_TextureList[i].Name == FileName)
		{
			LoadTex = (loadtex_t)g_TextureList[i];
			return true;
		}
	}

	Loaded = Tartan::LoadTextureFile(FileName, LoadTex);

	if (Loaded)
	{
		gltexture_t Newtexture;
		loadtex_t &LT = Newtexture;
		LT = LoadTex;
		Newtexture.Name = FileName;
		g_TextureList.add(Newtexture);
	}
	else
	{
		Print("Missing MS Texture: %s\n", FileName);
	}

	return Loaded;
}

bool CEnvMgr::LoadGLTexture(const char *FileName, uint &TextureID)
{
	loadtex_t LoadTex;
	bool Success = LoadGLTexture(FileName, LoadTex);
	if (Success)
		TextureID = LoadTex.GLTexureID;

	return Success;
}

void CEnvMgr::DeleteGLTextures()
{
	for (int i = 0; i < g_TextureList.size(); i++)
		glDeleteTextures(1, &g_TextureList[i].GLTexureID);
	g_TextureList.clear();
}

//Called by the Tartan texture loader
void GetCompatibleTextureSize(uint SizeW, uint SizeH, uint &outNewSizeW, uint &outNewSizeH, float &outTexCoordU, float &outTexCoordV)
{
	//Force the texture to have dimensions 2^X
	float LargestDimension = SizeW > SizeH ? SizeW : SizeH;
	float Power = logf(LargestDimension) / logf(2);
	int IntPower = (int)Power;
	if (Power > IntPower)
		IntPower++; //Dimension is in-between standardized texure sizes.  Use the next highest size
	float TexSize = pow(2, (int)IntPower);

	//Cap texure size at GL_MAX_TEXTURE_SIZE
	int TexSizeMax = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &TexSizeMax);
	if (TexSize > TexSizeMax)
		TexSize = (float)TexSizeMax;

	outNewSizeW = outNewSizeH = TexSize;
	outTexCoordU = SizeW / (float)outNewSizeW;
	outTexCoordV = SizeH / (float)outNewSizeH;
}
