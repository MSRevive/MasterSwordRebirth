//==========================================================================
// ASItemBindings.cpp - asbind20 Implementation
// 
// Item and weapon type bindings for AngelScript integration
// Provides CBaseAnimating, CBasePlayerItem, CBasePlayerWeapon for script usage
// Uses asbind20 for clean, type-safe binding syntax
//==========================================================================

// Include standard library headers first
#include <cstdio>
#include <new>
#include <vector>
#include <string>
#include <cmath>
#include <type_traits>
#include <angelscript.h>
#include "addons/scriptarray/scriptarray.h"

// Include asbind20 for modern binding syntax
#include <asbind20/asbind.hpp>

// Include vector.h directly for Vector type
typedef float vec_t;
#include "hl/vector.h"

// Include MSLogger with proper path
#include "mslogger.h"

// Include Master Sword entity classes
#ifdef CLIENT_DLL
    // Client-side includes
    #include "hud.h"
    #include "cl_util.h"
#else
    // Server-side includes (VALVE_DLL is defined for server builds)
    #include "extdll.h"
    #include "util.h"
    #include "hl/cbase.h"
    #include "player/player.h"
    #include "weapons/weapons.h"
    #include "monsters/msmonster.h"
#endif

// Include AngelScript headers after engine headers
#include "ASEngineInterface.h"
#include "ASCoreTypes.h"
#include "ASEntityBindings.h"

// Engine globals - declared globally to avoid namespace issues
#ifndef CLIENT_DLL
extern globalvars_t *gpGlobals;
extern enginefuncs_t g_engfuncs;
#endif

namespace ASItemBindings
{
    // ========================================================================
    // Reference Counting Functions for AngelScript Reference Types
    // Note: These are no-ops since entities are managed by the game engine
    // ========================================================================
    
    void AddRef_CBaseAnimating(CBaseAnimating* entity)
    {
        // No-op: Engine manages entity lifetime
        MS_ANGEL_DEBUG("AddRef_CBaseAnimating called for entity %p", entity);
    }
    
    void Release_CBaseAnimating(CBaseAnimating* entity) 
    {
        // No-op: Engine manages entity lifetime
        MS_ANGEL_DEBUG("Release_CBaseAnimating called for entity %p", entity);
    }
    
    void AddRef_CBasePlayerItem(CBasePlayerItem* item)
    {
        // No-op: Engine manages entity lifetime
        MS_ANGEL_DEBUG("AddRef_CBasePlayerItem called for item %p", item);
    }
    
    void Release_CBasePlayerItem(CBasePlayerItem* item)
    {
        // No-op: Engine manages entity lifetime
        MS_ANGEL_DEBUG("Release_CBasePlayerItem called for item %p", item);
    }
    
    void AddRef_CBasePlayerWeapon(CBasePlayerWeapon* weapon)
    {
        // No-op: Engine manages entity lifetime
        MS_ANGEL_DEBUG("AddRef_CBasePlayerWeapon called for weapon %p", weapon);
    }
    
    void Release_CBasePlayerWeapon(CBasePlayerWeapon* weapon)
    {
        // No-op: Engine manages entity lifetime
        MS_ANGEL_DEBUG("Release_CBasePlayerWeapon called for weapon %p", weapon);
    }
    
    // ========================================================================
    // CBaseAnimating Helper Functions
    // ========================================================================
    
    // Wrapper functions for CBaseAnimating methods
    int AS_CBaseAnimating_LookupSequence(CBaseAnimating* pAnimating, const std::string& label)
    {
        #ifdef VALVE_DLL
            if (!pAnimating || FNullEnt(pAnimating))
            {
                MS_ANGEL_ERROR("LookupSequence: NULL animating entity pointer");
                return -1;
            }
            return pAnimating->LookupSequence(label.c_str());
        #else
            MS_ANGEL_DEBUG("LookupSequence: Client-side access limited");
            return -1;
        #endif
    }
    
    void AS_CBaseAnimating_ResetSequenceInfo(CBaseAnimating* pAnimating)
    {
        #ifdef VALVE_DLL
            if (!pAnimating || FNullEnt(pAnimating))
            {
                MS_ANGEL_ERROR("ResetSequenceInfo: NULL animating entity pointer");
                return;
            }
            pAnimating->ResetSequenceInfo();
        #else
            MS_ANGEL_DEBUG("ResetSequenceInfo: Client-side modification not allowed");
        #endif
    }
    
    void AS_CBaseAnimating_SetBodygroup(CBaseAnimating* pAnimating, int group, int value)
    {
        #ifdef VALVE_DLL
            if (!pAnimating || FNullEnt(pAnimating))
            {
                MS_ANGEL_ERROR("SetBodygroup: NULL animating entity pointer");
                return;
            }
            pAnimating->SetBodygroup(group, value);
        #else
            MS_ANGEL_DEBUG("SetBodygroup: Client-side modification not allowed");
        #endif
    }
    
    int AS_CBaseAnimating_GetBodygroup(CBaseAnimating* pAnimating, int group)
    {
        #ifdef VALVE_DLL
            if (!pAnimating || FNullEnt(pAnimating))
            {
                MS_ANGEL_ERROR("GetBodygroup: NULL animating entity pointer");
                return 0;
            }
            return pAnimating->GetBodygroup(group);
        #else
            MS_ANGEL_DEBUG("GetBodygroup: Client-side access limited");
            return 0;
        #endif
    }
    
    float AS_CBaseAnimating_GetFrameRate(CBaseAnimating* pAnimating)
    {
        #ifdef VALVE_DLL
            if (!pAnimating || FNullEnt(pAnimating))
            {
                MS_ANGEL_ERROR("GetFrameRate: NULL animating entity pointer");
                return 0.0f;
            }
            return pAnimating->m_flFrameRate;
        #else
            MS_ANGEL_DEBUG("GetFrameRate: Client-side access limited");
            return 0.0f;
        #endif
    }
    
    bool AS_CBaseAnimating_IsSequenceFinished(CBaseAnimating* pAnimating)
    {
        #ifdef VALVE_DLL
            if (!pAnimating || FNullEnt(pAnimating))
            {
                MS_ANGEL_ERROR("IsSequenceFinished: NULL animating entity pointer");
                return true;
            }
            return pAnimating->m_fSequenceFinished ? true : false;
        #else
            MS_ANGEL_DEBUG("IsSequenceFinished: Client-side access limited");
            return true;
        #endif
    }
    
    bool AS_CBaseAnimating_IsSequenceLooping(CBaseAnimating* pAnimating)
    {
        #ifdef VALVE_DLL
            if (!pAnimating || FNullEnt(pAnimating))
            {
                MS_ANGEL_ERROR("IsSequenceLooping: NULL animating entity pointer");
                return false;
            }
            return pAnimating->m_fSequenceLoops ? true : false;
        #else
            MS_ANGEL_DEBUG("IsSequenceLooping: Client-side access limited");
            return false;
        #endif
    }
    
    // ========================================================================
    // CBasePlayerItem Helper Functions
    // ========================================================================
    
    std::string AS_CBasePlayerItem_GetItemName(CBasePlayerItem* pItem)
    {
        #ifdef VALVE_DLL
            if (!pItem || FNullEnt(pItem))
            {
                MS_ANGEL_ERROR("GetItemName: NULL item pointer");
                return "";
            }
            return pItem->ItemName.c_str();
        #else
            MS_ANGEL_DEBUG("GetItemName: Client-side access limited");
            return "";
        #endif
    }
    
    std::string AS_CBasePlayerItem_GetWorldModel(CBasePlayerItem* pItem)
    {
        #ifdef VALVE_DLL
            if (!pItem || FNullEnt(pItem))
            {
                MS_ANGEL_ERROR("GetWorldModel: NULL item pointer");
                return "";
            }
            return pItem->WorldModel.c_str();
        #else
            MS_ANGEL_DEBUG("GetWorldModel: Client-side access limited");
            return "";
        #endif
    }
    
    std::string AS_CBasePlayerItem_GetHandSpriteName(CBasePlayerItem* pItem)
    {
        #ifdef VALVE_DLL
            if (!pItem || FNullEnt(pItem))
            {
                MS_ANGEL_ERROR("GetHandSpriteName: NULL item pointer");
                return "";
            }
            return pItem->HandSpriteName.c_str();
        #else
            MS_ANGEL_DEBUG("GetHandSpriteName: Client-side access limited");
            return "";
        #endif
    }
    
    std::string AS_CBasePlayerItem_GetTradeSpriteName(CBasePlayerItem* pItem)
    {
        #ifdef VALVE_DLL
            if (!pItem || FNullEnt(pItem))
            {
                MS_ANGEL_ERROR("GetTradeSpriteName: NULL item pointer");
                return "";
            }
            return pItem->TradeSpriteName.c_str();
        #else
            MS_ANGEL_DEBUG("GetTradeSpriteName: Client-side access limited");
            return "";
        #endif
    }
    
    // GetOwnerPlayer is registered separately in RegisterCrossReferences() 
    // after CBasePlayer is registered to avoid circular dependency
    CBasePlayer* AS_CBasePlayerItem_GetOwnerPlayer(CBasePlayerItem* pItem)
    {
        #ifdef VALVE_DLL
            if (!pItem || FNullEnt(pItem))
            {
                MS_ANGEL_ERROR("GetOwnerPlayer: NULL item pointer");
                return nullptr;
            }
            return pItem->m_pPlayer;
        #else
            MS_ANGEL_DEBUG("GetOwnerPlayer: Client-side access limited");
            return nullptr;
        #endif
    }
    
    CMSMonster* AS_CBasePlayerItem_GetOwnerMonster(CBasePlayerItem* pItem)
    {
        #ifdef VALVE_DLL
            if (!pItem || FNullEnt(pItem))
            {
                MS_ANGEL_ERROR("GetOwnerMonster: NULL item pointer");
                return nullptr;
            }
            return pItem->m_pOwner;
        #else
            MS_ANGEL_DEBUG("GetOwnerMonster: Client-side access limited");
            return nullptr;
        #endif
    }
    
    int AS_CBasePlayerItem_GetItemID(CBasePlayerItem* pItem)
    {
        #ifdef VALVE_DLL
            if (!pItem || FNullEnt(pItem))
            {
                MS_ANGEL_ERROR("GetItemID: NULL item pointer");
                return 0;
            }
            return pItem->m_iId;
        #else
            MS_ANGEL_DEBUG("GetItemID: Client-side access limited");
            return 0;
        #endif
    }
    
    unsigned int AS_CBasePlayerItem_GetValue(CBasePlayerItem* pItem)
    {
        #ifdef VALVE_DLL
            if (!pItem || FNullEnt(pItem))
            {
                MS_ANGEL_ERROR("GetValue: NULL item pointer");
                return 0;
            }
            return pItem->m_Value;
        #else
            MS_ANGEL_DEBUG("GetValue: Client-side access limited");
            return 0;
        #endif
    }
    
    bool AS_CBasePlayerItem_IsWielded(CBasePlayerItem* pItem)
    {
        #ifdef VALVE_DLL
            if (!pItem || FNullEnt(pItem))
            {
                MS_ANGEL_ERROR("IsWielded: NULL item pointer");
                return false;
            }
            return pItem->Wielded ? true : false;
        #else
            MS_ANGEL_DEBUG("IsWielded: Client-side access limited");
            return false;
        #endif
    }
    
    bool AS_CBasePlayerItem_IsUseable(CBasePlayerItem* pItem)
    {
        #ifdef VALVE_DLL
            if (!pItem || FNullEnt(pItem))
            {
                MS_ANGEL_ERROR("IsUseable: NULL item pointer");
                return false;
            }
            return !pItem->m_NotUseable;
        #else
            MS_ANGEL_DEBUG("IsUseable: Client-side access limited");
            return false;
        #endif
    }
    
    // ========================================================================
    // CBasePlayerWeapon Helper Functions
    // ========================================================================
    
    int AS_CBasePlayerWeapon_GetClip(CBasePlayerWeapon* pWeapon)
    {
        #ifdef VALVE_DLL
            if (!pWeapon || FNullEnt(pWeapon))
            {
                MS_ANGEL_ERROR("GetClip: NULL weapon pointer");
                return 0;
            }
            return pWeapon->m_iClip;
        #else
            MS_ANGEL_DEBUG("GetClip: Client-side access limited");
            return 0;
        #endif
    }
    
    void AS_CBasePlayerWeapon_SetClip(CBasePlayerWeapon* pWeapon, int amount)
    {
        #ifdef VALVE_DLL
            if (!pWeapon || FNullEnt(pWeapon))
            {
                MS_ANGEL_ERROR("SetClip: NULL weapon pointer");
                return;
            }
            pWeapon->m_iClip = amount;
            MS_ANGEL_DEBUG("SetClip: Weapon clip set to %d", amount);
        #else
            MS_ANGEL_DEBUG("SetClip: Client-side modification not allowed");
        #endif
    }
    
    int AS_CBasePlayerWeapon_GetPrimaryAmmoType(CBasePlayerWeapon* pWeapon)
    {
        #ifdef VALVE_DLL
            if (!pWeapon || FNullEnt(pWeapon))
            {
                MS_ANGEL_ERROR("GetPrimaryAmmoType: NULL weapon pointer");
                return -1;
            }
            return pWeapon->m_iPrimaryAmmoType;
        #else
            MS_ANGEL_DEBUG("GetPrimaryAmmoType: Client-side access limited");
            return -1;
        #endif
    }
    
    int AS_CBasePlayerWeapon_GetSecondaryAmmoType(CBasePlayerWeapon* pWeapon)
    {
        #ifdef VALVE_DLL
            if (!pWeapon || FNullEnt(pWeapon))
            {
                MS_ANGEL_ERROR("GetSecondaryAmmoType: NULL weapon pointer");
                return -1;
            }
            return pWeapon->m_iSecondaryAmmoType;
        #else
            MS_ANGEL_DEBUG("GetSecondaryAmmoType: Client-side access limited");
            return -1;
        #endif
    }
    
    float AS_CBasePlayerWeapon_GetNextPrimaryAttack(CBasePlayerWeapon* pWeapon)
    {
        #ifdef VALVE_DLL
            if (!pWeapon || FNullEnt(pWeapon))
            {
                MS_ANGEL_ERROR("GetNextPrimaryAttack: NULL weapon pointer");
                return 0.0f;
            }
            return pWeapon->m_flNextPrimaryAttack;
        #else
            MS_ANGEL_DEBUG("GetNextPrimaryAttack: Client-side access limited");
            return 0.0f;
        #endif
    }
    
    void AS_CBasePlayerWeapon_SetNextPrimaryAttack(CBasePlayerWeapon* pWeapon, float time)
    {
        #ifdef VALVE_DLL
            if (!pWeapon || FNullEnt(pWeapon))
            {
                MS_ANGEL_ERROR("SetNextPrimaryAttack: NULL weapon pointer");
                return;
            }
            pWeapon->m_flNextPrimaryAttack = time;
            MS_ANGEL_DEBUG("SetNextPrimaryAttack: Weapon next primary attack set to %f", time);
        #else
            MS_ANGEL_DEBUG("SetNextPrimaryAttack: Client-side modification not allowed");
        #endif
    }
    
    float AS_CBasePlayerWeapon_GetNextSecondaryAttack(CBasePlayerWeapon* pWeapon)
    {
        #ifdef VALVE_DLL
            if (!pWeapon || FNullEnt(pWeapon))
            {
                MS_ANGEL_ERROR("GetNextSecondaryAttack: NULL weapon pointer");
                return 0.0f;
            }
            return pWeapon->m_flNextSecondaryAttack;
        #else
            MS_ANGEL_DEBUG("GetNextSecondaryAttack: Client-side access limited");
            return 0.0f;
        #endif
    }
    
    void AS_CBasePlayerWeapon_SetNextSecondaryAttack(CBasePlayerWeapon* pWeapon, float time)
    {
        #ifdef VALVE_DLL
            if (!pWeapon || FNullEnt(pWeapon))
            {
                MS_ANGEL_ERROR("SetNextSecondaryAttack: NULL weapon pointer");
                return;
            }
            pWeapon->m_flNextSecondaryAttack = time;
            MS_ANGEL_DEBUG("SetNextSecondaryAttack: Weapon next secondary attack set to %f", time);
        #else
            MS_ANGEL_DEBUG("SetNextSecondaryAttack: Client-side modification not allowed");
        #endif
    }
    
    float AS_CBasePlayerWeapon_GetTimeWeaponIdle(CBasePlayerWeapon* pWeapon)
    {
        #ifdef VALVE_DLL
            if (!pWeapon || FNullEnt(pWeapon))
            {
                MS_ANGEL_ERROR("GetTimeWeaponIdle: NULL weapon pointer");
                return 0.0f;
            }
            return pWeapon->m_flTimeWeaponIdle;
        #else
            MS_ANGEL_DEBUG("GetTimeWeaponIdle: Client-side access limited");
            return 0.0f;
        #endif
    }
    
    bool AS_CBasePlayerWeapon_IsInReload(CBasePlayerWeapon* pWeapon)
    {
        #ifdef VALVE_DLL
            if (!pWeapon || FNullEnt(pWeapon))
            {
                MS_ANGEL_ERROR("IsInReload: NULL weapon pointer");
                return false;
            }
            return pWeapon->m_fInReload != 0;
        #else
            MS_ANGEL_DEBUG("IsInReload: Client-side access limited");
            return false;
        #endif
    }
    
    std::string AS_CBasePlayerWeapon_pszAmmo1(CBasePlayerWeapon* pWeapon)
    {
        #ifdef VALVE_DLL
            if (!pWeapon || FNullEnt(pWeapon))
            {
                MS_ANGEL_ERROR("pszAmmo1: NULL weapon pointer");
                return "";
            }
            const char* ammo = pWeapon->pszAmmo1();
            return ammo ? std::string(ammo) : "";
        #else
            MS_ANGEL_DEBUG("pszAmmo1: Client-side access limited");
            return "";
        #endif
    }
    
    std::string AS_CBasePlayerWeapon_pszAmmo2(CBasePlayerWeapon* pWeapon)
    {
        #ifdef VALVE_DLL
            if (!pWeapon || FNullEnt(pWeapon))
            {
                MS_ANGEL_ERROR("pszAmmo2: NULL weapon pointer");
                return "";
            }
            const char* ammo = pWeapon->pszAmmo2();
            return ammo ? std::string(ammo) : "";
        #else
            MS_ANGEL_DEBUG("pszAmmo2: Client-side access limited");
            return "";
        #endif
    }
    
    // Note: Player inventory helper functions are implemented as methods
    // on CBasePlayer in ASEntityBindings.cpp for better OOP design
    
    // ========================================================================
    // Type Registration Functions
    // ========================================================================
    
    void RegisterCBaseAnimating(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASItemBindings] Registering CBaseAnimating type with asbind20...");
        
        // Register CBaseAnimating type with asbind20
        asbind20::ref_class<CBaseAnimating>(pEngine, "CBaseAnimating")
            // Reference counting behaviors (required for reference types)
            .addref(&AddRef_CBaseAnimating)
            .release(&Release_CBaseAnimating)
            // Animation methods
            .method("int LookupSequence(const string &in)", AS_CBaseAnimating_LookupSequence)
            .method("void ResetSequenceInfo()", AS_CBaseAnimating_ResetSequenceInfo)
            .method("void SetBodygroup(int, int)", AS_CBaseAnimating_SetBodygroup)
            .method("int GetBodygroup(int)", AS_CBaseAnimating_GetBodygroup)
            // Properties (read-only from script)
            .method("float GetFrameRate() const", AS_CBaseAnimating_GetFrameRate)
            .method("bool IsSequenceFinished() const", AS_CBaseAnimating_IsSequenceFinished)
            .method("bool IsSequenceLooping() const", AS_CBaseAnimating_IsSequenceLooping)
            
            // Inherit from CBaseEntity (called AFTER registering CBaseAnimating's own methods)
            .base<CBaseEntity>();
        
        MS_ANGEL_INFO("CBaseAnimating registration complete");
    }
    
    void RegisterCBasePlayerItem(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASItemBindings] Registering CBasePlayerItem type with asbind20...");
        
        // Register CBasePlayerItem type with asbind20
        // Note: GetOwnerPlayer() is registered later in RegisterCrossReferences() to avoid circular dependency
        asbind20::ref_class<CBasePlayerItem>(pEngine, "CBasePlayerItem")
            // Reference counting behaviors (required for reference types)
            .addref(&AddRef_CBasePlayerItem)
            .release(&Release_CBasePlayerItem)
            // Property accessors
            .method("string GetItemName() const", AS_CBasePlayerItem_GetItemName)
            .method("string GetWorldModel() const", AS_CBasePlayerItem_GetWorldModel)
            .method("string GetHandSpriteName() const", AS_CBasePlayerItem_GetHandSpriteName)
            .method("string GetTradeSpriteName() const", AS_CBasePlayerItem_GetTradeSpriteName)
            // GetOwnerPlayer() registered in RegisterCrossReferences() after CBasePlayer is available
            .method("int GetItemID() const", AS_CBasePlayerItem_GetItemID)
            .method("uint GetValue() const", AS_CBasePlayerItem_GetValue)
            .method("bool IsWielded() const", AS_CBasePlayerItem_IsWielded)
            .method("bool IsUseable() const", AS_CBasePlayerItem_IsUseable)
            // Item methods
            .method("bool IsMSItem()", &CBasePlayerItem::IsMSItem)
            .method("bool CanDrop()", &CBasePlayerItem::CanDrop)
            #ifdef VALVE_DLL
            .method("bool Deploy()", &CBasePlayerItem::Deploy)
            .method("void Holster()", &CBasePlayerItem::Holster)
            .method("void Materialize()", &CBasePlayerItem::Materialize)
            #endif
            
            // Inherit from CBaseAnimating (called AFTER registering CBasePlayerItem's own methods)
            .base<CBaseAnimating>();
        
        MS_ANGEL_INFO("CBasePlayerItem registration complete");
    }
    
    void RegisterCBasePlayerWeapon(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASItemBindings] Registering CBasePlayerWeapon type with asbind20...");
        
        // Register CBasePlayerWeapon type with asbind20
        asbind20::ref_class<CBasePlayerWeapon>(pEngine, "CBasePlayerWeapon")
            // Reference counting behaviors (required for reference types)
            .addref(&AddRef_CBasePlayerWeapon)
            .release(&Release_CBasePlayerWeapon)
            // Ammo properties
            .method("int GetClip() const", AS_CBasePlayerWeapon_GetClip)
            .method("void SetClip(int)", AS_CBasePlayerWeapon_SetClip)
            .method("int GetPrimaryAmmoType() const", AS_CBasePlayerWeapon_GetPrimaryAmmoType)
            .method("int GetSecondaryAmmoType() const", AS_CBasePlayerWeapon_GetSecondaryAmmoType)
            // Attack timing
            .method("float GetNextPrimaryAttack() const", AS_CBasePlayerWeapon_GetNextPrimaryAttack)
            .method("void SetNextPrimaryAttack(float)", AS_CBasePlayerWeapon_SetNextPrimaryAttack)
            .method("float GetNextSecondaryAttack() const", AS_CBasePlayerWeapon_GetNextSecondaryAttack)
            .method("void SetNextSecondaryAttack(float)", AS_CBasePlayerWeapon_SetNextSecondaryAttack)
            .method("float GetTimeWeaponIdle() const", AS_CBasePlayerWeapon_GetTimeWeaponIdle)
            // State properties
            .method("bool IsInReload() const", AS_CBasePlayerWeapon_IsInReload)
            // Weapon methods
            #ifdef VALVE_DLL
            .method("bool CanDeploy()", &CBasePlayerWeapon::CanDeploy)
            .method("bool IsUseable()", &CBasePlayerWeapon::IsUseable)
            .method("void SendWeaponAnim(int, int)", &CBasePlayerWeapon::SendWeaponAnim)
            #endif
            // Ammo info methods
            .method("int PrimaryAmmoIndex()", &CBasePlayerWeapon::PrimaryAmmoIndex)
            .method("int SecondaryAmmoIndex()", &CBasePlayerWeapon::SecondaryAmmoIndex)
            .method("string pszAmmo1()", AS_CBasePlayerWeapon_pszAmmo1)
            .method("string pszAmmo2()", AS_CBasePlayerWeapon_pszAmmo2)
            
            // Inherit from CBasePlayerItem (called AFTER registering CBasePlayerWeapon's own methods)
            .base<CBasePlayerItem>();
        
        MS_ANGEL_INFO("CBasePlayerWeapon registration complete");
    }
    
    void RegisterCrossReferences(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASItemBindings] Registering cross-reference methods...");
        
        // Add methods that reference CBasePlayer (must be called after CBasePlayer is registered)
        int r = pEngine->RegisterObjectMethod("CBasePlayerItem", 
            "CBasePlayer@ GetOwnerPlayer() const", 
            asFUNCTION(AS_CBasePlayerItem_GetOwnerPlayer), asCALL_CDECL_OBJFIRST);
        
        if (r < 0)
        {
            MS_ANGEL_ERROR("Failed to register CBasePlayerItem::GetOwnerPlayer() - Error code: %d", r);
        }
        else
        {
            MS_ANGEL_INFO("   OK CBasePlayerItem::GetOwnerPlayer() registered");
        }
        
        MS_ANGEL_INFO("[ASItemBindings] Cross-reference registration complete");
    }
    
    void RegisterAll(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASItemBindings] Starting item/weapon bindings registration...");
        
        // Register types in inheritance order
        RegisterCBaseAnimating(pEngine);
        RegisterCBasePlayerItem(pEngine);
        RegisterCBasePlayerWeapon(pEngine);
        
        MS_ANGEL_INFO("[ASItemBindings] Item/weapon bindings registration complete");
    }
}

