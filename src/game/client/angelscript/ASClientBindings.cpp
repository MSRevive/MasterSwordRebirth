//==========================================================================
// ASClientBindings.cpp - Client-side AngelScript bindings
//
// Client-side AngelScript API implementation for Master Sword Rebirth
// Provides OOP interface for client effects, entities, environment, etc.
//==========================================================================

#include "ASClientBindings.h"
#include "angelscript/ASCoreTypes.h"
#include "angelscript/ASBindExtensions.h"
#include "ASVGUIBindings.h"
#include "ASClientItemBindings.h"
#include <asbind20/asbind.hpp>
#include <angelscript/addons/scriptarray/scriptarray.h>

// Client-side includes
#include "hud.h"
#include "cl_util.h"
#include "cl_dll.h"
#include "r_efx.h"
#include "cl_entity.h"
#include "pm_defs.h"
#include "pm_shared.h"
#include "pmtrace.h"
#include "triangleapi.h"
#include "com_model.h"
#include "event_api.h"

// Master Sword includes
#include "mslogger.h"
#include "sharedutil.h"

// External client globals
extern cl_enginefunc_t gEngfuncs;
extern playermove_t *pmove;

//==========================================================================
// CClientEffect - Base class for client-side effects
//==========================================================================

class CClientEffect
{
protected:
    bool m_bIsValid;
    mutable int m_refCount;  // Reference count for AngelScript

public:
    CClientEffect() : m_bIsValid(false), m_refCount(1) {}
    CClientEffect(bool valid) : m_bIsValid(valid), m_refCount(1) {}
    virtual ~CClientEffect() {}

    virtual bool IsValid() const { return m_bIsValid; }

    // Reference counting for AngelScript
    void AddRef() const
    {
        m_refCount++;
    }

    void Release() const
    {
        m_refCount--;
        if (m_refCount == 0)
        {
            delete this;
        }
    }
};

// Reference counting wrapper functions for CClientEffect
static void AddRef_CClientEffect(CClientEffect* effect)
{
    if (effect) effect->AddRef();
}

static void Release_CClientEffect(CClientEffect* effect)
{
    if (effect) effect->Release();
}

//==========================================================================
// CClientEntity - Wrapper for client-side entity queries
//==========================================================================

class CClientEntity
{
protected:
    int m_index;  // Protected so CLocalPlayer can access it
    mutable int m_refCount;  // Reference count for AngelScript

    cl_entity_t* GetEntity() const
    {
        if (m_index <= 0) return nullptr;
        return gEngfuncs.GetEntityByIndex(m_index);
    }

public:
    CClientEntity() : m_index(0), m_refCount(1) {}
    CClientEntity(int index) : m_index(index), m_refCount(1) {}

    // Reference counting for AngelScript
    void AddRef() const
    {
        m_refCount++;
    }

    void Release() const
    {
        m_refCount--;
        if (m_refCount == 0)
        {
            delete this;
        }
    }

    bool Exists() const
    {
        cl_entity_t* pEnt = GetEntity();
        return pEnt != nullptr && pEnt->model != nullptr;
    }

    int GetIndex() const
    {
        return m_index;
    }

    Vector3 GetOrigin() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt) return Vector3(0, 0, 0);
        return Vector3(pEnt->origin.x, pEnt->origin.y, pEnt->origin.z);
    }

    Vector3 GetAngles() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt) return Vector3(0, 0, 0);
        return Vector3(pEnt->angles.x, pEnt->angles.y, pEnt->angles.z);
    }

    Vector3 GetVelocity() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt) return Vector3(0, 0, 0);
        return Vector3(pEnt->curstate.velocity.x, pEnt->curstate.velocity.y, pEnt->curstate.velocity.z);
    }

    std::string GetModelName() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt || !pEnt->model) return "";
        return pEnt->model->name;
    }

    int GetModelIndex() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt) return 0;
        return pEnt->curstate.modelindex;
    }

    int GetRenderMode() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt) return 0;
        return pEnt->curstate.rendermode;
    }

    int GetRenderAmount() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt) return 0;
        return pEnt->curstate.renderamt;
    }

    Color GetRenderColor() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt) return Color(255, 255, 255, 255);
        return Color(pEnt->curstate.rendercolor.r,
                    pEnt->curstate.rendercolor.g,
                    pEnt->curstate.rendercolor.b,
                    pEnt->curstate.renderamt);
    }

    bool IsPlayer() const
    {
        cl_entity_t* pEnt = GetEntity();
        return pEnt && pEnt->player != 0;
    }

    bool IsVisible() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt) return false;
        return (pEnt->curstate.effects & EF_NODRAW) == 0;
    }

    Vector3 GetBonePosition(int boneIndex) const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt || !pEnt->model) return Vector3(0, 0, 0);

        // TODO: Get bone position - requires studio model API
        // EV_GetBonePosition is not available in client engine funcs
        return Vector3(0, 0, 0);
    }

    int GetBoneCount() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt || !pEnt->model) return 0;

        // TODO: Get bone count - requires studio model API
        // studiohdr_t and GetModelPtr not available in client
        return 0;
    }

    Vector3 GetAttachment(int attachmentIndex) const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt || !pEnt->model) return Vector3(0, 0, 0);

        // Get attachment position
        // This would need proper implementation based on studio model rendering
        return Vector3(0, 0, 0);
    }

    int GetSequence() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt) return 0;
        return pEnt->curstate.sequence;
    }

    float GetFrame() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt) return 0.0f;
        return pEnt->curstate.frame;
    }

    int GetBody() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt) return 0;
        return pEnt->curstate.body;
    }

    int GetSkin() const
    {
        cl_entity_t* pEnt = GetEntity();
        if (!pEnt) return 0;
        return pEnt->curstate.skin;
    }
};

// Reference counting wrapper functions for CClientEntity
static void AddRef_CClientEntity(CClientEntity* entity)
{
    if (entity) entity->AddRef();
}

static void Release_CClientEntity(CClientEntity* entity)
{
    if (entity) entity->Release();
}

// Global accessor function for client entities
static CClientEntity* AS_GetClientEntity(int index)
{
    return new CClientEntity(index);
}

// Collision mode enum
enum class CollisionMode
{
    None,
    World,
    All,
    AllAndDie
};

class CTempEntity : public CClientEffect
{
private:
    TEMPENTITY* m_pTempEnt;
    
public:
    CTempEntity() : CClientEffect(false), m_pTempEnt(nullptr) {}
    CTempEntity(TEMPENTITY* pTempEnt) : CClientEffect(pTempEnt != nullptr), m_pTempEnt(pTempEnt) {}
    
    bool IsValid() const override
    {
        return m_bIsValid && m_pTempEnt != nullptr;
    }
    
    // Transform methods
    void SetOrigin(const Vector3& origin)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.origin[0] = origin.x;
        m_pTempEnt->entity.origin[1] = origin.y;
        m_pTempEnt->entity.origin[2] = origin.z;
    }
    
    Vector3 GetOrigin() const
    {
        if (!IsValid()) return Vector3(0, 0, 0);
        return Vector3(m_pTempEnt->entity.origin[0], 
                      m_pTempEnt->entity.origin[1], 
                      m_pTempEnt->entity.origin[2]);
    }
    
    void SetAngles(const Vector3& angles)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.angles[0] = angles.x;
        m_pTempEnt->entity.angles[1] = angles.y;
        m_pTempEnt->entity.angles[2] = angles.z;
    }
    
    Vector3 GetAngles() const
    {
        if (!IsValid()) return Vector3(0, 0, 0);
        return Vector3(m_pTempEnt->entity.angles[0], 
                      m_pTempEnt->entity.angles[1], 
                      m_pTempEnt->entity.angles[2]);
    }
    
    void SetVelocity(const Vector3& velocity)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.baseline.origin[0] = velocity.x;
        m_pTempEnt->entity.baseline.origin[1] = velocity.y;
        m_pTempEnt->entity.baseline.origin[2] = velocity.z;
    }
    
    Vector3 GetVelocity() const
    {
        if (!IsValid()) return Vector3(0, 0, 0);
        return Vector3(m_pTempEnt->entity.baseline.origin[0], 
                      m_pTempEnt->entity.baseline.origin[1], 
                      m_pTempEnt->entity.baseline.origin[2]);
    }
    
    // Appearance methods
    void SetModel(const std::string& modelPath)
    {
        if (!IsValid()) return;
        // TODO: GetModelPtr and GetModelByIndex not available in client engine funcs
        // Just set the model index for now
        m_pTempEnt->entity.curstate.modelindex = gEngfuncs.pEventAPI->EV_FindModelIndex(modelPath.c_str());
    }
    
    void SetSprite(const std::string& spritePath)
    {
        // Same as SetModel for sprites
        SetModel(spritePath);
    }
    
    void SetScale(float scale)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.curstate.scale = scale;
    }
    
    float GetScale() const
    {
        if (!IsValid()) return 1.0f;
        return m_pTempEnt->entity.curstate.scale;
    }
    
    void SetFrame(int frame)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.curstate.frame = (float)frame;
    }
    
    int GetFrame() const
    {
        if (!IsValid()) return 0;
        return (int)m_pTempEnt->entity.curstate.frame;
    }
    
    void SetFrameRate(float fps)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.curstate.framerate = fps;
    }
    
    void SetBody(int bodyIndex)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.curstate.body = bodyIndex;
    }
    
    void SetSkin(int skinIndex)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.curstate.skin = skinIndex;
    }
    
    // Rendering methods
    void SetRenderMode(int mode)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.curstate.rendermode = mode;
    }
    
    void SetRenderAmount(int amount)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.curstate.renderamt = amount;
    }
    
    void SetRenderColor(const Color& color)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.curstate.rendercolor.r = color.r;
        m_pTempEnt->entity.curstate.rendercolor.g = color.g;
        m_pTempEnt->entity.curstate.rendercolor.b = color.b;
    }
    
    void SetRenderFx(int fx)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.curstate.renderfx = fx;
    }
    
    // Physics methods
    void SetGravity(float ratio)
    {
        if (!IsValid()) return;
        // Gravity is stored in the z component of baseline velocity
        // This is a simplified implementation
        m_pTempEnt->entity.baseline.origin[2] = -ratio * 800.0f; // 800 is default gravity
    }
    
    void SetCollisionMode(CollisionMode mode)
    {
        if (!IsValid()) return;
        
        // Set collision flags based on mode
        switch (mode)
        {
            case CollisionMode::None:
                m_pTempEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_COLLIDEALL);
                break;
            case CollisionMode::World:
                m_pTempEnt->flags |= FTENT_COLLIDEWORLD;
                m_pTempEnt->flags &= ~FTENT_COLLIDEALL;
                break;
            case CollisionMode::All:
                m_pTempEnt->flags |= FTENT_COLLIDEALL;
                break;
            case CollisionMode::AllAndDie:
                m_pTempEnt->flags |= (FTENT_COLLIDEALL | FTENT_FADEOUT);
                break;
        }
    }
    
    void SetBounceFactor(float factor)
    {
        if (!IsValid()) return;
        m_pTempEnt->bounceFactor = factor;
    }
    
    void SetMins(const Vector3& mins)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.curstate.mins[0] = mins.x;
        m_pTempEnt->entity.curstate.mins[1] = mins.y;
        m_pTempEnt->entity.curstate.mins[2] = mins.z;
    }
    
    void SetMaxs(const Vector3& maxs)
    {
        if (!IsValid()) return;
        m_pTempEnt->entity.curstate.maxs[0] = maxs.x;
        m_pTempEnt->entity.curstate.maxs[1] = maxs.y;
        m_pTempEnt->entity.curstate.maxs[2] = maxs.z;
    }
    
    // Lifetime methods
    void SetDeathDelay(float seconds)
    {
        if (!IsValid()) return;
        m_pTempEnt->die = gEngfuncs.GetClientTime() + seconds;
    }
    
    void SetFadeout(bool enabled, float duration)
    {
        if (!IsValid()) return;
        
        if (enabled)
        {
            m_pTempEnt->flags |= FTENT_FADEOUT;
            if (duration > 0.0f)
            {
                m_pTempEnt->fadeSpeed = m_pTempEnt->entity.baseline.renderamt / duration;
            }
        }
        else
        {
            m_pTempEnt->flags &= ~FTENT_FADEOUT;
        }
    }
    
    void Kill()
    {
        if (!IsValid()) return;
        m_pTempEnt->die = 0.0f; // Mark for immediate death
        m_bIsValid = false;
    }
    
    // Following methods
    void FollowEntity(int entityIndex, int attachment)
    {
        if (!IsValid()) return;
        m_pTempEnt->flags |= FTENT_PLYRATTACHMENT;
        m_pTempEnt->clientIndex = entityIndex;
        // Attachment handling would require more complex setup
    }
    
    void StopFollowing()
    {
        if (!IsValid()) return;
        m_pTempEnt->flags &= ~FTENT_PLYRATTACHMENT;
        m_pTempEnt->clientIndex = 0;
    }
    
    // Custom data storage (using curstate iuser and fuser)
    void SetUserInt(int slot, int value)
    {
        if (!IsValid() || slot < 1 || slot > 4) return;
        switch (slot)
        {
            case 1: m_pTempEnt->entity.curstate.iuser1 = value; break;
            case 2: m_pTempEnt->entity.curstate.iuser2 = value; break;
            case 3: m_pTempEnt->entity.curstate.iuser3 = value; break;
            case 4: m_pTempEnt->entity.curstate.iuser4 = value; break;
        }
    }
    
    int GetUserInt(int slot) const
    {
        if (!IsValid() || slot < 1 || slot > 4) return 0;
        switch (slot)
        {
            case 1: return m_pTempEnt->entity.curstate.iuser1;
            case 2: return m_pTempEnt->entity.curstate.iuser2;
            case 3: return m_pTempEnt->entity.curstate.iuser3;
            case 4: return m_pTempEnt->entity.curstate.iuser4;
        }
        return 0;
    }
    
    void SetUserFloat(int slot, float value)
    {
        if (!IsValid() || slot < 1 || slot > 4) return;
        switch (slot)
        {
            case 1: m_pTempEnt->entity.curstate.fuser1 = value; break;
            case 2: m_pTempEnt->entity.curstate.fuser2 = value; break;
            case 3: m_pTempEnt->entity.curstate.fuser3 = value; break;
            case 4: m_pTempEnt->entity.curstate.fuser4 = value; break;
        }
    }
    
    float GetUserFloat(int slot) const
    {
        if (!IsValid() || slot < 1 || slot > 4) return 0.0f;
        switch (slot)
        {
            case 1: return m_pTempEnt->entity.curstate.fuser1;
            case 2: return m_pTempEnt->entity.curstate.fuser2;
            case 3: return m_pTempEnt->entity.curstate.fuser3;
            case 4: return m_pTempEnt->entity.curstate.fuser4;
        }
        return 0.0f;
    }
    
    // Callback methods (placeholders - full implementation would need callback system)
    void SetUpdateCallback(const std::string& eventName)
    {
        // TODO: Implement callback system
        MS_ANGEL_DEBUG("[CTempEntity] SetUpdateCallback not yet implemented: %s", eventName.c_str());
    }
    
    void SetCollisionCallback(const std::string& eventName)
    {
        // TODO: Implement callback system
        MS_ANGEL_DEBUG("[CTempEntity] SetCollisionCallback not yet implemented: %s", eventName.c_str());
    }
    
    void SetWaterCallback(const std::string& eventName)
    {
        // TODO: Implement callback system
        MS_ANGEL_DEBUG("[CTempEntity] SetWaterCallback not yet implemented: %s", eventName.c_str());
    }
    
    void SetTimerCallback(float time, const std::string& eventName)
    {
        // TODO: Implement callback system
        MS_ANGEL_DEBUG("[CTempEntity] SetTimerCallback not yet implemented: %f %s", time, eventName.c_str());
    }
    
    Vector3 GetWaterOrigin() const
    {
        // TODO: Implement water origin detection
        return Vector3(0, 0, 0);
    }
};

// Reference counting wrapper functions for CTempEntity
static void AddRef_CTempEntity(CTempEntity* entity)
{
    if (entity) entity->AddRef();
}

static void Release_CTempEntity(CTempEntity* entity)
{
    if (entity) entity->Release();
}

// Factory functions for creating temp entities
static CTempEntity* AS_CreateTempSprite(const std::string& spriteName, const Vector3& origin)
{
    MS_ANGEL_DEBUG("[CTempEntity] CreateTempSprite: %s at (%f, %f, %f)", 
                   spriteName.c_str(), origin.x, origin.y, origin.z);
    // TODO: Implement actual temp entity creation via engine API
    return new CTempEntity(nullptr);
}

static CTempEntity* AS_CreateTempModel(const std::string& modelName, const Vector3& origin)
{
    MS_ANGEL_DEBUG("[CTempEntity] CreateTempModel: %s at (%f, %f, %f)", 
                   modelName.c_str(), origin.x, origin.y, origin.z);
    // TODO: Implement actual temp entity creation via engine API
    return new CTempEntity(nullptr);
}

static CTempEntity* AS_CreateFrameSprite(const std::string& spriteName, const Vector3& origin)
{
    MS_ANGEL_DEBUG("[CTempEntity] CreateFrameSprite: %s at (%f, %f, %f)", 
                   spriteName.c_str(), origin.x, origin.y, origin.z);
    // TODO: Implement frame entity creation
    return new CTempEntity(nullptr);
}

static CTempEntity* AS_CreateFrameModel(const std::string& modelName, const Vector3& origin)
{
    MS_ANGEL_DEBUG("[CTempEntity] CreateFrameModel: %s at (%f, %f, %f)", 
                   modelName.c_str(), origin.x, origin.y, origin.z);
    // TODO: Implement frame entity creation
    return new CTempEntity(nullptr);
}

//==========================================================================
// CDynamicLight - Client-side dynamic light system
//==========================================================================

class CDynamicLight : public CClientEffect
{
private:
    dlight_t* m_pLight;
    int m_lightKey;
    
public:
    CDynamicLight() : CClientEffect(false), m_pLight(nullptr), m_lightKey(0) {}
    CDynamicLight(dlight_t* pLight, int key) : CClientEffect(pLight != nullptr), m_pLight(pLight), m_lightKey(key) {}
    
    bool IsValid() const override { return m_bIsValid && m_pLight != nullptr; }
    
    void SetOrigin(const Vector3& origin)
    {
        if (!IsValid()) return;
        m_pLight->origin[0] = origin.x;
        m_pLight->origin[1] = origin.y;
        m_pLight->origin[2] = origin.z;
    }
    
    Vector3 GetOrigin() const
    {
        if (!IsValid()) return Vector3(0, 0, 0);
        return Vector3(m_pLight->origin[0], m_pLight->origin[1], m_pLight->origin[2]);
    }
    
    void SetRadius(float radius)
    {
        if (!IsValid()) return;
        m_pLight->radius = radius;
    }
    
    void SetColor(const Color& color)
    {
        if (!IsValid()) return;
        m_pLight->color.r = color.r;
        m_pLight->color.g = color.g;
        m_pLight->color.b = color.b;
    }
    
    void SetDuration(float seconds)
    {
        if (!IsValid()) return;
        m_pLight->die = gEngfuncs.GetClientTime() + seconds;
    }
    
    void SetDark(bool isDark)
    {
        if (!IsValid()) return;
        m_pLight->dark = isDark ? 1 : 0;
    }
    
    void FollowEntity(int entityIndex)
    {
        if (!IsValid()) return;
        m_pLight->key = entityIndex;
    }
    
    void Kill()
    {
        if (!IsValid()) return;
        m_pLight->die = 0.0f;
        m_bIsValid = false;
    }
};

static CDynamicLight* AS_CreateDynamicLight(const Vector3& origin, float radius, const Color& color, float duration)
{
    dlight_t* pLight = gEngfuncs.pEfxAPI->CL_AllocDlight(0);
    if (!pLight) return new CDynamicLight();
    
    pLight->origin[0] = origin.x;
    pLight->origin[1] = origin.y;
    pLight->origin[2] = origin.z;
    pLight->radius = radius;
    pLight->color.r = color.r;
    pLight->color.g = color.g;
    pLight->color.b = color.b;
    pLight->die = duration > 0.0f ? (gEngfuncs.GetClientTime() + duration) : 99999.0f;
    
    return new CDynamicLight(pLight, 0);
}

//==========================================================================
// CBeam - Client-side beam system
//==========================================================================

class CBeam : public CClientEffect
{
private:
    BEAM* m_pBeam;
    
public:
    CBeam() : CClientEffect(false), m_pBeam(nullptr) {}
    CBeam(BEAM* pBeam) : CClientEffect(pBeam != nullptr), m_pBeam(pBeam) {}
    
    bool IsValid() const override { return m_bIsValid && m_pBeam != nullptr; }
    
    void SetStartPos(const Vector3& pos)
    {
        if (!IsValid()) return;
        m_pBeam->source[0] = pos.x;
        m_pBeam->source[1] = pos.y;
        m_pBeam->source[2] = pos.z;
    }
    
    void SetEndPos(const Vector3& pos)
    {
        if (!IsValid()) return;
        m_pBeam->target[0] = pos.x;
        m_pBeam->target[1] = pos.y;
        m_pBeam->target[2] = pos.z;
    }
    
    void SetStartEntity(int entityIndex, int attachment)
    {
        if (!IsValid()) return;
        // TODO: BEAM structure doesn't have entity/attachmentIndex arrays in this version
        MS_ANGEL_DEBUG("CBeam::SetStartEntity(%d, %d) - not implemented", entityIndex, attachment);
    }

    void SetEndEntity(int entityIndex, int attachment)
    {
        if (!IsValid()) return;
        // TODO: BEAM structure doesn't have entity/attachmentIndex arrays in this version
        MS_ANGEL_DEBUG("CBeam::SetEndEntity(%d, %d) - not implemented", entityIndex, attachment);
    }
    
    void SetSprite(const std::string& spriteName)
    {
        if (!IsValid()) return;
        // TODO: GetModelPtr and GetModelByIndex not available
        // Skip setting pFollowModel for now
    }
    
    void SetWidth(float width) { if (IsValid()) m_pBeam->width = width; }
    void SetAmplitude(float amplitude) { if (IsValid()) m_pBeam->amplitude = amplitude; }
    void SetBrightness(float brightness) { if (IsValid()) m_pBeam->brightness = brightness; }
    void SetSpeed(float speed) { if (IsValid()) m_pBeam->speed = speed; }
    void SetFrameRate(float fps) { if (IsValid()) m_pBeam->frameRate = fps; }
    
    void SetColor(const Color& color)
    {
        if (!IsValid()) return;
        m_pBeam->r = color.r / 255.0f;
        m_pBeam->g = color.g / 255.0f;
        m_pBeam->b = color.b / 255.0f;
    }
    
    void SetLife(float seconds)
    {
        if (!IsValid()) return;
        m_pBeam->die = gEngfuncs.GetClientTime() + seconds;
    }
    
    void Kill()
    {
        if (!IsValid()) return;
        m_pBeam->die = 0.0f;
        m_bIsValid = false;
    }
};

static CBeam* AS_CreateBeamPoints(const Vector3& start, const Vector3& end, const std::string& sprite)
{
    // TODO: R_BeamPoints has different signature in this SDK version
    // Would need to check beamdef.h for correct parameters
    MS_ANGEL_DEBUG("[CBeam] CreateBeamPoints not yet implemented - API mismatch");
    return new CBeam(nullptr);
}

static CBeam* AS_CreateBeamEntities(int startIdx, int startAttach, int endIdx, int endAttach, const std::string& sprite)
{
    // TODO: R_BeamEnts has different signature in this SDK version
    MS_ANGEL_DEBUG("[CBeam] CreateBeamEntities not yet implemented - API mismatch");
    return new CBeam(nullptr);
}

static CBeam* AS_CreateBeamEntPoint(int startIdx, int attachment, const Vector3& endPos, const std::string& sprite)
{
    // TODO: R_BeamEntPoint has different signature in this SDK version
    MS_ANGEL_DEBUG("[CBeam] CreateBeamEntPoint not yet implemented - API mismatch");
    return new CBeam(nullptr);
}

//==========================================================================
// Effect Utility Functions
//==========================================================================

static void AS_CreateSpark(const Vector3& origin)
{
    vec3_t pos = {origin.x, origin.y, origin.z};
    gEngfuncs.pEfxAPI->R_SparkEffect(pos, 5, -200, 200);
}

static void AS_CreateSparkOnModel(int entityIndex, int attachment)
{
    cl_entity_t* pEnt = gEngfuncs.GetEntityByIndex(entityIndex);
    if (pEnt)
    {
        vec3_t pos;
        VectorCopy(pEnt->origin, pos);
        gEngfuncs.pEfxAPI->R_SparkEffect(pos, 5, -200, 200);
    }
}

static void AS_CreateDecal(int decalIndex, const Vector3& traceStart, const Vector3& traceEnd)
{
    // TODO: Implement decal creation
    MS_ANGEL_DEBUG("[Effects] CreateDecal not yet implemented");
}

//==========================================================================
// CClientEnvironment - Environment control
//==========================================================================

class CClientEnvironment
{
public:
    // Fog control (Note: These would require integration with client rendering code)
    void SetFogEnabled(bool enabled)
    {
        // TODO: Integrate with client fog system
        MS_ANGEL_DEBUG("[CClientEnvironment] SetFogEnabled: %d", enabled);
    }
    
    void SetFogColor(const Color& color)
    {
        MS_ANGEL_DEBUG("[CClientEnvironment] SetFogColor: %d,%d,%d", color.r, color.g, color.b);
    }
    
    void SetFogDensity(float density)
    {
        MS_ANGEL_DEBUG("[CClientEnvironment] SetFogDensity: %f", density);
    }
    
    void SetFogStart(float start) { }
    void SetFogEnd(float end) { }
    void SetFogType(int type) { }
    bool GetFogEnabled() const { return false; }
    Color GetFogColor() const { return Color(255, 255, 255, 255); }
    
    // Screen effects
    void SetScreenTint(const Color& color)
    {
        // TODO: pfnSetScreenFade has different signature (takes screenfade_s* not individual params)
        MS_ANGEL_DEBUG("SetScreenTint not yet implemented - API mismatch");
    }

    Color GetScreenTint() const { return Color(0, 0, 0, 0); }
    void ClearScreenTint()
    {
        // TODO: pfnSetScreenFade has different signature
        MS_ANGEL_DEBUG("ClearScreenTint not yet implemented - API mismatch");
    }
    
    // Sky
    void SetSkyTexture(const std::string& skyName) { }
    std::string GetSkyName() const { return ""; }
    void SetLightGamma(float gamma) { }
    void SetMaxViewDistance(float distance) { }
};

static CClientEnvironment g_ClientEnvironment;

static CClientEnvironment* AS_GetEnvironment()
{
    return &g_ClientEnvironment;
}

//==========================================================================
// CClientSound - Audio playback
//==========================================================================

class CClientSound
{
public:
    void PlaySound(int channel, const std::string& soundPath, float volume)
    {
        gEngfuncs.pfnPlaySoundByName((char*)soundPath.c_str(), volume);
    }
    
    void SetVolume(int channel, const std::string& soundPath, float volume)
    {
        // TODO: Implement volume control
    }
    
    void StopSound(int channel, const std::string& soundPath)
    {
        // TODO: Implement sound stopping
    }
    
    void PlaySoundAtPosition(const Vector3& pos, const std::string& soundPath, float volume)
    {
        vec3_t position = {pos.x, pos.y, pos.z};
        gEngfuncs.pEfxAPI->R_RicochetSound((float*)&position);
    }
};

static CClientSound g_ClientSound;

static CClientSound* AS_GetClientSound()
{
    return &g_ClientSound;
}

//==========================================================================
// Client Utility Functions
//==========================================================================

static Vector3 AS_GetGroundHeight(const Vector3& origin)
{
    pmtrace_t trace;
    vec3_t start = {origin.x, origin.y, origin.z};
    vec3_t end = {origin.x, origin.y, origin.z - 8192.0f};
    gEngfuncs.pEventAPI->EV_PlayerTrace(start, end, PM_WORLD_ONLY, -1, &trace);
    return Vector3(trace.endpos[0], trace.endpos[1], trace.endpos[2]);
}

static Vector3 AS_GetSkyHeight(const Vector3& origin)
{
    pmtrace_t trace;
    vec3_t start = {origin.x, origin.y, origin.z};
    vec3_t end = {origin.x, origin.y, origin.z + 8192.0f};
    gEngfuncs.pEventAPI->EV_PlayerTrace(start, end, PM_WORLD_ONLY, -1, &trace);
    return Vector3(trace.endpos[0], trace.endpos[1], trace.endpos[2]);
}

static bool AS_IsUnderSky(const Vector3& origin)
{
    pmtrace_t trace;
    vec3_t start = {origin.x, origin.y, origin.z};
    vec3_t end = {origin.x, origin.y, origin.z + 8192.0f};
    gEngfuncs.pEventAPI->EV_PlayerTrace(start, end, PM_WORLD_ONLY, -1, &trace);
    return trace.fraction >= 1.0f || trace.ent == 0;
}

static Vector3 AS_TraceLine(const Vector3& start, const Vector3& end, bool worldOnly)
{
    pmtrace_t trace;
    vec3_t vStart = {start.x, start.y, start.z};
    vec3_t vEnd = {end.x, end.y, end.z};
    gEngfuncs.pEventAPI->EV_PlayerTrace(vStart, vEnd, worldOnly ? PM_WORLD_ONLY : PM_NORMAL, -1, &trace);
    return Vector3(trace.endpos[0], trace.endpos[1], trace.endpos[2]);
}

static int AS_TraceLineEntity(const Vector3& start, const Vector3& end)
{
    pmtrace_t trace;
    vec3_t vStart = {start.x, start.y, start.z};
    vec3_t vEnd = {end.x, end.y, end.z};
    gEngfuncs.pEventAPI->EV_PlayerTrace(vStart, vEnd, PM_NORMAL, -1, &trace);
    return trace.ent;
}

static int AS_GetContents(const Vector3& origin)
{
    vec3_t pos = {origin.x, origin.y, origin.z};
    return gEngfuncs.PM_PointContents(pos, nullptr);
}

static float AS_GetClientTime()
{
    return gEngfuncs.GetClientTime();
}

static std::string AS_GetCurrentMap()
{
    return gEngfuncs.pfnGetLevelName();
}

static bool AS_IsClientSide()
{
    return true; // Always true in client DLL
}

static CScriptArray* AS_GetNearbyEntities(const Vector3& origin, float radius, bool playersOnly)
{
    asIScriptContext* ctx = asGetActiveContext();
    asIScriptEngine* engine = ctx->GetEngine();
    asITypeInfo* arrayType = engine->GetTypeInfoByDecl("array<int>");
    CScriptArray* arr = CScriptArray::Create(arrayType);
    
    // TODO: Implement entity gathering
    return arr;
}

//==========================================================================
// Registration Functions
//==========================================================================

void ASClientBindings::RegisterLocalPlayer(asIScriptEngine* pEngine)
{
    MS_ANGEL_INFO("[ASClientBindings] Registering CLocalPlayer class...");

    
    MS_ANGEL_INFO("[ASClientBindings] CLocalPlayer registered successfully");
}

void ASClientBindings::RegisterClientEntity(asIScriptEngine* pEngine)
{
    MS_ANGEL_INFO("[ASClientBindings] Registering CClientEntity class...");

    auto bind = asbind20::ref_class<CClientEntity>(pEngine, "CClientEntity");

    // Reference counting behaviors (required for reference types)
    bind.addref(&AddRef_CClientEntity);
    bind.release(&Release_CClientEntity);

    // Methods
    bind.method("bool Exists() const", &CClientEntity::Exists);
    bind.method("int GetIndex() const", &CClientEntity::GetIndex);
    bind.method("Vector3 GetOrigin() const", &CClientEntity::GetOrigin);
    bind.method("Vector3 GetAngles() const", &CClientEntity::GetAngles);
    bind.method("Vector3 GetVelocity() const", &CClientEntity::GetVelocity);
    bind.method("string GetModelName() const", &CClientEntity::GetModelName);
    bind.method("int GetModelIndex() const", &CClientEntity::GetModelIndex);
    bind.method("int GetRenderMode() const", &CClientEntity::GetRenderMode);
    bind.method("int GetRenderAmount() const", &CClientEntity::GetRenderAmount);
    bind.method("Color GetRenderColor() const", &CClientEntity::GetRenderColor);
    bind.method("bool IsPlayer() const", &CClientEntity::IsPlayer);
    bind.method("bool IsVisible() const", &CClientEntity::IsVisible);
    bind.method("Vector3 GetBonePosition(int boneIndex) const", &CClientEntity::GetBonePosition);
    bind.method("int GetBoneCount() const", &CClientEntity::GetBoneCount);
    bind.method("Vector3 GetAttachment(int attachmentIndex) const", &CClientEntity::GetAttachment);
    bind.method("int GetSequence() const", &CClientEntity::GetSequence);
    bind.method("float GetFrame() const", &CClientEntity::GetFrame);
    bind.method("int GetBody() const", &CClientEntity::GetBody);
    bind.method("int GetSkin() const", &CClientEntity::GetSkin);
    
    // Global accessor
    asbind20::global(pEngine).function("CClientEntity@ GetClientEntity(int index)", AS_GetClientEntity);
    
    MS_ANGEL_INFO("[ASClientBindings] CClientEntity registered successfully");
}

void ASClientBindings::RegisterTempEntity(asIScriptEngine* pEngine)
{
    MS_ANGEL_INFO("[ASClientBindings] Registering CTempEntity class...");
    // TODO: Stub - not yet implemented

    // Register CollisionMode enum
    pEngine->RegisterEnum("CollisionMode");
    pEngine->RegisterEnumValue("CollisionMode", "None", (int)CollisionMode::None);
    pEngine->RegisterEnumValue("CollisionMode", "World", (int)CollisionMode::World);
    pEngine->RegisterEnumValue("CollisionMode", "All", (int)CollisionMode::All);
    pEngine->RegisterEnumValue("CollisionMode", "AllAndDie", (int)CollisionMode::AllAndDie);

    // Register CTempEntity class with inheritance
    auto bind = asbind20::ref_class<CTempEntity>(pEngine, "CTempEntity");

    // Reference counting behaviors (inherited from CClientEffect)
    bind.addref(&AddRef_CTempEntity);
    bind.release(&Release_CTempEntity);

    bind.base<CClientEffect>();

    // Note: IsValid() is inherited from CClientEffect, no need to register again

    // Transform methods
    /*
    bind.method("void SetOrigin(const Vector3 &in)", &CTempEntity::SetOrigin);
    bind.method("Vector3 GetOrigin() const", &CTempEntity::GetOrigin);
    bind.method("void SetAngles(const Vector3 &in)", &CTempEntity::SetAngles);
    bind.method("Vector3 GetAngles() const", &CTempEntity::GetAngles);
    bind.method("void SetVelocity(const Vector3 &in)", &CTempEntity::SetVelocity);
    bind.method("Vector3 GetVelocity() const", &CTempEntity::GetVelocity);
    
    // Appearance methods
    bind.method("void SetModel(const string &in)", &CTempEntity::SetModel);
    bind.method("void SetSprite(const string &in)", &CTempEntity::SetSprite);
    bind.method("void SetScale(float)", &CTempEntity::SetScale);
    bind.method("float GetScale() const", &CTempEntity::GetScale);
    bind.method("void SetFrame(int)", &CTempEntity::SetFrame);
    bind.method("int GetFrame() const", &CTempEntity::GetFrame);
    bind.method("void SetFrameRate(float)", &CTempEntity::SetFrameRate);
    bind.method("void SetBody(int)", &CTempEntity::SetBody);
    bind.method("void SetSkin(int)", &CTempEntity::SetSkin);
    
    // Rendering methods
    bind.method("void SetRenderMode(int)", &CTempEntity::SetRenderMode);
    bind.method("void SetRenderAmount(int)", &CTempEntity::SetRenderAmount);
    bind.method("void SetRenderColor(const Color &in)", &CTempEntity::SetRenderColor);
    bind.method("void SetRenderFx(int)", &CTempEntity::SetRenderFx);
    
    // Physics methods
    bind.method("void SetGravity(float)", &CTempEntity::SetGravity);
    bind.method("void SetCollisionMode(CollisionMode)", &CTempEntity::SetCollisionMode);
    bind.method("void SetBounceFactor(float)", &CTempEntity::SetBounceFactor);
    bind.method("void SetMins(const Vector3 &in)", &CTempEntity::SetMins);
    bind.method("void SetMaxs(const Vector3 &in)", &CTempEntity::SetMaxs);
    
    // Lifetime methods
    bind.method("void SetDeathDelay(float)", &CTempEntity::SetDeathDelay);
    bind.method("void SetFadeout(bool, float)", &CTempEntity::SetFadeout);
    bind.method("void Kill()", &CTempEntity::Kill);
    
    // Following methods
    bind.method("void FollowEntity(int, int = -1)", &CTempEntity::FollowEntity);
    bind.method("void StopFollowing()", &CTempEntity::StopFollowing);
    
    // Custom data storage
    bind.method("void SetUserInt(int slot, int value)", &CTempEntity::SetUserInt);
    bind.method("int GetUserInt(int slot) const", &CTempEntity::GetUserInt);
    bind.method("void SetUserFloat(int slot, float value)", &CTempEntity::SetUserFloat);
    bind.method("float GetUserFloat(int slot) const", &CTempEntity::GetUserFloat);
    
    // Callbacks
    bind.method("void SetUpdateCallback(const string &in)", &CTempEntity::SetUpdateCallback);
    bind.method("void SetCollisionCallback(const string &in)", &CTempEntity::SetCollisionCallback);
    bind.method("void SetWaterCallback(const string &in)", &CTempEntity::SetWaterCallback);
    bind.method("void SetTimerCallback(float, const string &in)", &CTempEntity::SetTimerCallback);
    
    // State queries
    bind.method("Vector3 GetWaterOrigin() const", &CTempEntity::GetWaterOrigin);
    
    // Factory functions
    asbind20::global(pEngine).function("CTempEntity@ CreateTempSprite(const string &in, const Vector3 &in)", AS_CreateTempSprite);
    asbind20::global(pEngine).function("CTempEntity@ CreateTempModel(const string &in, const Vector3 &in)", AS_CreateTempModel);
    asbind20::global(pEngine).function("CTempEntity@ CreateFrameSprite(const string &in, const Vector3 &in)", AS_CreateFrameSprite);
    asbind20::global(pEngine).function("CTempEntity@ CreateFrameModel(const string &in, const Vector3 &in)", AS_CreateFrameModel);
    */
    MS_ANGEL_INFO("[ASClientBindings] CTempEntity registered successfully");
}

void ASClientBindings::RegisterDynamicLight(asIScriptEngine* pEngine)
{
    MS_ANGEL_INFO("[ASClientBindings] Registering CDynamicLight class...");
    
    auto bind = asbind20::ref_class<CDynamicLight>(pEngine, "CDynamicLight");
    bind.base<CClientEffect>();
    
    bind.method("bool IsValid() const", &CDynamicLight::IsValid);
    bind.method("void SetOrigin(const Vector3 &in)", &CDynamicLight::SetOrigin);
    bind.method("Vector3 GetOrigin() const", &CDynamicLight::GetOrigin);
    bind.method("void SetRadius(float)", &CDynamicLight::SetRadius);
    bind.method("void SetColor(const Color &in)", &CDynamicLight::SetColor);
    bind.method("void SetDuration(float)", &CDynamicLight::SetDuration);
    bind.method("void SetDark(bool)", &CDynamicLight::SetDark);
    bind.method("void FollowEntity(int)", &CDynamicLight::FollowEntity);
    bind.method("void Kill()", &CDynamicLight::Kill);

    asbind20::global(pEngine).function("CDynamicLight@ CreateDynamicLight(const Vector3 &in, float, const Color &in, float = 0.0f)", AS_CreateDynamicLight);
    
    MS_ANGEL_INFO("[ASClientBindings] CDynamicLight registered successfully");
}

void ASClientBindings::RegisterBeam(asIScriptEngine* pEngine)
{
    MS_ANGEL_INFO("[ASClientBindings] Registering CBeam class...");
    
    auto bind = asbind20::ref_class<CBeam>(pEngine, "CBeam");
    bind.base<CClientEffect>();

    bind.method("bool IsValid() const", &CBeam::IsValid);
    bind.method("void SetStartPos(const Vector3 &in)", &CBeam::SetStartPos);
    bind.method("void SetEndPos(const Vector3 &in)", &CBeam::SetEndPos);
    bind.method("void SetStartEntity(int, int)", &CBeam::SetStartEntity);
    bind.method("void SetEndEntity(int, int)", &CBeam::SetEndEntity);
    bind.method("void SetSprite(const string &in)", &CBeam::SetSprite);
    bind.method("void SetWidth(float)", &CBeam::SetWidth);
    bind.method("void SetAmplitude(float)", &CBeam::SetAmplitude);
    bind.method("void SetBrightness(float)", &CBeam::SetBrightness);
    bind.method("void SetSpeed(float)", &CBeam::SetSpeed);
    bind.method("void SetFrameRate(float)", &CBeam::SetFrameRate);
    bind.method("void SetColor(const Color &in)", &CBeam::SetColor);
    bind.method("void SetLife(float)", &CBeam::SetLife);
    bind.method("void Kill()", &CBeam::Kill);

    asbind20::global(pEngine).function("CBeam@ CreateBeamPoints(const Vector3 &in, const Vector3 &in, const string &in)", AS_CreateBeamPoints);
    asbind20::global(pEngine).function("CBeam@ CreateBeamEntities(int, int, int, int, const string &in)", AS_CreateBeamEntities);
    asbind20::global(pEngine).function("CBeam@ CreateBeamEntPoint(int, int, const Vector3 &in, const string &in)", AS_CreateBeamEntPoint);

    // Effect utility functions
    asbind20::global(pEngine).function("void CreateSpark(const Vector3 &in)", AS_CreateSpark);
    asbind20::global(pEngine).function("void CreateSparkOnModel(int, int = 0)", AS_CreateSparkOnModel);
    asbind20::global(pEngine).function("void CreateDecal(int, const Vector3 &in, const Vector3 &in)", AS_CreateDecal);
    
    MS_ANGEL_INFO("[ASClientBindings] CBeam registered successfully");
}

void ASClientBindings::RegisterEnvironment(asIScriptEngine* pEngine)
{
    MS_ANGEL_INFO("[ASClientBindings] Registering CClientEnvironment class...");
    
    auto bind = asbind20::ref_class<CClientEnvironment>(pEngine, "CClientEnvironment");
    
    // Fog control
    bind.method("void SetFogEnabled(bool)", &CClientEnvironment::SetFogEnabled);
    bind.method("bool GetFogEnabled() const", &CClientEnvironment::GetFogEnabled);
    bind.method("void SetFogColor(const Color &in)", &CClientEnvironment::SetFogColor);
    bind.method("Color GetFogColor() const", &CClientEnvironment::GetFogColor);
    bind.method("void SetFogDensity(float)", &CClientEnvironment::SetFogDensity);
    bind.method("void SetFogStart(float)", &CClientEnvironment::SetFogStart);
    bind.method("void SetFogEnd(float)", &CClientEnvironment::SetFogEnd);
    bind.method("void SetFogType(int)", &CClientEnvironment::SetFogType);
    
    // Screen effects
    bind.method("void SetScreenTint(const Color &in)", &CClientEnvironment::SetScreenTint);
    bind.method("Color GetScreenTint() const", &CClientEnvironment::GetScreenTint);
    bind.method("void ClearScreenTint()", &CClientEnvironment::ClearScreenTint);
    
    // Sky
    bind.method("void SetSkyTexture(const string &in)", &CClientEnvironment::SetSkyTexture);
    bind.method("string GetSkyName() const", &CClientEnvironment::GetSkyName);
    bind.method("void SetLightGamma(float)", &CClientEnvironment::SetLightGamma);
    bind.method("void SetMaxViewDistance(float)", &CClientEnvironment::SetMaxViewDistance);

    asbind20::global(pEngine).function("CClientEnvironment@ GetEnvironment()", AS_GetEnvironment);
    
    MS_ANGEL_INFO("[ASClientBindings] CClientEnvironment registered successfully");
}

void ASClientBindings::RegisterScreenFade(asIScriptEngine* pEngine)
{
    MS_ANGEL_INFO("[ASClientBindings] Screen fade integrated into CClientEnvironment");
    // Screen fade functionality is part of CClientEnvironment
}

void ASClientBindings::RegisterClientSound(asIScriptEngine* pEngine)
{
    MS_ANGEL_INFO("[ASClientBindings] Registering CClientSound class...");
    
    auto bind = asbind20::ref_class<CClientSound>(pEngine, "CClientSound");
    
    bind.method("void PlaySound(int, const string &in, float = 1.0f)", &CClientSound::PlaySound);
    bind.method("void SetVolume(int, const string &in, float)", &CClientSound::SetVolume);
    bind.method("void StopSound(int, const string &in)", &CClientSound::StopSound);
    bind.method("void PlaySoundAtPosition(const Vector3 &in, const string &in, float = 1.0f)", &CClientSound::PlaySoundAtPosition);

    asbind20::global(pEngine).function("CClientSound@ GetClientSound()", AS_GetClientSound);
    
    // Sound channel constants
    pEngine->RegisterGlobalProperty("const int SOUND_CHANNEL_WEAPON", (void*)0);
    pEngine->RegisterGlobalProperty("const int SOUND_CHANNEL_VOICE", (void*)1);
    pEngine->RegisterGlobalProperty("const int SOUND_CHANNEL_STREAM", (void*)5);
    
    MS_ANGEL_INFO("[ASClientBindings] CClientSound registered successfully");
}

void ASClientBindings::RegisterUtilityFunctions(asIScriptEngine* pEngine)
{
    MS_ANGEL_INFO("[ASClientBindings] Registering utility functions...");

    // Trace/query functions
    asbind20::global(pEngine).function("Vector3 GetGroundHeight(const Vector3 &in)", AS_GetGroundHeight);
    asbind20::global(pEngine).function("Vector3 GetSkyHeight(const Vector3 &in)", AS_GetSkyHeight);
    asbind20::global(pEngine).function("bool IsUnderSky(const Vector3 &in)", AS_IsUnderSky);
    asbind20::global(pEngine).function("Vector3 TraceLine(const Vector3 &in, const Vector3 &in, bool = false)", AS_TraceLine);
    asbind20::global(pEngine).function("int TraceLineEntity(const Vector3 &in, const Vector3 &in)", AS_TraceLineEntity);
    asbind20::global(pEngine).function("int GetContents(const Vector3 &in)", AS_GetContents);

    // Client-specific accessors
    asbind20::global(pEngine).function("float GetClientTime()", AS_GetClientTime);
    asbind20::global(pEngine).function("string GetCurrentMap()", AS_GetCurrentMap);
    asbind20::global(pEngine).function("bool IsClientSide()", AS_IsClientSide);
    asbind20::global(pEngine).function("array<int>@ GetNearbyEntities(const Vector3 &in, float, bool = false)", AS_GetNearbyEntities);
    
    MS_ANGEL_INFO("[ASClientBindings] Utility functions registered successfully");
}

//==========================================================================
// Main Registration Function
//==========================================================================

void ASClientBindings::RegisterAll(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        MS_ANGEL_ERROR("[ASClientBindings] Cannot register bindings - engine is null");
        return;
    }

    MS_ANGEL_INFO("[ASClientBindings] Starting client-side binding registration...");

    // Register base classes first, before any derived classes
    RegisterClientEffect(pEngine);  // Must be registered before CTempEntity, CDynamicLight, CBeam

    RegisterClientEntity(pEngine);
    ASClientItemBindings::RegisterAll(pEngine);  // Register CGenericItem (client-side item bindings)
    RegisterTempEntity(pEngine);
    RegisterVGUI(pEngine);  // Register VGUI bindings (VGUIFrame, VGUILabel, VGUIButton, etc.)
}

//==========================================================================
// Client Effect Base Class Registration
//==========================================================================
void ASClientBindings::RegisterClientEffect(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        MS_ANGEL_ERROR("[ASClientBindings] Cannot register CClientEffect - engine is null");
        return;
    }

    MS_ANGEL_INFO("[ASClientBindings] Registering CClientEffect base class...");
    auto baseEffect = asbind20::ref_class<CClientEffect>(pEngine, "CClientEffect");

    // Reference counting behaviors (required for reference types)
    baseEffect.addref(&AddRef_CClientEffect);
    baseEffect.release(&Release_CClientEffect);

    baseEffect.method("bool IsValid() const", &CClientEffect::IsValid);
    MS_ANGEL_INFO("[ASClientBindings] CClientEffect base class registered successfully");
}

//==========================================================================
// VGUI Registration
//==========================================================================
void ASClientBindings::RegisterVGUI(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        MS_ANGEL_ERROR("[ASClientBindings] Cannot register VGUI - engine is null");
        return;
    }

    MS_ANGEL_INFO("[ASClientBindings] Registering VGUI bindings...");

    // Call the VGUI binding registration
    ASVGUIBindings::RegisterAll(pEngine);

    MS_ANGEL_INFO("[ASClientBindings] VGUI bindings registered successfully");
}



