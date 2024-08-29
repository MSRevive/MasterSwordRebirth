class CPortal : public CBaseEntity
{
public:
	//virtual int ObjectCaps( void ) { return FCAP_DONT_SAVE; }
	void Spawn(void){};
	void Precache(void){};
	void Spawn2(void);
	void CloseSpawnPortal(void);
};

#define SPRITE_TORCH 1

#include "effects.h"

class CMSSprite : public CSprite
{
public:
	int SpriteType;

	//Torch sprite
	void TorchInit(char *pszName, float flFramerate, float flScale, edict_t *peOwner);
	void Think();
};

#define TORCH_LIGHTS 2

class CTorchLight : public CBaseEntity
{
public:
	CBaseEntity *pLight[2];
	ulong m_OwnerID;

	//Torch sprite
	static void Create(float flDuration, edict_t *peOwner);
	void Spawn();
	void Think();
	void SUB_Remove();
};

//Makes entities glow for a time
class CEntGlow : public CBaseEntity
{
public:
	static CEntGlow *Create(CBaseEntity *pTarget, Vector Color, float Amount, float Duration, float FadeDuration);

	void Think();
	void SetGlow(bool On);

	entityinfo_t m_Target;
	float m_Amount, m_CurentAmount;
	float m_StartTime;
	float m_Duration;
	float m_FadeDuration;
	Vector m_Color;
};
