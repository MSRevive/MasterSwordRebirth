#include "inc_weapondefs.h"
#include "render/clrender.h"
#include "hud.h"
#include "cl_util.h"
#include "vgui_teamfortressviewport.h"
#include "vgui_mscontrols.h"
#include "mslogger.h"

mslist<VGUI_Image3D *> g_VGUIImages;

VGUI_Image3D::VGUI_Image3D(const char *pszImageName, bool TGAorSprite, bool Delayed, int x, int y, int wide, int tall) 
: CImageDelayed(pszImageName, TGAorSprite, Delayed, x, y, wide, tall)
{
	init();
}
void VGUI_Image3D::init()
{
	m_Particle = new CParticle();
	m_Particle->m_ContinuedParticle = false;
	m_Particle->m_DoubleSided = true;
	m_Particle->m_Color = Color4F(1, 1, 1, 1);
	m_Particle->m_Brightness = 1.0f;
	m_Particle->m_Square = false;
	m_Particle->m_RenderMode = kRenderTransAlpha;
	g_VGUIImages.add(this);
}

void VGUI_Image3D::LoadImg()
{
	if (m_ImageLoaded)
		return;

	if (!m_Particle)
		return;

	if (m_TGAorSprite)
	{
		msstring FileName = msstring("gfx/vgui/") + m_ImageName + ".tga";

		loadtex_t LoadTex;
		m_ImageLoaded = CEnvMgr::LoadGLTexture(FileName, LoadTex);
		if (m_ImageLoaded)
		{
			m_Particle->m_GLTex = LoadTex.GLTexureID;
			m_Particle->m_TexCoords[0] = Vector2D(LoadTex.CoordU, 0);
			m_Particle->m_TexCoords[1] = Vector2D(LoadTex.CoordU, LoadTex.CoordV);
			m_Particle->m_TexCoords[2] = Vector2D(0, LoadTex.CoordV);
			m_Particle->m_TexCoords[3] = Vector2D(0, 0);
		}
	}
	else
	{
		CImageDelayed::LoadImg();
		m_Particle->m_Texture = (model_s *)gEngfuncs.GetSpritePointer(m_SpriteHandle);
		m_ImageLoaded = m_Particle->m_Texture ? true : false;
	}
}

void VGUI_Image3D::paintBackground()
{
	if (!m_ImageLoaded)
		return;

	if (!m_Particle)
		return;

	CEnvMgr::PushHLStates();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	//m_Particle->BillBoard();
	m_Particle->m_Width = getWide();
	m_Particle->m_Height = getTall();
	m_Particle->m_SpriteFrame = m_Frame;
	m_Particle->m_DirForward = Vector(0, 0, 1);
	float VerticalMultiplier = m_TGAorSprite ? -1 : 1;
	m_Particle->m_DirRight = Vector(-1, 0, 0);
	m_Particle->m_DirUp = Vector(0, 1 * VerticalMultiplier, 0); //m_DirUp is actually pointing down here
	m_Particle->m_Origin = (-m_Particle->m_DirRight * m_Particle->m_Width / 2.0f) + (m_Particle->m_DirUp * VerticalMultiplier * m_Particle->m_Height / 2.0f);
	m_Particle->Render();

	CEnvMgr::PopHLStates();
}

VGUI_Image3D::~VGUI_Image3D()
{
	for (int i = 0; i < g_VGUIImages.size(); i++)
	{
		if (g_VGUIImages[i] == this)
		{
			g_VGUIImages.erase(i);
			break;
		}
	}
}

void VGUIImages_NewLevel()
{
	//Reload the TGA textures for the 3D VGUI Images
	for (int i = 0; i < g_VGUIImages.size(); i++)
	{
		if (g_VGUIImages[i]->m_TGAorSprite)
		{
			g_VGUIImages[i]->m_ImageLoaded = false;
			g_VGUIImages[i]->LoadImg();
		}
	}
	MS_INFO("[VGUIImages_NewLevel Complete]");
}
