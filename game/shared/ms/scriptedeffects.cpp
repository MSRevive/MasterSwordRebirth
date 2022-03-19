#include "msdllheaders.h"
#include "script.h"
#include "scriptedeffects.h"
#include "logfile.h"

//  ==============================================================
//								Global
//  ==============================================================
mslist<globalscripteffect_t> CGlobalScriptedEffects::Effects;

void CGlobalScriptedEffects::RegisterEffect(globalscripteffect_t &Effect)
{
	//logfile << "DEBUG: CGlobalScriptedEffects - Loading: " << Effect.m_ScriptName.c_str() << "\n";

	for (int i = 0; i < Effects.size(); i++)
	{
		if (!stricmp(Effects[i].m_Name, Effect.m_Name))
			return;
	} //Effect already exists

	Effects.add(Effect);
}

//Apply an effect
CScript *CGlobalScriptedEffects::ApplyEffect(msstring_ref ScriptName, IScripted *pScriptTarget, CBaseEntity *pTarget, msstringlist *Parameters)
{
	CScript *Script = pScriptTarget->Script_Add(ScriptName, pTarget);
	if (!Script)
	{
		MSErrorConsoleText("CGlobalScriptedEffects::ApplyEffect()", msstring("Scripted Effect '") + ScriptName + "' does not exist!'");
		return NULL;
	}

	Script->RunScriptEvents(); //Initialize 'game.effect.X'

#define EFFECT_ID "game.effect.id"
#define EFFECT_FLAGS "game.effect.flags"

	//Check if this effect is being stacked
	if (Script->VarExists(EFFECT_ID))
		for (int i = 0; i < pScriptTarget->m_Scripts.size(); i++)
			if (pScriptTarget->m_Scripts[i] != Script && FStrEq(pScriptTarget->m_Scripts[i]->GetVar(EFFECT_ID), Script->GetVar(EFFECT_ID)))
			{
				//Check if the effect doesn't allow stacking
				if (Script->VarExists(EFFECT_FLAGS))
					if (strstr(Script->GetVar(EFFECT_FLAGS), "nostack"))
					{
						Script->m.RemoveNextFrame = true; //The effect doesn't allow stacking, delete the effect script
						return NULL;
					}

				//The effect allows stacking, warn it that it is being stacked
				pScriptTarget->m_Scripts[i]->RunScriptEventByName("game_duplicated");
			}

	//if( Effect.m_Type == SCRIPTEFFECT_PLAYERACTION )
	//	Script->SetVar( "game.effect.updateplayer", 1 ); //Make sure the player gets an initial update

	Script->RunScriptEventByName("game_activate", Parameters); //A player action script shouldn't respond to this.. only scripts that get activated when applied
	return Script;
}

void CGlobalScriptedEffects::DeleteEffects()
{
	Effects.clear();
}
