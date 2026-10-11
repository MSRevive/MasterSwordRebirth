
//curstate.oldbuttons
#define MSRDR_VIEWMODEL (1 << 0) //Model is a view model (parent view model... never rendered fully)
#define MSRDR_FLIPPED (1 << 1)	 //Model is flipped across the verical axis, changing it from the right to left hand
#define MSRDR_SKIP (1 << 2)		 //Do all setup (for attachments later), but don't render
#define MSRDR_FULLROT (1 << 4)	 //Use full rotation this frame (automatically unset in )

//Separate flags from the above -- stored in curstate.colormap
#define MSRDR_GLOW_GRN (1 << 0) //Green glow
#define MSRDR_GLOW_RED (1 << 1) //Highlight slected weapons - Red Glow
#define MSRDR_DARK (1 << 2)		//Dim lights on the inset player model
#define MSRDR_LIGHT_DIM (1 << 3)
#define MSRDR_LIGHT_NORMAL (1 << 4) //Give a moderate amount of artificial light
#define MSRDR_LIGHT_BRIGHT (1 << 5) //Make it fullbright
#define MSRDR_ANIM_ONCE (1 << 6)	//Only play the client-side anim once (no loop)
#define MSRDR_ASPLAYER (1 << 7)		//Render as player
#define MSRDR_COPYPLAYER (1 << 8)	//Copy the local player's anims
#define MSRDR_HANDMODEL (1 << 9)	//This is one of the two hand models (coming off the viewmodel)
#define MSRDR_DRAWLATE (1 << 10)	//Hand model was queued as transparent so it gets drawn late.  It's really kRenderNormal

//mouth.sndavg
#define MSRDR_HASEXTRA (1 << 0) //The extra info is initialied

//Specifics
#define INSET_SCALE 0.026f //0.02f
#define ANIM_RUN 11
#define ANIM_CROUCH 0
#define ANIM_CRAWL 1
#define ANIM_SIT 19
#define ANIM_ATTENTION 4
#define ANIM_DEEPIDLE 5
#define ANIM_JUMP 6
#define ANIM_SIT 19

typedef struct msurface_s msurface_t;
typedef struct decal_s decal_t;

#include "clenv.h"
#include "tartan/textureloader.h"

//
//CRenderEntity -- Two modes of operation:  Either m_pEnt == (some other entity) or m_Ent == (local entity)
//				   Using GetEntity() will automatically return the correct entity
class CRenderEntity
{
public:
	CRenderEntity()
	{
		m_Visible = true;
		m_ClientEnt = true;
	}
	virtual void SetEntity(cl_entity_t *pEnt) { m_pEnt = pEnt; }
	virtual void CopyEnt(cl_entity_t &CopyEnt) { m_Ent = CopyEnt; }
	virtual void Render();
	virtual void Register();
	virtual void UnRegister();
	virtual bool IsPermanent() { return false; }
	virtual bool IsVisible() { return m_Visible; }
	virtual void SetVisible(bool Visible) { m_Visible = Visible; }
	virtual bool IsClientEntity() { return m_ClientEnt; }
	virtual void SetClientEntity(bool ClientEnt) { m_ClientEnt = ClientEnt; }
	virtual cl_entity_t &GetEntity() { return m_pEnt ? *m_pEnt : m_Ent; }

	//Callbacks
	virtual void CB_UnRegistered();

	cl_entity_t *m_pEnt = nullptr;
	cl_entity_t m_Ent;
	bool m_Visible,	 //Currently visible
		m_ClientEnt; //If true, this should be added to the entity list.  If false, it's a server ent
					 //and is already going to be rendered
};

class CRenderPlayer : public CRenderEntity
{
public:
	CRenderPlayer();
	void SetGear(CItemList *Gear) { m_pGear = Gear; } //Continuously use external gearlist
	void SetGear(CItemList Gear) { m_Gear = Gear; }	  //Copy the gearlist locally
	CItemList &GetGear() { return m_pGear ? *m_pGear : m_Gear; }
	void Render();
	bool IsPermanent() { return true; }
	void CB_UnRegistered() {}

	virtual void RenderGearItem(CGenericItem &Item);
	virtual cl_entity_t &GearItemEntity(CGenericItem &Item) { return Item.m_ClEntity[CGenericItem::ITEMENT_NORMAL]; }

	CItemList *m_pGear = nullptr;
	CItemList m_Gear;

	//cl_entity_t m_BodyParts[HUMAN_BODYPARTS];
	gender_e m_Gender;
};

// The on-screen HUD 3D inset of the local player
class CRenderPlayerInset : public CRenderPlayer
{
	void Render();
	void RenderGearItem(CGenericItem &Item);
	bool IsClientEntity() { return true; }
	cl_entity_t &GearItemEntity(CGenericItem &Item) { return Item.m_ClEntity[CGenericItem::ITEMENT_3DINSET]; }
};

class Plane
{
public:
	Vector m_Normal;
	float m_Dist;
	Plane() {}
	Plane(Vector &Normal, float Dist)
	{
		m_Normal = Normal;
		m_Dist = Dist;
	}
	Plane &operator=(const Plane &OtherPlane)
	{
		m_Normal = OtherPlane.m_Normal;
		m_Dist = OtherPlane.m_Dist;
		return *this;
	}

	//Distance of point from plane
	inline float GetDist(Vector &Point) { return DotProduct(Point, m_Normal) - m_Dist; }

	//Returns true if one point of the bbox is in front of the plane or intersecting the plane
	bool BBoxIsInFront(Vector Bounds[2])
	{
		Vector Point;
		for (int i = 0; i < 6; i++)
		{
			switch (i)
			{
			case 0:
				Point = Bounds[0];
				break;
			case 1:
				Point = Bounds[1];
				break;
			case 2:
				Point = Vector(Bounds[1].x, Bounds[0].y, Bounds[0].z);
				break;
			case 3:
				Point = Vector(Bounds[0].x, Bounds[1].y, Bounds[0].z);
				break;
			case 4:
				Point = Vector(Bounds[0].x, Bounds[0].y, Bounds[1].z);
				break;
			case 5:
				Point = Vector(Bounds[0].x, Bounds[1].y, Bounds[1].z);
				break;
			case 6:
				Point = Vector(Bounds[1].x, Bounds[0].y, Bounds[1].z);
				break;
			case 7:
				Point = Vector(Bounds[1].x, Bounds[1].y, Bounds[0].z);
				break;
			}

			float Dist = DotProduct(Point, m_Normal) - m_Dist;
			if (Dist >= 0)
				return true;
		}
		//float Dist = DotProduct( Point, m_Normal ) - m_Dist;
		//float Dist2 = DotProduct( Bounds[1], m_Normal ) - m_Dist;
		//return (Dist >= 0) || (Dist2 >= 0);
		return false;
	}
};

#include <GL/gl.h>	  // Header File For The OpenGL32 Library
#include <GL/glext.h>

#include "ref_params.h"

struct viewmgr_t
{
	Vector Origin, Angles, LastOrigin, LastAngles;
	ref_params_s *Params;
};
extern viewmgr_t ViewMgr;

class CParticle
{
public:
	CParticle();
	void SetAngles(Vector Angles);
	void BillBoard();
	bool LoadTexture(const char* Name);
	void Render();

	Vector m_Origin;
	Color4F m_Color;					   //RGBA, all [0-1]
	float m_Width, m_Height, m_Brightness; //Brightness [0-1]
	model_s *m_Texture;
	Vector m_DirForward, m_DirRight, m_DirUp;
	Vector2D m_TexCoords[4];
	bool m_Square, //If square, then use width for height
		m_DoubleSided, m_ContinuedParticle;
	uint m_GLTex, m_RenderMode;
	int m_SpriteFrame;
};

typedef void ParseAllSurfacesFunc(struct msurface_s *pSurface);
