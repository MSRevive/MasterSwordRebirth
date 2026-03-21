/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
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
#ifndef EFFECTS_H
#define EFFECTS_H

enum {
	SF_BEAM_STARTON = 0x0001,
	SF_BEAM_TOGGLE = 0x0002,
	SF_BEAM_RANDOM = 0x0004,
	SF_BEAM_RING = 0x0008,
	SF_BEAM_SPARKSTART = 0x0010,
	SF_BEAM_SPARKEND = 0x0020,
	SF_BEAM_DECALS = 0x0040,
	SF_BEAM_SHADEIN = 0x0080,
	SF_BEAM_SHADEOUT = 0x0100,
	SF_BEAM_TEMPORARY = 0x8000,
	SF_SPRITE_STARTON = 0x0001,
	SF_SPRITE_ONCE = 0x0002,
	SF_SPRITE_TEMPORARY = 0x8000,
	SF_GIBSHOOTER_REPEATABLE = 1, // allows a gibshooter to be refired
	SF_FUNNEL_REVERSE = 1, // funnel effect repels particles instead of attracting them.
	SF_BUBBLES_STARTOFF = 0x0001,
	SF_BLOOD_RANDOM = 0x0001,
	SF_BLOOD_STREAM = 0x0002,
	SF_BLOOD_PLAYER = 0x0004,
	SF_BLOOD_DECAL = 0x0008,
	SF_SHAKE_EVERYONE = 0x0001, // Don't check radius
	// UNDONE: These don't work yet
	SF_SHAKE_DISRUPT = 0x0002, // Disrupt controls
	SF_SHAKE_INAIR = 0x0004,	// Shake players in air
	SF_FADE_IN = 0x0001,		// Fade in, not out
	SF_FADE_MODULATE = 0x0002, // Modulate, don't blend
	SF_FADE_ONLYONE = 0x0004,
	SF_MESSAGE_ONCE = 0x0001, // Fade in, not out
	SF_MESSAGE_ALL = 0x0002,  // Send to all clients
	SF_DETONATE = 0x0001,
	SF_LOOP = 1,
	SF_REMOVE_ON_FIRE = 2,
	SF_DECAL_NOTINDEATHMATCH = 2048,
	SF_WORLD_DARK = 0x0001,	  // Fade from black at startup
	SF_WORLD_TITLE = 0x0002,	  // Display game title at startup
	SF_WORLD_FORCETEAM = 0x0004 // Force teams

};


enum {
	SF_AUTO_FIREONCE = 0x0001,
	SF_TRIGGER_PUSH_START_OFF = 2,		   //spawnflag that makes trigger_push spawn turned OFF
	SF_TRIGGER_HURT_TARGETONCE = 1,	   // Only fire hurt target once
	SF_TRIGGER_HURT_START_OFF = 2,		   //spawnflag that makes trigger_push spawn turned OFF
	SF_TRIGGER_HURT_NO_CLIENTS = 8,	   //spawnflag that makes trigger_push spawn turned OFF
	SF_TRIGGER_HURT_CLIENTONLYFIRE = 16,  // trigger hurt will only fire its target if it is hurting a client
	SF_TRIGGER_HURT_CLIENTONLYTOUCH = 32, // only clients may touch this trigger.
	SF_RELAY_FIREONCE = 0x0001,
	SF_MULTIMAN_CLONE = 0x80000000,
	SF_MULTIMAN_THREAD = 0x00000001,
	SF_RENDER_MASKFX = (1 << 0),
	SF_RENDER_MASKAMT = (1 << 1),
	SF_RENDER_MASKMODE = (1 << 2),
	SF_RENDER_MASKCOLOR = (1 << 3),
	SF_CHANGELEVEL_USEONLY = 0x0002,
	SF_ENDSECTION_USEONLY = 0x0001,
	SF_CAMERA_PLAYER_POSITION = 1,
	SF_CAMERA_PLAYER_TARGET = 2,
	SF_CAMERA_PLAYER_TAKECONTROL = 4,
	SF_CAMERA_PLAYER_ALL = 8
};

class CSprite : public CPointEntity
{
public:
	void Spawn(void);
	void Precache(void);

	int ObjectCaps(void)
	{
		int flags = 0;
		if (pev->spawnflags & SF_SPRITE_TEMPORARY)
			flags = FCAP_DONT_SAVE;
		return (CBaseEntity ::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | flags;
	}
	void EXPORT AnimateThink(void);
	void EXPORT ExpandThink(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void Animate(float frames);
	void Expand(float scaleSpeed, float fadeSpeed);
	void SpriteInit(const char *pSpriteName, const Vector &origin);

	inline void SetAttachment(edict_t *pEntity, int attachment)
	{
		if (pEntity)
		{
			pev->skin = ENTINDEX(pEntity);
			pev->body = attachment;
			pev->aiment = pEntity;
			pev->movetype = MOVETYPE_FOLLOW;
		}
	}
	void TurnOff(void);
	void TurnOn(void);
	inline float Frames(void) { return m_maxFrame; }
	inline void SetTransparency(int rendermode, int r, int g, int b, int a, int fx)
	{
		pev->rendermode = rendermode;
		pev->rendercolor.x = r;
		pev->rendercolor.y = g;
		pev->rendercolor.z = b;
		pev->renderamt = a;
		pev->renderfx = fx;
	}
	inline void SetTexture(int spriteIndex) { pev->modelindex = spriteIndex; }
	inline void SetScale(float scale) { pev->scale = scale; }
	inline void SetColor(int r, int g, int b)
	{
		pev->rendercolor.x = r;
		pev->rendercolor.y = g;
		pev->rendercolor.z = b;
	}
	inline void SetBrightness(int brightness) { pev->renderamt = brightness; }

	inline void AnimateAndDie(float framerate)
	{
		SetThink(&CSprite::AnimateUntilDead);
		pev->framerate = framerate;
		pev->dmgtime = gpGlobals->time + (m_maxFrame / framerate);
		pev->nextthink = gpGlobals->time;
	}

	void EXPORT AnimateUntilDead(void);

	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);
	static TYPEDESCRIPTION m_SaveData[];
	static CSprite *SpriteCreate(const char *pSpriteName, const Vector &origin, BOOL animate);

private:
	float m_lastTime;
	float m_maxFrame;
};

class CBeam : public CBaseEntity
{
public:
	void Spawn(void);
	void Precache(void);
	int ObjectCaps(void)
	{
		int flags = 0;
		if (pev->spawnflags & SF_BEAM_TEMPORARY)
			flags = FCAP_DONT_SAVE;
		return (CBaseEntity ::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | flags;
	}

	void EXPORT TriggerTouch(CBaseEntity *pOther);

	// These functions are here to show the way beams are encoded as entities.
	// Encoding beams as entities simplifies their management in the client/server architecture
	inline void SetType(int type) { pev->rendermode = (pev->rendermode & 0xF0) | (type & 0x0F); }
	inline void SetFlags(int flags) { pev->rendermode = (pev->rendermode & 0x0F) | (flags & 0xF0); }
	inline void SetStartPos(const Vector &pos) { pev->origin = pos; }
	inline void SetEndPos(const Vector &pos) { pev->angles = pos; }
	void SetStartEntity(int entityIndex);
	void SetEndEntity(int entityIndex);

	inline void SetStartAttachment(int attachment) { pev->sequence = (pev->sequence & 0x0FFF) | ((attachment & 0xF) << 12); }
	inline void SetEndAttachment(int attachment) { pev->skin = (pev->skin & 0x0FFF) | ((attachment & 0xF) << 12); }

	inline void SetTexture(int spriteIndex) { pev->modelindex = spriteIndex; }
	inline void SetWidth(int width) { pev->scale = width; }
	inline void SetNoise(int amplitude) { pev->body = amplitude; }
	inline void SetColor(int r, int g, int b)
	{
		pev->rendercolor.x = r;
		pev->rendercolor.y = g;
		pev->rendercolor.z = b;
	}
	inline void SetBrightness(int brightness) { pev->renderamt = brightness; }
	inline void SetFrame(float frame) { pev->frame = frame; }
	inline void SetScrollRate(int speed) { pev->animtime = speed; }

	inline int GetType(void) { return pev->rendermode & 0x0F; }
	inline int GetFlags(void) { return pev->rendermode & 0xF0; }
	inline int GetStartEntity(void) { return pev->sequence & 0xFFF; }
	inline int GetEndEntity(void) { return pev->skin & 0xFFF; }

	const Vector &GetStartPos(void);
	const Vector &GetEndPos(void);

	Vector Center(void) { return (GetStartPos() + GetEndPos()) * 0.5; }; // center point of beam

	inline int GetTexture(void) { return pev->modelindex; }
	inline int GetWidth(void) { return pev->scale; }
	inline int GetNoise(void) { return pev->body; }
	// inline void GetColor( int r, int g, int b ) { pev->rendercolor.x = r; pev->rendercolor.y = g; pev->rendercolor.z = b; }
	inline int GetBrightness(void) { return pev->renderamt; }
	inline int GetFrame(void) { return pev->frame; }
	inline int GetScrollRate(void) { return pev->animtime; }

	// Call after you change start/end positions
	void RelinkBeam(void);
	//	void		SetObjectCollisionBox( void );

	void DoSparks(const Vector &start, const Vector &end);
	CBaseEntity *RandomTargetname(const char *szName);
	void BeamDamage(TraceResult *ptr);
	// Init after BeamCreate()
	void BeamInit(const char *pSpriteName, int width);
	void PointsInit(const Vector &start, const Vector &end);
	void PointEntInit(const Vector &start, int endIndex);
	void EntsInit(int startIndex, int endIndex);
	void HoseInit(const Vector &start, const Vector &direction);

	static CBeam *BeamCreate(const char *pSpriteName, int width);

	inline void LiveForTime(float time)
	{
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + time;
	}
	inline void BeamDamageInstant(TraceResult *ptr, float damage)
	{
		pev->dmg = damage;
		pev->dmgtime = gpGlobals->time - 1;
		BeamDamage(ptr);
	}
};



class CLaser : public CBeam
{
public:
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData *pkvd);

	void TurnOn(void);
	void TurnOff(void);
	int IsOn(void);

	void FireAtPoint(TraceResult &point);

	void EXPORT StrikeThink(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);
	static TYPEDESCRIPTION m_SaveData[];

	CSprite *m_pSprite;
	int m_iszSpriteName;
	Vector m_firePosition;
};

#endif //EFFECTS_H
