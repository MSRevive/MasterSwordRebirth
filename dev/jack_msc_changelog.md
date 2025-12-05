# Master Sword: Rebirth J.A.C.K. FGD file
##### By MS:C community

## Changelog:
v1.000 – Thursday, September 21, 2017 – First release
The base file for this FGD file is halflife.fgd that’s distributed along with J.A.C.K. on Steam. A copy
of this file was made and all obsolete entities were removed. These are mostly entities that don’t work
in MS:C. The full list of removed entities is as follows:
* aiscripted_sequence
* ammo_*
* cycler
* cycler_*
* env_beverage
* env_global
* func_healthcharger
* func_recharge
* game_end
* game_player_equip
* game_player_team
* game_score
* game_team_*
* game_zone_player
* info_bigmomma
* info_intermission
* info_landmark
* info_node
* info_node_air
* info_player_coop
* info_player_deathmatch
* item_*
* monster_*
* monstermaker
* player_*
* scripted_*
* speaker
* target_cdaudio
* trigger_auto
* trigger_autosave
* trigger_cdaudio
* trigger_changelevel
* trigger_endsection
* trigger_transition
* weapon_*
* weaponbox
* world_items
* xen_*

A few unused BaseClasses were also removed:
* Ammo
* Appearflags
* Monster
* TalkMonster
* Global
* Weapon


Secondly, the modified FGD file was compared to MS:C’s halflife.fgd using Diff Checker. All of
MS:C’s changes have been added, such as the Renderless BaseClass, the ‘Unblockable’ spawnflag for
the Door BaseClass and much more, with a few exceptions, notably:
* BaseClass Light removed ‘movewith’ key (because it doesn’t work)
* BaseClass Targetname removed ‘note’ key
* game_counter defaulted ‘Reset On fire’ spawnflag to 0
* trigger_relay defaulted ‘Remove On fire’ spawnflag to 0

Next, almost all MS:C entities from ms1.4.fgd were added. Obsolete entities that have been omitted
are:
* info_compile_parameters
* info_texlights (is defined in zhlt.fgd and is also made obsolete by the light_surface entity that
can be found in zhlt.fgd from the more recent versions of vluzacn’s ZHLT)
* ms_npcscript (use mstrig_act instead)
* msarea_nosave
* msmonster_giantboar (there was a duplicate entry)
* mstrig_multi (use multi_manager instead)
* mstrig_push (use trigger_push instead)
* mstrig_relay (use trigger_relay instead)
Two BaseClasses from ms1.4.fgd were removed:
* MSRenderless (made obsolete by the Renderless BaseClass)
* mstarget (made obsolete by the Target BaseClass)

Special care was taken in piecing together the worldspawn entity. The best of both worlds was kept:
The extended J.A.C.K. FGD file format (if applicable) and the latest MS:C edits.
After that came the bulk of the work. First of all, as a matter of convention, all BaseClass names have
been changed to all caps. A bunch of BaseClasses have also been renamed.
Next they were listed in alphabetical order… sort of. If a certain BaseClass is needed for another one,
the BaseClass that’s being depended on is moved up because it has to be defined first. This is why the
TARGETNAME BaseClass appears before the BREAKABLE BaseClass, for example.
The full list of all 22 BaseClasses is now (left: original name, right: new name):
* Angles ANGLES
* Targetname TARGETNAME
* Breakable BREAKABLE
* [N/A] MASTER (new BaseClass)
* RenderFxChoices RENDERFXCHOICES
* RenderFields RENDERFIELDS
* Door DOOR
* gibshooterbase GIBSHOOT
* Light LIGHT
* msscripted MSC_SCRIPTED
* ms_basenpc MSC_NPC
* PlatSounds PLATSOUNDS
* PlayerClass PLAYERCLASS
* Target TARGET
* BaseTank TANK
* Targetx TARGETX
* Trackchange TRACKCHANGE
* Trigger TRIGGER
* ZHLT ZHLT_BRUSH
* ZHLT_point ZHLT_POINT
* Renderless ZHLT_RENDERLESS
* ms_area MSC_AREA


Another convention related to BaseClasses: In the base() header of BaseClasses and entities,
TARGETNAME, TARGET (or TARGETX) and/or MASTER now appear first, in that order (should they be
needed). Other entries inside the base() header have been listed alphabetically.
Removed BaseClasses were also removed from all base() headers where they appeared.
The headers for all BaseClasses and entities, such as base(), color(), sequence() and so on have been
listed alphabetically.
All entities have also been listed alphabetically.
As a matter of convention, the ‘spawnflags’ key has been moved to the end of every BaseClass and
entity that has this key.
Some keyvalue replacements that have been made on various entities are (left: original, right: new):
* target(target_source) target(target_destination)
* message(target_source) message(target_destination)
* *target(string) *target(target_destination)
* spawnarea(string) spawnarea(target_destination)

The ‘master’ key has been replaced with the MASTER BaseClass for the following:
* BaseClass DOOR
* BaseClass TANK
* BaseClass TRIGGER
* button_target
* func_button
* func_rot_button
* game_counter
* game_counter_set
* game_player_hurt
* game_text
* momentary_rot_button
* msarea_music
* msarea_music_dynamic
* msarea_transition
* msarea_transition_local
* trigger_hurt
* trigger_monsterjump
* trigger_relay

The ZHLT_RENDERLESS BaseClass has been (re)added to:
* BaseClass MSC_AREA
* env_bubbles
* func_ladder
* func_monsterclip
* func_mortar_field
* func_tankcontrols
* func_traincontrols
* msarea_monsterspawn
* msarea_music
* trigger_counter
* trigger_gravity
* trigger_hurt
* trigger_monsterjump
* trigger_multiple
* trigger_once
* trigger_push
* trigger_teleport


Other notable changes:
* worldspawn defaulted ‘MaxRange’ key to 16384
* worldspawn defaulted ‘skyname’ key to arcd
* BaseClass BeamStartEnd merged keys into env_beam
* BaseClass ms_baseitem merged keys into msitem_spawn
* BaseClass Sequence merged key into env_model
* BaseClass spawnspot merged key into the ms_player_* entities
* BaseClass BREAKABLE removed ‘spawnobject’ key
* BaseClass MASTER changed master(string) to master(target_destination)
* BaseClass MSC_NPC defaulted ‘*multi’ keys to 1.00
* BaseClass MSC_NPC defaulted ‘nplayers’ key to 1
* BaseClass MSC_NPC removed ‘spawnwith’ key
* env_model added zhlt_copylight(target_destination) keyvalue
* env_model added zhlt_usemodel(target_destination) keyvalue
* func_button added killtarget(target_destination) keyvalue
* func_button added zhlt_noclip(choices) keyvalue
* func_door added scale(string) keyvalue
* func_door added zhlt_usemodel(target_destination) keyvalue
* func_door_rotating defaulted ‘distance’ key to 85
* func_pushable removed non-functional ‘size’ key
* func_rot_button removed non-functional ‘changetarget’ key
* func_rotating added scriptname(string) keyvalue
* func_wall added zhlt_usemodel(target_destination) keyvalue
* func_wall_toggle added scriptname(string) keyvalue
* func_wall_toggle added zhlt_noclip(choices) keyvalue
* light_environment added _diffuse_light(color255) keyvalue
* light_environment added _spread(string) keyvalue
* ms_monsterspawn defaulted ‘nplayers’ key to 1
* ms_monsterspawn removed ‘spawnloc’ key
* msarea_monsterspawn is now based on ms_monsterspawn
* msarea_music is now based on mstrig_music
* msmonster_random defaulted ‘random_*_*multi’ keys to 1.00
* msmonster_random defaulted ‘random_*_nplayers’ keys to 1
* mstrig_weather added some missing entries under ‘weather’ key
* multi_manager changed random(integer) to random(choices)
* multisource removed ‘globalstate’ key
Some changes to the formatting/layout have also been made:
* removed double spaces
* removed spaces at the end of lines
* removed tabs at the end of lines
* replaced spaces followed by tabs (or vice versa) with single spaces
* removed double empty lines
* replaced all instances of “Jackhammer” with “J.A.C.K.”
And last but not least there’s hundreds of minor changes that are not listed here, mostly related to
cleaning up the FGD file, aesthetics and adding headers (such as the color() and iconsprite() headers)
to differentiate between the different types of point-based entities.


This FGD file refers to some custom content (sprites and models) from:
* banan2288's Sven Co-op J.A.C.K. FGD file - (Link lost to time)
	* ...\models\path_corner.mdl
	* ...\models\path_track.mdl
	* ...\sprites\env_beam.spr
	* ...\sprites\env_blood.spr
	* ...\sprites\env_exposion.spr
	* ...\sprites\env_fade.spr
	* ...\sprites\env_funnel.spr
	* ...\sprites\env_laser.spr
	* ...\sprites\env_render.spr
	* ...\sprites\env_shake.spr
	* ...\sprites\env_shooter.spr
	* ...\sprites\env_spark.spr
	* ...\sprites\game_counter.spr
	* ...\sprites\game_counter_set.spr
	* ...\sprites\game_player_hurt.spr
	* ...\sprites\game_text.spr
	* ...\sprites\gibshooter.spr
	* ...\sprites\info_null.spr
	* ...\sprites\info_target.spr
	* ...\sprites\multi_manager.spr
	* ...\sprites\multisource.spr
	* ...\sprites\scripted_sentence.spr
	* ...\sprites\trigger_changetarget.spr
	* ...\sprites\trigger_relay.spr
	* ...\sprites\trigger_script.spr
	* ...\sprites\trigger_track_goal.spr
* Valve's Source SDK [link](https://developer.valvesoftware.com/wiki/Source_SDK)
	* ...\sprites\light_environment.spr
	* ...\sprites\light_spot.spr
* Avacado Maelstrom (original content)
	* ...\sprites\ms_monsterspawn.spr
	* ...\sprites\msitem_spawn.spr
	* ...\sprites\mstrig_stopspawn.spr
	* ...\sprites\mstrig_weather.spr
* MS:C community (original content)
	* ...\sprites\ms_counter.spr
	* ...\sprites\mstrig_music.spr
	* ...\sprites\mstrig_resetplayerspeed.spr
	* ...\sprites\mstrig_setplayerspeed.spr