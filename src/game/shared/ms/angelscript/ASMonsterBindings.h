//==========================================================================
// ASMonsterBindings.h
// 
// CBaseMonster and CMSMonster type bindings for AngelScript integration
// Provides comprehensive monster AI and stats functionality for script usage
//==========================================================================

#pragma once

class asIScriptEngine;

namespace ASMonsterBindings
{
    // Main registration functions
    void RegisterAll(asIScriptEngine* pEngine);
    void RegisterCBaseMonster(asIScriptEngine* pEngine);
    void RegisterCMSMonster(asIScriptEngine* pEngine);
    void RegisterMonsterEnums(asIScriptEngine* pEngine);
    void RegisterMonsterCastingFunctions(asIScriptEngine* pEngine);
    
    // CBaseMonster Functions Available in AngelScript:
    // 
    // Properties:
    // - float FieldOfView - Monster's view cone width (dot product)
    // - float WaitFinished - Time when wait completes
    // - Activity Activity - Current animation activity
    // - Activity IdealActivity - Desired animation activity
    // - int MonsterState - Current AI state
    // - int IdealMonsterState - Desired AI state
    // - int LastHitGroup - Last body region damaged
    // - float NextAttack - Time of next allowed attack
    // - int BloodColor - Blood particle color
    // - float DistTooFar - Distance threshold for enemy too far
    // - float DistLook - Monster sight distance
    // 
    // Methods:
    // - int BloodColor() - Get blood color value
    // - void Look(int distance) - Look for targets
    // - void RunAI() - Execute AI tick
    // - bool IsAlive() - Check if alive
    // - void MonsterThink() - AI think function
    // - int IRelationship(CBaseEntity@) - Get relationship to entity
    // - void MonsterInit() - Initialize monster
    // - void StartMonster() - Activate monster
    // - CBaseEntity@ BestVisibleEnemy() - Find best visible enemy
    // - bool FInViewCone(CBaseEntity@) - Check if entity in view cone
    // - void SetActivity(int) - Set animation activity
    // - void SetState(int) - Set monster state
    // - void SetConditions(int) - Set condition flags
    // - void ClearConditions(int) - Clear condition flags
    // - bool HasConditions(int) - Check condition flags
    // - void Remember(int) - Set memory flag
    // - void Forget(int) - Clear memory flag
    // - bool HasMemory(int) - Check memory flag
    // - void TaskComplete() - Mark task done
    // - void TaskFail() - Mark task failed
    // - void ClearSchedule() - Clear current schedule
    // - bool IsMoving() - Check if moving
    // 
    // CMSMonster Functions Available in AngelScript:
    // 
    // Properties:
    // - float HP - Current health
    // - float MP - Current mana
    // - float MaxHP - Maximum health (read via MaxHP() method)
    // - float MaxMP - Maximum mana (read via MaxMP() method)
    // - int Gold - Gold amount
    // - float StepSize - Step up height
    // - float SpeedMultiplier - Speed modifier
    // - float TurnRate - Turn speed ratio
    // - float Width - Monster width
    // - float Height - Monster height
    // - int Gender - Monster gender
    // - string Title - Monster display title
    // - string ScriptName - Script file name
    // - int Lives - Remaining lives
    // - float SkillLevel - Experience worth
    // - bool MenuAutoOpen - Auto-open menu on use
    // 
    // Methods:
    // - bool IsMSMonster() - Check if MS monster
    // - float MaxHP() - Get effective max HP
    // - float MaxMP() - Get effective max MP
    // - int GiveGold(int amount) - Give/take gold
    // - float GiveHP(float amount) - Give/take HP
    // - float GiveMP(float amount) - Give/take MP
    // - int GetNatStat(int stat) - Get natural stat
    // - int GetSkillStat(int stat) - Get skill stat
    // - float WalkSpeed() - Get walk speed
    // - float RunSpeed() - Get run speed
    // - bool IsActing() - Check if attacking
    // - bool IsShielding() - Check if blocking
    // - bool IsFlying() - Check if flying
    // - float Weight() - Get total weight
    // - void CancelAttack() - Cancel current attack
    // - void SetSpeed() - Recalculate speed
}

