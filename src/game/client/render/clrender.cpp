// Triangle rendering, if any

#include "inc_weapondefs.h"
#include "clrender.h"
#include "clglobal.h"

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "hudscript.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "studio_util.h"
#include "r_studioint.h"
#include "ref_params.h"
#include "mslogger.h"

CParticle::CParticle()
{
	m_Width = 0;
	m_Origin = g_vecZero;
	m_Texture = NULL;
	m_Color = Color4F(1.0f, 1.0f, 1.0f, 1.0f);
	m_Brightness = 1.0f;
	m_DirForward = g_vecZero;
	m_DirRight = g_vecZero;
	m_DirUp = g_vecZero;
	m_DoubleSided = true;
	m_ContinuedParticle = false;
	m_Square = true;
	m_GLTex = 0;
	m_RenderMode = kRenderNormal;
	m_SpriteFrame = 0;

	m_TexCoords[0] = Vector2D(1, 0);
	m_TexCoords[1] = Vector2D(1, 1);
	m_TexCoords[2] = Vector2D(0, 1);
	m_TexCoords[3] = Vector2D(0, 0);
}

void CParticle::SetAngles(Vector Angles)
{
	EngineFunc::MakeVectors(Angles, &m_DirForward, &m_DirRight, &m_DirUp);
}

void CParticle::BillBoard()
{
	//Make this face the player
	m_DirForward = ViewMgr.Params->forward;
	m_DirRight = ViewMgr.Params->right;
	m_DirUp = ViewMgr.Params->up;
}

bool CParticle::LoadTexture(const char* Name)
{
	HLSPRITE Sprite = MSBitmap::GetSprite(Name);
	if (!Sprite)
		return false;

	m_Texture = (model_s *)gEngfuncs.GetSpritePointer(Sprite);
	if (!m_Texture)
		return false;

	return true;
}

void CParticle::Render()
{
	//return;
	Vector &vForward = m_DirForward, &vRight = m_DirRight, &vUp = m_DirUp;

	Vector Points[4];

	float HalfWidth = m_Width / 2.0f;
	float HalfHeight = m_Square ? HalfWidth : m_Height / 2.0f;
	Vector SideRay = vRight * HalfWidth;
	Vector VerticalRay = vUp * HalfHeight;

	Points[0] = m_Origin - SideRay - VerticalRay;
	Points[1] = m_Origin + SideRay - VerticalRay;
	Points[2] = m_Origin + SideRay + VerticalRay;
	Points[3] = m_Origin - SideRay + VerticalRay;

	// Create me a triangle

	//gEngfuncs.pTriAPI->RenderMode( kRenderNormal );

	if (m_GLTex)
		glBindTexture(GL_TEXTURE_2D, m_GLTex);
	else if (m_Texture)
		gEngfuncs.pTriAPI->SpriteTexture(m_Texture, m_SpriteFrame);
	else
		glBindTexture(GL_TEXTURE_2D, 0);

	gEngfuncs.pTriAPI->RenderMode(m_RenderMode);
	gEngfuncs.pTriAPI->CullFace(m_DoubleSided ? TRI_NONE : TRI_FRONT);
	gEngfuncs.pTriAPI->Brightness(m_Brightness);

	gEngfuncs.pTriAPI->Color4f(m_Color.r, m_Color.g, m_Color.b, m_Color.a);

	//glDisable( GL_DEPTH_TEST );
	//glEnable( GL_BLEND );
	/*glBegin( GL_QUADS );
	glColor4fv( (float *)&m_Color.r );
	
	glTexCoord2f( m_TexCoords[0].x, m_TexCoords[0].y ); glVertex3fv( Points[0] );
	glTexCoord2f( m_TexCoords[1].x, m_TexCoords[1].y ); glVertex3fv( Points[3] );
	glTexCoord2f( m_TexCoords[2].x, m_TexCoords[2].y ); glVertex3fv( Points[2] );
	glTexCoord2f( m_TexCoords[3].x, m_TexCoords[3].y ); glVertex3fv( Points[1] );
	glEnd( );*/

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	if (!m_ContinuedParticle)
		gEngfuncs.pTriAPI->Begin(TRI_QUADS);

	gEngfuncs.pTriAPI->TexCoord2f(m_TexCoords[0].x, m_TexCoords[0].y);
	gEngfuncs.pTriAPI->Vertex3fv(Points[0]);

	gEngfuncs.pTriAPI->TexCoord2f(m_TexCoords[1].x, m_TexCoords[1].y);
	gEngfuncs.pTriAPI->Vertex3fv(Points[3]);

	gEngfuncs.pTriAPI->TexCoord2f(m_TexCoords[2].x, m_TexCoords[2].y);
	gEngfuncs.pTriAPI->Vertex3fv(Points[2]);

	gEngfuncs.pTriAPI->TexCoord2f(m_TexCoords[3].x, m_TexCoords[3].y);
	gEngfuncs.pTriAPI->Vertex3fv(Points[1]);

	if (!m_ContinuedParticle)
		gEngfuncs.pTriAPI->End();

	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
}
