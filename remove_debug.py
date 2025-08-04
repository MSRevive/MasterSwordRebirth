#!/usr/bin/env python3
import re
import os
import sys

def remove_debug_from_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # Remove dbg(...) calls
        content = re.sub(r'\s*dbg\([^)]*\);\s*\n', '', content)
        
        # Remove genitemdbg(...) calls
        content = re.sub(r'\s*genitemdbg\([^)]*\);\s*\n', '', content)
        
        # Remove #define genitemdbg line
        content = re.sub(r'#define genitemdbg\([^)]*\)[^\n]*\n', '', content)
        
        # Replace startdbg; with try {
        content = re.sub(r'(\s*)startdbg;', r'\1try {', content)
        
        # Replace enddbg; with }
        content = re.sub(r'(\s*)enddbg;', r'\1}', content)
        
        # Replace enddbgprt(...); with }
        content = re.sub(r'(\s*)enddbgprt\([^)]*\);', r'\1}', content)
        
        # Remove SetDebugProgress calls
        content = re.sub(r'\s*SetDebugProgress\([^)]*\);\s*\n', '', content)
        
        if content != original_content:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        return False
        
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return False

files_to_process = [
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/shared/weapons/genericitem.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/shared/weapons/giattack.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/shared/weapons/gipack.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/shared/weapons/giprojectile.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/shared/weapons/weapons.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/shared/ms/script.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/shared/ms/msmonstershared.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/shared/ms/netcodeshared.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/shared/ms/global.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/shared/movement/pm_shared.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/player/player.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/player/playershared.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/player/playerstats.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/shield.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/sv_character.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/monsters/msmonsterserver.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/monsters/npcscript.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/hl/util.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/hl/monsters.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/hl/cbase.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/func_break.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/gamerules/multiplay_gamerules.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/effects/mseffects.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/bmodels.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/server/client.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ui/vgui_status.h",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ui/vgui_teamfortressviewport.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/view.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ui/vgui_int.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ui/ms/vgui_stats.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ui/ms/vgui_menu_interact.h",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ui/ms/vgui_menu_main.h",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ui/ms/vgui_menubase.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ui/ms/vgui_mscontrols.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ui/ms/vgui_options.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ui/ms/vgui_spawn.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ui/ms/vgui_choosecharacter.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ui/ms/vgui_container.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/render/clrendermirror.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/render/studiomodelrenderer.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ms/action.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ms/clglobal.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ms/clplayer.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ms/health.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ms/hudid.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/ms/hudmisc.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/hud_msg.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/hud_spectator.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/hud.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/input.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/entity.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/hl/hl_baseentity.cpp",
    "/home/runner/work/MasterSwordRebirth/MasterSwordRebirth/src/game/client/cl_util.h"
]

modified_files = []
for filepath in files_to_process:
    if os.path.exists(filepath):
        if remove_debug_from_file(filepath):
            modified_files.append(filepath)
            print(f"Modified: {filepath}")
        else:
            print(f"No changes needed: {filepath}")
    else:
        print(f"File not found: {filepath}")

print(f"\nTotal files modified: {len(modified_files)}")