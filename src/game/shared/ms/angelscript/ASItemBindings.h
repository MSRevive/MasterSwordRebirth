//==========================================================================
// ASItemBindings.h
// 
// Item and weapon type bindings for AngelScript integration
// Includes CBaseAnimating, CBasePlayerItem, and CBasePlayerWeapon classes
//==========================================================================

#ifndef AS_ITEM_BINDINGS_H
#define AS_ITEM_BINDINGS_H

#pragma once

class asIScriptEngine;

namespace ASItemBindings
{
    void RegisterAll(asIScriptEngine* pEngine);
    void RegisterCBaseAnimating(asIScriptEngine* pEngine);
    void RegisterCBasePlayerItem(asIScriptEngine* pEngine);
    void RegisterCBasePlayerWeapon(asIScriptEngine* pEngine);
    void RegisterCrossReferences(asIScriptEngine* pEngine);  // Must be called AFTER CBasePlayer is registered
    
    // Item/Weapon Management Methods on CBasePlayer (see ASEntityBindings):
    // - CBasePlayerItem@ GetItemBySlot(int slot) const
    // - CBasePlayerWeapon@ GetActiveWeapon() const
    // - array<CBasePlayerItem@>@ GetInventory() const
    // - bool HasItem(const string& itemName) const
}

#endif // AS_ITEM_BINDINGS_H

