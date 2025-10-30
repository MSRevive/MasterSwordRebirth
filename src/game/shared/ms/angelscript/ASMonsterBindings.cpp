//==========================================================================
// ASMonsterBindings.cpp - asbind20 Implementation
// 
// CBaseMonster and CMSMonster type bindings for AngelScript integration
// Provides comprehensive monster AI and stats functionality
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
    #include "basemonster.h"
    #include "monsters/msmonster.h"
    #include "player/player.h"
#endif

// Include AngelScript headers after engine headers
#include "ASEngineInterface.h"
#include "ASCoreTypes.h"
#include "ASEntityBindings.h"
#include "ASMonsterBindings.h"

// Engine globals
#ifndef CLIENT_DLL
extern globalvars_t *gpGlobals;
extern enginefuncs_t g_engfuncs;
#endif

namespace ASMonsterBindings
{
    // ========================================================================
    // Reference Counting Functions for AngelScript Reference Types
    // Note: These are no-ops since entities are managed by the game engine
    // ========================================================================
    
    void AddRef_CBaseMonster(CBaseMonster* monster)
    {
        // No-op: Engine manages entity lifetime
        MS_ANGEL_DEBUG("AddRef_CBaseMonster called for monster %p", monster);
    }
    
    void Release_CBaseMonster(CBaseMonster* monster) 
    {
        // No-op: Engine manages entity lifetime
        MS_ANGEL_DEBUG("Release_CBaseMonster called for monster %p", monster);
    }
    
    void AddRef_CMSMonster(CMSMonster* monster)
    {
        // No-op: Engine manages entity lifetime
        MS_ANGEL_DEBUG("AddRef_CMSMonster called for monster %p", monster);
    }
    
    void Release_CMSMonster(CMSMonster* monster)
    {
        // No-op: Engine manages entity lifetime
        MS_ANGEL_DEBUG("Release_CMSMonster called for monster %p", monster);
    }
    
    // ========================================================================
    // CBaseMonster Property Wrappers (for complex types only)
    // Note: Simple properties use asbind20's .property() method directly
    // ========================================================================
    
    // Enemy (EHANDLE converted to CBaseEntity@)
    CBaseEntity* AS_CBaseMonster_get_Enemy(CBaseMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return nullptr;
            return monster->m_hEnemy;
        #else
            return nullptr;
        #endif
    }
    
    void AS_CBaseMonster_set_Enemy(CBaseMonster* monster, CBaseEntity* enemy)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return;
            monster->m_hEnemy = enemy;
        #endif
    }
    
    // Target Entity
    CBaseEntity* AS_CBaseMonster_get_TargetEnt(CBaseMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return nullptr;
            return monster->m_hTargetEnt;
        #else
            return nullptr;
        #endif
    }
    
    void AS_CBaseMonster_set_TargetEnt(CBaseMonster* monster, CBaseEntity* target)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return;
            monster->m_hTargetEnt = target;
        #endif
    }
    
    // Enemy Last Known Position
    Vector AS_CBaseMonster_get_EnemyLKP(CBaseMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return Vector(0,0,0);
            return monster->m_vecEnemyLKP;
        #else
            return Vector(0,0,0);
        #endif
    }
    
    void AS_CBaseMonster_set_EnemyLKP(CBaseMonster* monster, const Vector& pos)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return;
            monster->m_vecEnemyLKP = pos;
        #endif
    }
    
    // ========================================================================
    // CBaseMonster Method Wrappers
    // ========================================================================
    
    void AS_CBaseMonster_Look(CBaseMonster* monster, int distance)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster))
            {
                MS_ANGEL_ERROR("Look: NULL monster pointer");
                return;
            }
            monster->Look(distance);
        #endif
    }
    
    void AS_CBaseMonster_RunAI(CBaseMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster))
            {
                MS_ANGEL_ERROR("RunAI: NULL monster pointer");
                return;
            }
            monster->RunAI();
        #endif
    }
    
    void AS_CBaseMonster_MonsterThink(CBaseMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster))
            {
                MS_ANGEL_ERROR("MonsterThink: NULL monster pointer");
                return;
            }
            monster->MonsterThink();
        #endif
    }
    
    void AS_CBaseMonster_MonsterInit(CBaseMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster))
            {
                MS_ANGEL_ERROR("MonsterInit: NULL monster pointer");
                return;
            }
            monster->MonsterInit();
        #endif
    }
    
    void AS_CBaseMonster_StartMonster(CBaseMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster))
            {
                MS_ANGEL_ERROR("StartMonster: NULL monster pointer");
                return;
            }
            monster->StartMonster();
        #endif
    }
    
    CBaseEntity* AS_CBaseMonster_BestVisibleEnemy(CBaseMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster))
            {
                MS_ANGEL_ERROR("BestVisibleEnemy: NULL monster pointer");
                return nullptr;
            }
            return monster->BestVisibleEnemy();
        #else
            return nullptr;
        #endif
    }
    
    bool AS_CBaseMonster_FInViewCone_Entity(CBaseMonster* monster, CBaseEntity* entity)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster) || !entity)
            {
                return false;
            }
            return monster->FInViewCone(entity);
        #else
            return false;
        #endif
    }
    
    bool AS_CBaseMonster_FInViewCone_Vector(CBaseMonster* monster, const Vector& origin)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster))
            {
                return false;
            }
            Vector vecOrigin = origin;  // Remove const for method call
            return monster->FInViewCone(vecOrigin);
        #else
            return false;
        #endif
    }
    
    int AS_CBaseMonster_IRelationship(CBaseMonster* monster, CBaseEntity* target)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster) || !target)
            {
                return 0;
            }
            return monster->IRelationship(target);
        #else
            return 0;
        #endif
    }
    
    void AS_CBaseMonster_SetState(CBaseMonster* monster, int state)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return;
            monster->SetState(static_cast<MONSTERSTATE>(state));
        #endif
    }
    
    void AS_CBaseMonster_ReportAIState(CBaseMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return;
            monster->ReportAIState();
        #endif
    }
    
    int AS_CBaseMonster_CheckEnemy(CBaseMonster* monster, CBaseEntity* enemy)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster) || !enemy) return 0;
            return monster->CheckEnemy(enemy);
        #else
            return 0;
        #endif
    }
    
    void AS_CBaseMonster_PushEnemy(CBaseMonster* monster, CBaseEntity* enemy, const Vector& lastKnownPos)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster) || !enemy) return;
            Vector pos = lastKnownPos;
            monster->PushEnemy(enemy, pos);
        #endif
    }
    
    bool AS_CBaseMonster_PopEnemy(CBaseMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return false;
            return monster->PopEnemy() ? true : false;
        #else
            return false;
        #endif
    }
    
    void AS_CBaseMonster_ClearSchedule(CBaseMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return;
            monster->ClearSchedule();
        #endif
    }
    
    float AS_CBaseMonster_ChangeYaw(CBaseMonster* monster, int speed)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return 0.0f;
            return monster->ChangeYaw(speed);
        #else
            return 0.0f;
        #endif
    }
    
    bool AS_CBaseMonster_FacingIdeal(CBaseMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return false;
            return monster->FacingIdeal() ? true : false;
        #else
            return false;
        #endif
    }
    
    Vector AS_CBaseMonster_BodyTarget(CBaseMonster* monster, const Vector& posSrc)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return Vector(0,0,0);
            return monster->BodyTarget(posSrc);
        #else
            return Vector(0,0,0);
        #endif
    }
    
    // ========================================================================
    // CMSMonster Property Wrappers (for complex types only - msstring)
    // Note: Simple properties use asbind20's .property() method directly
    // ========================================================================
    
    // Title (msstring needs conversion)
    std::string AS_CMSMonster_get_Title(CMSMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return "";
            return std::string(monster->m_title.c_str());
        #else
            return "";
        #endif
    }
    
    void AS_CMSMonster_set_Title(CMSMonster* monster, const std::string& value)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return;
            monster->m_title = value.c_str();
        #endif
    }
    
    // Script Name (msstring needs conversion)
    std::string AS_CMSMonster_get_ScriptName(CMSMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return "";
            return std::string(monster->m_ScriptName.c_str());
        #else
            return "";
        #endif
    }
    
    void AS_CMSMonster_set_ScriptName(CMSMonster* monster, const std::string& value)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return;
            monster->m_ScriptName = value.c_str();
        #endif
    }
    
    // ========================================================================
    // CMSMonster Method Wrappers
    // ========================================================================
    
    int AS_CMSMonster_GiveGold(CMSMonster* monster, int amount, bool verbose)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster))
            {
                MS_ANGEL_ERROR("GiveGold: NULL monster pointer");
                return 0;
            }
            return monster->GiveGold(amount, verbose);
        #else
            return 0;
        #endif
    }
    
    float AS_CMSMonster_GiveHP(CMSMonster* monster, float amount)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster))
            {
                MS_ANGEL_ERROR("GiveHP: NULL monster pointer");
                return 0.0f;
            }
            return monster->GiveHP(amount);
        #else
            return 0.0f;
        #endif
    }
    
    float AS_CMSMonster_GiveMP(CMSMonster* monster, float amount)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster))
            {
                MS_ANGEL_ERROR("GiveMP: NULL monster pointer");
                return 0.0f;
            }
            return monster->GiveMP(amount);
        #else
            return 0.0f;
        #endif
    }
    
    int AS_CMSMonster_GetNatStat(CMSMonster* monster, int statName)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return 0;
            return monster->GetNatStat(statName);
        #else
            return 0;
        #endif
    }
    
    int AS_CMSMonster_GetSkillStat(CMSMonster* monster, int statIdx)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return 0;
            return monster->GetSkillStat(statIdx);
        #else
            return 0;
        #endif
    }
    
    int AS_CMSMonster_GetSkillStatCount(CMSMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return 0;
            return monster->GetSkillStatCount();
        #else
            return 0;
        #endif
    }
    
    float AS_CMSMonster_WalkSpeed(CMSMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return 0.0f;
            return monster->WalkSpeed();
        #else
            return 0.0f;
        #endif
    }
    
    float AS_CMSMonster_RunSpeed(CMSMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return 0.0f;
            return monster->RunSpeed();
        #else
            return 0.0f;
        #endif
    }
    
    bool AS_CMSMonster_IsActing(CMSMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return false;
            return monster->IsActing();
        #else
            return false;
        #endif
    }
    
    bool AS_CMSMonster_IsShielding(CMSMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return false;
            return monster->IsShielding();
        #else
            return false;
        #endif
    }
    
    float AS_CMSMonster_Weight(CMSMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return 0.0f;
            return monster->Weight();
        #else
            return 0.0f;
        #endif
    }
    
    void AS_CMSMonster_CancelAttack(CMSMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return;
            monster->CancelAttack();
        #endif
    }
    
    void AS_CMSMonster_SetSpeed(CMSMonster* monster)
    {
        #ifdef VALVE_DLL
            if (!monster || FNullEnt(monster)) return;
            monster->SetSpeed();
        #endif
    }
    
    // ========================================================================
    // Casting Functions
    // ========================================================================
    
    CBaseMonster* EntityToMonster_Cast(CBaseEntity* entity)
    {
        if (!entity)
        {
            MS_ANGEL_DEBUG("EntityToMonster_Cast: NULL entity pointer");
            return nullptr;
        }
        
        // Check if this entity is a monster
        CBaseMonster* pMonster = entity->MyMonsterPointer();
        if (!pMonster)
        {
            MS_ANGEL_DEBUG("EntityToMonster_Cast: Entity is not a monster");
            return nullptr;
        }
        
        MS_ANGEL_DEBUG("EntityToMonster_Cast: Successfully cast entity to monster");
        return pMonster;
    }
    
    CMSMonster* EntityToMSMonster_Cast(CBaseEntity* entity)
    {
        if (!entity)
        {
            MS_ANGEL_DEBUG("EntityToMSMonster_Cast: NULL entity pointer");
            return nullptr;
        }
        
        #ifdef VALVE_DLL
            // Check if this entity is an MS monster
            if (!entity->IsMSMonster())
            {
                MS_ANGEL_DEBUG("EntityToMSMonster_Cast: Entity is not an MSMonster");
                return nullptr;
            }
            
            MS_ANGEL_DEBUG("EntityToMSMonster_Cast: Successfully cast entity to MSMonster");
            return static_cast<CMSMonster*>(entity);
        #else
            return nullptr;
        #endif
    }
    
    CMSMonster* MonsterToMSMonster_Cast(CBaseMonster* monster)
    {
        if (!monster)
        {
            MS_ANGEL_DEBUG("MonsterToMSMonster_Cast: NULL monster pointer");
            return nullptr;
        }
        
        #ifdef VALVE_DLL
            // Check if this monster is an MS monster
            CBaseEntity* pEntity = static_cast<CBaseEntity*>(monster);
            if (!pEntity->IsMSMonster())
            {
                MS_ANGEL_DEBUG("MonsterToMSMonster_Cast: Monster is not an MSMonster");
                return nullptr;
            }
            
            MS_ANGEL_DEBUG("MonsterToMSMonster_Cast: Successfully cast monster to MSMonster");
            return static_cast<CMSMonster*>(monster);
        #else
            return nullptr;
        #endif
    }
    
    CBaseEntity* MonsterToEntity_Cast(CBaseMonster* monster)
    {
        if (!monster)
        {
            MS_ANGEL_DEBUG("MonsterToEntity_Cast: NULL monster pointer");
            return nullptr;
        }
        
        MS_ANGEL_DEBUG("MonsterToEntity_Cast: Successfully cast monster to entity");
        return static_cast<CBaseEntity*>(monster);
    }
    
    CBaseEntity* MSMonsterToEntity_Cast(CMSMonster* monster)
    {
        if (!monster)
        {
            MS_ANGEL_DEBUG("MSMonsterToEntity_Cast: NULL monster pointer");
            return nullptr;
        }
        
        MS_ANGEL_DEBUG("MSMonsterToEntity_Cast: Successfully cast MSMonster to entity");
        return static_cast<CBaseEntity*>(monster);
    }
    
    // ========================================================================
    // Enumeration Registration
    // ========================================================================
    
    void RegisterMonsterEnums(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASMonsterBindings] Registering monster enumerations...");
        
        // Gender enum
        pEngine->RegisterEnum("Gender");
        pEngine->RegisterEnumValue("Gender", "GENDER_MALE", 0);
        pEngine->RegisterEnumValue("Gender", "GENDER_FEMALE", 1);
        pEngine->RegisterEnumValue("Gender", "GENDER_UNKNOWN", 2);
        
        // Speech Type enum
        pEngine->RegisterEnum("SpeechType");
        pEngine->RegisterEnumValue("SpeechType", "SPEECH_GLOBAL", 0);
        pEngine->RegisterEnumValue("SpeechType", "SPEECH_LOCAL", 1);
        pEngine->RegisterEnumValue("SpeechType", "SPEECH_PARTY", 2);
        
        // Monster State enum (if not already registered)
        // Note: These may already be defined in engine headers
        pEngine->RegisterEnum("MonsterState");
        pEngine->RegisterEnumValue("MonsterState", "MONSTERSTATE_NONE", 0);
        pEngine->RegisterEnumValue("MonsterState", "MONSTERSTATE_IDLE", 1);
        pEngine->RegisterEnumValue("MonsterState", "MONSTERSTATE_COMBAT", 2);
        pEngine->RegisterEnumValue("MonsterState", "MONSTERSTATE_ALERT", 3);
        pEngine->RegisterEnumValue("MonsterState", "MONSTERSTATE_HUNT", 4);
        pEngine->RegisterEnumValue("MonsterState", "MONSTERSTATE_PRONE", 5);
        pEngine->RegisterEnumValue("MonsterState", "MONSTERSTATE_SCRIPT", 6);
        pEngine->RegisterEnumValue("MonsterState", "MONSTERSTATE_PLAYDEAD", 7);
        pEngine->RegisterEnumValue("MonsterState", "MONSTERSTATE_DEAD", 8);
        
        MS_ANGEL_INFO("[ASMonsterBindings] Monster enumerations registered");
    }
    
    void RegisterMonsterCastingFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASMonsterBindings] Registering monster casting functions...");
        
        asbind20::global(pEngine)
            .function("CBaseMonster@ ToMonster(CBaseEntity@)", EntityToMonster_Cast)
            .function("CMSMonster@ ToMSMonster(CBaseEntity@)", EntityToMSMonster_Cast)
            .function("CMSMonster@ ToMSMonster(CBaseMonster@)", MonsterToMSMonster_Cast)
            .function("CBaseEntity@ ToEntity(CBaseMonster@)", MonsterToEntity_Cast)
            .function("CBaseEntity@ ToEntity(CMSMonster@)", MSMonsterToEntity_Cast);
        
        MS_ANGEL_INFO("[ASMonsterBindings] Monster casting functions registered");
    }
    
    // ========================================================================
    // Type Registration Functions
    // ========================================================================
    
    void RegisterCBaseMonster(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASMonsterBindings] Registering CBaseMonster type with asbind20...");
        
        asbind20::ref_class<CBaseMonster>(pEngine, "CBaseMonster")
            // Reference counting behaviors
            .addref(&AddRef_CBaseMonster)
            .release(&Release_CBaseMonster)
            
            // Properties (using asbind20 .property() for direct member access)
            .property("float m_flFieldOfView", &CBaseMonster::m_flFieldOfView)
            .property("float m_flWaitFinished", &CBaseMonster::m_flWaitFinished)
            .property("float m_flMoveWaitFinished", &CBaseMonster::m_flMoveWaitFinished)
            .property("int m_LastHitGroup", &CBaseMonster::m_LastHitGroup)
            .property("float m_flNextAttack", &CBaseMonster::m_flNextAttack)
            .property("int m_bloodColor", &CBaseMonster::m_bloodColor)
            .property("float m_flDistTooFar", &CBaseMonster::m_flDistTooFar)
            .property("float m_flDistLook", &CBaseMonster::m_flDistLook)
            .property("int m_iMaxHealth", &CBaseMonster::m_iMaxHealth)
            .property("int m_afCapability", &CBaseMonster::m_afCapability)
            // Note: Activity and MONSTERSTATE enums are accessed as their underlying int type
            // Complex properties that need wrapper functions
            .method("CBaseEntity@ get_Enemy() const", AS_CBaseMonster_get_Enemy)
            .method("void set_Enemy(CBaseEntity@)", AS_CBaseMonster_set_Enemy)
            .method("CBaseEntity@ get_TargetEnt() const", AS_CBaseMonster_get_TargetEnt)
            .method("void set_TargetEnt(CBaseEntity@)", AS_CBaseMonster_set_TargetEnt)
            .method("Vector3 get_EnemyLKP() const", AS_CBaseMonster_get_EnemyLKP)
            .method("void set_EnemyLKP(const Vector3 &in)", AS_CBaseMonster_set_EnemyLKP)
            
            // Core Methods
            .method("int BloodColor()", &CBaseMonster::BloodColor)
            .method("void Look(int)", AS_CBaseMonster_Look)
            .method("void RunAI()", AS_CBaseMonster_RunAI)
            .method("bool IsAlive()", &CBaseMonster::IsAlive)
            .method("void MonsterThink()", AS_CBaseMonster_MonsterThink)
            .method("int IRelationship(CBaseEntity@)", AS_CBaseMonster_IRelationship)
            .method("void MonsterInit()", AS_CBaseMonster_MonsterInit)
            .method("void StartMonster()", AS_CBaseMonster_StartMonster)
            .method("CBaseEntity@ BestVisibleEnemy()", AS_CBaseMonster_BestVisibleEnemy)
            .method("bool FInViewCone(CBaseEntity@)", AS_CBaseMonster_FInViewCone_Entity)
            .method("bool FInViewCone(const Vector3 &in)", AS_CBaseMonster_FInViewCone_Vector)
            .method("void SetState(int)", AS_CBaseMonster_SetState)
            .method("void ReportAIState()", AS_CBaseMonster_ReportAIState)
            .method("int CheckEnemy(CBaseEntity@)", AS_CBaseMonster_CheckEnemy)
            .method("void PushEnemy(CBaseEntity@, const Vector3 &in)", AS_CBaseMonster_PushEnemy)
            .method("bool PopEnemy()", AS_CBaseMonster_PopEnemy)
            .method("void ClearSchedule()", AS_CBaseMonster_ClearSchedule)
            .method("float ChangeYaw(int)", AS_CBaseMonster_ChangeYaw)
            .method("bool FacingIdeal()", AS_CBaseMonster_FacingIdeal)
            .method("Vector3 BodyTarget(const Vector3 &in)", AS_CBaseMonster_BodyTarget)
            
            // Condition and Memory Methods (inlined, so call directly)
            .method("void SetConditions(int)", &CBaseMonster::SetConditions)
            .method("void ClearConditions(int)", &CBaseMonster::ClearConditions)
            .method("bool HasConditions(int)", &CBaseMonster::HasConditions)
            .method("bool HasAllConditions(int)", &CBaseMonster::HasAllConditions)
            .method("void Remember(int)", &CBaseMonster::Remember)
            .method("void Forget(int)", &CBaseMonster::Forget)
            .method("bool HasMemory(int)", &CBaseMonster::HasMemory)
            .method("bool HasAllMemories(int)", &CBaseMonster::HasAllMemories)
            
            // Task Methods (inlined, so call directly)
            .method("void TaskComplete()", &CBaseMonster::TaskComplete)
            .method("void TaskFail()", &CBaseMonster::TaskFail)
            .method("void TaskBegin()", &CBaseMonster::TaskBegin)
            .method("bool IsMoving()", &CBaseMonster::IsMoving)
            
            // Stop method
            .method("void Stop()", &CBaseMonster::Stop)
            
            // Inherit from CBaseEntity (called AFTER registering CBaseMonster's own methods
            // so that overrides like IsAlive() are registered first)
            .base<CBaseEntity>();
        
        MS_ANGEL_INFO("[ASMonsterBindings] CBaseMonster registration complete");
    }
    
    void RegisterCMSMonster(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASMonsterBindings] Registering CMSMonster type with asbind20...");
        
        asbind20::ref_class<CMSMonster>(pEngine, "CMSMonster")
            // Reference counting behaviors
            .addref(&AddRef_CMSMonster)
            .release(&Release_CMSMonster)
            
            // Properties (using asbind20 .property() for direct member access)
            .property("float m_HP", &CMSMonster::m_HP)
            .property("float m_MP", &CMSMonster::m_MP)
            .property("int m_Gold", &CMSMonster::m_Gold)
            .property("float m_StepSize", &CMSMonster::m_StepSize)
            .property("float m_SpeedMultiplier", &CMSMonster::m_SpeedMultiplier)
            .property("float m_TurnRate", &CMSMonster::m_TurnRate)
            .property("float m_Width", &CMSMonster::m_Width)
            .property("float m_Height", &CMSMonster::m_Height)
            .property("uint8 m_Gender", &CMSMonster::m_Gender)
            .property("int m_Lives", &CMSMonster::m_Lives)
            .property("float m_SkillLevel", &CMSMonster::m_SkillLevel)
            .property("bool m_Menu_Autoopen", &CMSMonster::m_Menu_Autoopen)
            .property("float m_HITMulti", &CMSMonster::m_HITMulti)
            .property("float m_HPMulti", &CMSMonster::m_HPMulti)
            .property("float m_DMGMulti", &CMSMonster::m_DMGMulti)
            .property("float m_HearingSensitivity", &CMSMonster::m_HearingSensitivity)
            // Complex properties that need wrapper functions (msstring types)
            .method("string get_Title()", AS_CMSMonster_get_Title)
            .method("void set_Title(const string &in)", AS_CMSMonster_set_Title)
            .method("string get_ScriptName()", AS_CMSMonster_get_ScriptName)
            .method("void set_ScriptName(const string &in)", AS_CMSMonster_set_ScriptName)
            
            // Lifecycle Methods
            .method("bool IsMSMonster()", &CMSMonster::IsMSMonster)
            .method("float MaxHP()", &CMSMonster::MaxHP)
            .method("float MaxMP()", &CMSMonster::MaxMP)
            .method("bool IsAlive()", &CMSMonster::IsAlive)
            
            // Economy Methods
            .method("int GiveGold(int, bool)", AS_CMSMonster_GiveGold)
            .method("int GiveGold(int)", +[](CMSMonster* m, int amt) { return AS_CMSMonster_GiveGold(m, amt, true); })
            .method("float GiveHP(float)", AS_CMSMonster_GiveHP)
            .method("float GiveMP(float)", AS_CMSMonster_GiveMP)
            
            // Stats Methods
            .method("int GetNatStat(int)", AS_CMSMonster_GetNatStat)
            .method("int GetSkillStat(int)", AS_CMSMonster_GetSkillStat)
            .method("int GetSkillStatCount()", AS_CMSMonster_GetSkillStatCount)
            
            // Movement Methods
            .method("float WalkSpeed()", AS_CMSMonster_WalkSpeed)
            .method("float RunSpeed()", AS_CMSMonster_RunSpeed)
            .method("bool IsFlying()", &CMSMonster::IsFlying)
            
            // Combat Methods
            .method("bool IsActing()", AS_CMSMonster_IsActing)
            .method("bool IsShielding()", AS_CMSMonster_IsShielding)
            .method("void CancelAttack()", AS_CMSMonster_CancelAttack)
            
            // Misc Methods
            .method("float Weight()", AS_CMSMonster_Weight)
            .method("void SetSpeed()", AS_CMSMonster_SetSpeed)
            
            // Inherit from CBaseMonster (called AFTER registering CMSMonster's own methods
            // so that overrides like IsAlive() are registered first)
            // NOTE: This automatically inherits all CBaseMonster AND CBaseEntity methods!
            .base<CBaseMonster>();
        
        MS_ANGEL_INFO("[ASMonsterBindings] CMSMonster registration complete");
    }
    
    void RegisterAll(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASMonsterBindings] Starting monster bindings registration...");
        
        // Register enums first
        RegisterMonsterEnums(pEngine);
        
        // Register types in inheritance order (must come after CBaseEntity is registered)
        RegisterCBaseMonster(pEngine);
        RegisterCMSMonster(pEngine);
        
        // Register casting functions
        RegisterMonsterCastingFunctions(pEngine);
        
        MS_ANGEL_INFO("[ASMonsterBindings] Monster bindings registration complete");
    }
}

