//==========================================================================
// ASClientItemBindings.cpp - Client-side item bindings for AngelScript
//
// Provides read-only access to CGenericItem properties for client-side
// UI, display, and visualization purposes. NO game logic methods exposed.
//==========================================================================

// Include weapon definitions (provides CGenericItem) - must come FIRST
#include "inc_weapondefs.h"
#undef DLLEXPORT

#include "ASClientItemBindings.h"
#include <asbind20/asbind.hpp>

// Master Sword includes
#include "mslogger.h"

//==========================================================================
// Reference Counting for CGenericItem
// Note: Items are engine-managed, so these are no-ops
//==========================================================================

static void AddRef_CGenericItem(CGenericItem* item)
{
    // No-op: Engine manages item lifetime
}

static void Release_CGenericItem(CGenericItem* item)
{
    // No-op: Engine manages item lifetime
}

//==========================================================================
// CGenericItem Property Accessors (Read-Only)
//==========================================================================

#ifndef VALVE_DLL

// Basic Properties
static std::string AS_CGenericItem_GetItemName(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetItemName - NULL item pointer");
        return "";
    }
    return std::string(pItem->m_Name);
}

static int AS_CGenericItem_GetItemID(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetItemID - NULL item pointer");
        return 0;
    }
    return pItem->m_iId;
}

static int AS_CGenericItem_GetQuality(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetQuality - NULL item pointer");
        return 0;
    }
    return pItem->Quality;
}

static int AS_CGenericItem_GetMaxQuality(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetMaxQuality - NULL item pointer");
        return 0;
    }
    return pItem->MaxQuality;
}

static int AS_CGenericItem_GetQuantity(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetQuantity - NULL item pointer");
        return 0;
    }
    return pItem->iQuantity;
}

static int AS_CGenericItem_GetMaxQuantity(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetMaxQuantity - NULL item pointer");
        return 0;
    }
    return pItem->iMaxGroupable;
}

static float AS_CGenericItem_GetWeight(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetWeight - NULL item pointer");
        return 0.0f;
    }
    return pItem->Weight();
}

static unsigned int AS_CGenericItem_GetValue(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetValue - NULL item pointer");
        return 0;
    }
    return pItem->m_Value;
}

static int AS_CGenericItem_GetLocation(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetLocation - NULL item pointer");
        return ITEMPOS_NONE;
    }
    return pItem->m_Location;
}

static int AS_CGenericItem_GetHand(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetHand - NULL item pointer");
        return LEFT_HAND;
    }
    return pItem->m_Hand;
}

// Visual/Rendering Properties
static std::string AS_CGenericItem_GetViewModel(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetViewModel - NULL item pointer");
        return "";
    }
    return pItem->m_ViewModel.c_str();
}

static std::string AS_CGenericItem_GetPlayerHoldModel(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetPlayerHoldModel - NULL item pointer");
        return "";
    }
    return pItem->m_PlayerHoldModel.c_str();
}

static std::string AS_CGenericItem_GetAnimExtension(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetAnimExtension - NULL item pointer");
        return "";
    }
    return pItem->m_AnimExt.c_str();
}

static std::string AS_CGenericItem_GetAnimExtensionLegs(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetAnimExtensionLegs - NULL item pointer");
        return "";
    }
    return pItem->m_AnimExtLegs.c_str();
}

static int AS_CGenericItem_GetRenderMode(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetRenderMode - NULL item pointer");
        return kRenderNormal;
    }
    return pItem->m_RenderMode;
}

static int AS_CGenericItem_GetRenderAmount(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetRenderAmount - NULL item pointer");
        return 255;
    }
    return pItem->m_RenderAmt;
}

static int AS_CGenericItem_GetRenderFx(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetRenderFx - NULL item pointer");
        return 0;
    }
    return pItem->m_RenderFx;
}

static int AS_CGenericItem_GetSkin(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetSkin - NULL item pointer");
        return 0;
    }
    return pItem->m_Skin;
}

static int AS_CGenericItem_GetViewModelAnim(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetViewModelAnim - NULL item pointer");
        return 0;
    }
    return pItem->m_ViewModelAnim;
}

static float AS_CGenericItem_GetViewModelAnimSpeed(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetViewModelAnimSpeed - NULL item pointer");
        return 1.0f;
    }
    return pItem->m_ViewModelAnimSpeed;
}

// Container Methods (read-only)
static int AS_CGenericItem_Container_ItemCount(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::Container_ItemCount - NULL item pointer");
        return 0;
    }
    return pItem->Container_ItemCount();
}

static float AS_CGenericItem_Container_Weight(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::Container_Weight - NULL item pointer");
        return 0.0f;
    }
    return pItem->Container_Weight();
}

static bool AS_CGenericItem_Container_IsOpen(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::Container_IsOpen - NULL item pointer");
        return false;
    }
    return pItem->Container_IsOpen();
}

static float AS_CGenericItem_Volume(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::Volume - NULL item pointer");
        return 0.0f;
    }
    return pItem->Volume();
}

// Item Type Checks (using MSProperties() bitmasks)
static bool AS_CGenericItem_IsWearable(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::IsWearable - NULL item pointer");
        return false;
    }
    return FBitSet(pItem->MSProperties(), ITEM_WEARABLE);
}

static bool AS_CGenericItem_IsShield(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::IsShield - NULL item pointer");
        return false;
    }
    return FBitSet(pItem->MSProperties(), ITEM_SHIELD);
}

static bool AS_CGenericItem_IsGroupable(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::IsGroupable - NULL item pointer");
        return false;
    }
    return FBitSet(pItem->MSProperties(), ITEM_GROUPABLE);
}

static bool AS_CGenericItem_IsContainer(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::IsContainer - NULL item pointer");
        return false;
    }
    return FBitSet(pItem->MSProperties(), ITEM_CONTAINER);
}

static bool AS_CGenericItem_IsProjectile(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::IsProjectile - NULL item pointer");
        return false;
    }
    return FBitSet(pItem->MSProperties(), ITEM_PROJECTILE);
}

static bool AS_CGenericItem_IsDrinkable(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::IsDrinkable - NULL item pointer");
        return false;
    }
    return FBitSet(pItem->MSProperties(), ITEM_DRINKABLE);
}

static bool AS_CGenericItem_IsPerishable(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::IsPerishable - NULL item pointer");
        return false;
    }
    return FBitSet(pItem->MSProperties(), ITEM_PERISHABLE);
}

static bool AS_CGenericItem_IsSpell(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::IsSpell - NULL item pointer");
        return false;
    }
    return FBitSet(pItem->MSProperties(), ITEM_SPELL);
}

// Attack Information (read-only count for UI)
static int AS_CGenericItem_GetAttackCount(CGenericItem* pItem)
{
    if (!pItem)
    {
        MS_ANGEL_ERROR("CGenericItem::GetAttackCount - NULL item pointer");
        return 0;
    }
    return pItem->m_Attacks.size();
}

#endif // !VALVE_DLL

//==========================================================================
// CGenericItem Registration
//==========================================================================

void ASClientItemBindings::RegisterAll(asIScriptEngine* pEngine)
{
#ifndef VALVE_DLL
    MS_ANGEL_INFO("[ASClientItemBindings] Registering CGenericItem (client-side)...");

    try
    {
        auto bind = asbind20::ref_class<CGenericItem>(pEngine, "CGenericItem");

        // Reference counting
        bind.addref(&AddRef_CGenericItem);
        bind.release(&Release_CGenericItem);

        // Basic Properties (read-only)
        bind.method("string GetItemName() const", AS_CGenericItem_GetItemName);
        bind.method("int GetItemID() const", AS_CGenericItem_GetItemID);
        bind.method("int GetQuality() const", AS_CGenericItem_GetQuality);
        bind.method("int GetMaxQuality() const", AS_CGenericItem_GetMaxQuality);
        bind.method("int GetQuantity() const", AS_CGenericItem_GetQuantity);
        bind.method("int GetMaxQuantity() const", AS_CGenericItem_GetMaxQuantity);
        bind.method("float GetWeight() const", AS_CGenericItem_GetWeight);
        bind.method("uint GetValue() const", AS_CGenericItem_GetValue);
        bind.method("int GetLocation() const", AS_CGenericItem_GetLocation);
        bind.method("int GetHand() const", AS_CGenericItem_GetHand);

        // Visual/Rendering Properties
        bind.method("string GetViewModel() const", AS_CGenericItem_GetViewModel);
        bind.method("string GetPlayerHoldModel() const", AS_CGenericItem_GetPlayerHoldModel);
        bind.method("string GetAnimExtension() const", AS_CGenericItem_GetAnimExtension);
        bind.method("string GetAnimExtensionLegs() const", AS_CGenericItem_GetAnimExtensionLegs);
        bind.method("int GetRenderMode() const", AS_CGenericItem_GetRenderMode);
        bind.method("int GetRenderAmount() const", AS_CGenericItem_GetRenderAmount);
        bind.method("int GetRenderFx() const", AS_CGenericItem_GetRenderFx);
        bind.method("int GetSkin() const", AS_CGenericItem_GetSkin);
        bind.method("int GetViewModelAnim() const", AS_CGenericItem_GetViewModelAnim);
        bind.method("float GetViewModelAnimSpeed() const", AS_CGenericItem_GetViewModelAnimSpeed);

        // Item Type Checks
        bind.method("bool IsWorn() const", &CGenericItem::IsWorn);
        bind.method("bool IsArmor() const", &CGenericItem::IsArmor);
        bind.method("bool IsWearable() const", AS_CGenericItem_IsWearable);
        bind.method("bool IsShield() const", AS_CGenericItem_IsShield);
        bind.method("bool IsGroupable() const", AS_CGenericItem_IsGroupable);
        bind.method("bool IsContainer() const", AS_CGenericItem_IsContainer);
        bind.method("bool IsProjectile() const", AS_CGenericItem_IsProjectile);
        bind.method("bool IsDrinkable() const", AS_CGenericItem_IsDrinkable);
        bind.method("bool IsPerishable() const", AS_CGenericItem_IsPerishable);
        bind.method("bool IsSpell() const", AS_CGenericItem_IsSpell);

        // Container Methods (read-only)
        bind.method("int Container_ItemCount() const", AS_CGenericItem_Container_ItemCount);
        bind.method("float Container_Weight() const", AS_CGenericItem_Container_Weight);
        bind.method("bool Container_IsOpen() const", AS_CGenericItem_Container_IsOpen);
        bind.method("float Volume() const", AS_CGenericItem_Volume);

        // Attack Information (read-only count for UI)
        bind.method("int GetAttackCount() const", AS_CGenericItem_GetAttackCount);

        // Inherit from CBasePlayerItem (once server-side bindings are ported)
        // bind.base<CBasePlayerItem>();

        MS_ANGEL_INFO("[ASClientItemBindings] CGenericItem registered successfully (28 methods)");
    }
    catch (const std::exception& e)
    {
        MS_ANGEL_ERROR(UTIL_VarArgs("[ASClientItemBindings] ERROR: CGenericItem registration failed - %s", e.what()));
    }
    catch (...)
    {
        MS_ANGEL_ERROR("[ASClientItemBindings] ERROR: CGenericItem registration failed - Unknown exception");
    }
#else
    MS_ANGEL_DEBUG("[ASClientItemBindings] Skipping client-side registration (server build)");
#endif
}
