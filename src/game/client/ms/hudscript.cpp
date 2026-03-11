/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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
//
// HUDScript.cpp - Controls script events that are called by other players
//
#include "../inc_huditem.h"

//Player functionality
#include "inc_weapondefs.h"
#include "script.h"
#include "pm_defs.h"
#include "cl_entity.h"
#include "event_api.h"
#include "shake.h"
#include "ref_params.h"
#include "clglobal.h"
#include "scriptedeffects.h"
#include "hudscript.h"

extern physent_t *MSUTIL_EntityByIndex( int playerindex );


static void SCRIPT_CONTROLVEC_POS(CScript* Script, const char* name, Vector& vec) {

	const char* ofsX = "_ofs.x";
	const char* ofsY = "_ofs.y";
	const char* ofsZ = "_ofs.z";
	const char* setX = "_set.x";
	const char* setY = "_set.y";
	const char* setZ = "_set.z";

	std::string ScriptVarNameX = name;	ScriptVarNameX += ofsX;
	std::string ScriptVarNameY = name;	ScriptVarNameY += ofsY;
	std::string ScriptVarNameZ = name;	ScriptVarNameZ += ofsZ;

	std::string ScriptVarSetNameX = {name};	ScriptVarSetNameX += setX;
	std::string ScriptVarSetNameY = {name}; ScriptVarSetNameY += setY;
	std::string ScriptVarSetNameZ = {name};	ScriptVarSetNameZ += setZ;

	if (Script->VarExists(ScriptVarNameX.c_str())) vec.x += atof(Script->GetVar(ScriptVarNameX.c_str()));
	if (Script->VarExists(ScriptVarNameY.c_str())) vec.y += atof(Script->GetVar(ScriptVarNameY.c_str()));
	if (Script->VarExists(ScriptVarNameZ.c_str())) vec.z += atof(Script->GetVar(ScriptVarNameZ.c_str()));
	if (Script->VarExists(ScriptVarSetNameX.c_str())) vec.x = atof(Script->GetVar(ScriptVarNameX.c_str()));
	if (Script->VarExists(ScriptVarSetNameY.c_str())) vec.y = atof(Script->GetVar(ScriptVarNameY.c_str()));
	if (Script->VarExists(ScriptVarSetNameZ.c_str())) vec.z = atof(Script->GetVar(ScriptVarNameZ.c_str()));


};

static void SCRIPT_CONTROLVEC_ANG(CScript* Script, const char* name, Vector& vec) {

	const char* ofsPitch = "_ofs.pitch";
	const char* ofsYaw = "_ofs.yaw";
	const char* ofsRoll = "_ofs.roll";
	const char* setPitch = "_set.pitch";
	const char* setYaw = "_set.yaw";
	const char* setRoll = "_set.roll";

	std::string ScriptVarPitch = { name };	ScriptVarPitch += ofsPitch;
	std::string ScriptVarYaw = {name};		ScriptVarYaw += ofsYaw;
	std::string ScriptVarRoll = {name};		ScriptVarRoll += ofsRoll;

	std::string ScriptVarSetPitch = {name};	ScriptVarSetPitch += setPitch;
	std::string ScriptVarSetYaw = {name};	ScriptVarSetYaw += setYaw;
	std::string ScriptVarSetRoll = {name};	ScriptVarSetRoll += setRoll;

	if (Script->VarExists(ScriptVarPitch.c_str())) vec.x += atof(Script->GetVar(ScriptVarPitch.c_str()));
	if (Script->VarExists(ScriptVarYaw.c_str()))   vec.y += atof(Script->GetVar(ScriptVarYaw.c_str()));
	if (Script->VarExists(ScriptVarRoll.c_str()))  vec.z += atof(Script->GetVar(ScriptVarRoll.c_str()));
	if (Script->VarExists(ScriptVarSetPitch.c_str())) vec.x = atof(Script->GetVar(ScriptVarSetPitch.c_str()));
	if (Script->VarExists(ScriptVarSetYaw.c_str()))   vec.y = atof(Script->GetVar(ScriptVarSetYaw.c_str()));
	if (Script->VarExists(ScriptVarSetRoll.c_str()))  vec.z = atof(Script->GetVar(ScriptVarSetRoll.c_str()));
};

static void SCRIPT_CONTROLVEC_POS_CUSTOM(CScript* Script, const char* name, Vector& vec, const char* xname, const char* yname, const char* zname) {
	
	const char* ofs = "_ofs.";
	const char* set = "_set.";

	std::string ScriptVarCustomXOffset{ name }; ScriptVarCustomXOffset += ofs; ScriptVarCustomXOffset += xname;
	std::string ScriptVarCustomYOffset{ name }; ScriptVarCustomYOffset += ofs; ScriptVarCustomYOffset += yname;
	std::string ScriptVarCustomZOffset{ name }; ScriptVarCustomZOffset += ofs; ScriptVarCustomZOffset += zname;

	std::string ScriptVarCustomXSet{ name }; ScriptVarCustomXSet += set; ScriptVarCustomXSet += xname;
	std::string ScriptVarCustomYSet{ name }; ScriptVarCustomYSet += set; ScriptVarCustomYSet += yname;
	std::string ScriptVarCustomZSet{ name }; ScriptVarCustomZSet += set; ScriptVarCustomZSet += zname;

	if (Script->VarExists(ScriptVarCustomXOffset.c_str())) vec.x += atof(Script->GetVar(ScriptVarCustomXOffset.c_str())); 
	if (Script->VarExists(ScriptVarCustomYOffset.c_str())) vec.y += atof(Script->GetVar(ScriptVarCustomYOffset.c_str()));
	if (Script->VarExists(ScriptVarCustomZOffset.c_str())) vec.z += atof(Script->GetVar(ScriptVarCustomZOffset.c_str()));
	if (Script->VarExists(ScriptVarCustomXSet.c_str())) vec.x = atof(Script->GetVar(ScriptVarCustomXSet.c_str()));
	if (Script->VarExists(ScriptVarCustomYSet.c_str())) vec.y = atof(Script->GetVar(ScriptVarCustomYSet.c_str()));
	if (Script->VarExists(ScriptVarCustomZSet.c_str())) vec.z = atof(Script->GetVar(ScriptVarCustomZSet.c_str()));

};
/*
#define SCRIPT_CONTROLVEC_POS( name, vec ) \
		if( Script->VarExists( name "_ofs.x" ) ) vec.x += atof(Script->GetVar( name "_ofs.x" )); \
		if( Script->VarExists( name "_ofs.y" ) ) vec.y += atof(Script->GetVar( name "_ofs.y" )); \
		if( Script->VarExists( name "_ofs.z" ) ) vec.z += atof(Script->GetVar( name "_ofs.z" )); \
		if( Script->VarExists( name "_set.x" ) ) vec.x  = atof(Script->GetVar( name "_ofs.x" )); \
		if( Script->VarExists( name "_set.y" ) ) vec.y  = atof(Script->GetVar( name "_ofs.y" )); \
		if( Script->VarExists( name "_set.z" ) ) vec.z  = atof(Script->GetVar( name "_ofs.z" ));

#define SCRIPT_CONTROLVEC_ANG( name, vec ) \
		if( Script->VarExists( name "_ofs.pitch" ) ) vec.x += atof(Script->GetVar( name "_ofs.pitch" )); \
		if( Script->VarExists( name "_ofs.yaw"	 ) ) vec.y += atof(Script->GetVar( name "_ofs.yaw"   )); \
		if( Script->VarExists( name "_ofs.roll"  ) ) vec.z += atof(Script->GetVar( name "_ofs.roll"  )); \
		if( Script->VarExists( name "_set.pitch" ) ) vec.x  = atof(Script->GetVar( name "_set.pitch" )); \
		if( Script->VarExists( name "_set.yaw"   ) ) vec.y  = atof(Script->GetVar( name "_set.yaw"   )); \
		if( Script->VarExists( name "_set.roll"  ) ) vec.z  = atof(Script->GetVar( name "_set.roll"  ));

#define SCRIPT_CONTROLVEC_POS_CUSTOM( name, vec, xname, yname, zname ) \
		if( Script->VarExists( name "_ofs." xname ) ) vec.x += atof(Script->GetVar( name "_ofs." xname )); \
		if( Script->VarExists( name "_ofs." yname ) ) vec.y += atof(Script->GetVar( name "_ofs." yname )); \
		if( Script->VarExists( name "_ofs." zname ) ) vec.z += atof(Script->GetVar( name "_ofs." zname )); \
		if( Script->VarExists( name "_set." xname ) ) vec.x  = atof(Script->GetVar( name "_set." xname )); \
		if( Script->VarExists( name "_set." yname ) ) vec.y  = atof(Script->GetVar( name "_set." yname )); \
		if( Script->VarExists( name "_set." zname ) ) vec.z  = atof(Script->GetVar( name "_set." zname ));
\
*/
//----------------


int CHudScript::Init( void )
{
	Reset();

	gHUD.AddHudElem( this );

	return 1;
}

int CHudScript::Draw( float flTime ) {
	return 1;
}

int CHudScript::VidInit( void ) { return 1; }

void CHudScript::Reset( void )
{
	m_iFlags |= HUD_ACTIVE;
}
void CHudScript::InitHUDData( void )
{
	 for (unsigned int i = 0; i < m_Scripts.size(); i++)
		Script_Remove( 0 );
}

// Think
void CHudScript::Think( )
{
	RunScriptEvents( );
}

//Receieved new client-side script
int CHudScript::MsgFunc_ClientScript( const char *pszName, int iSize, void *pbuf )
{
	byte Action = READ_BYTE( );
	ulong ID = READ_LONG( );
	msstringlist Parameters;

	if( !Action )	//Add Script
	{	
		msstring ScriptName = READ_STRING( );
		unsigned int iParameters = READ_BYTE( );
		for (unsigned int i = 0; i < iParameters; i++) Parameters.add(READ_STRING());
		CScript *Script = CreateScript( ScriptName, Parameters, true, ID );
	}
	else if( Action == 1 )	//Send Msg to Script
	{
		unsigned int iParameters = READ_BYTE( );
		msstring EventName = READ_STRING( );	//First parameter is the eventname
		for (unsigned int i = 0; i < (iParameters - 1); i++) Parameters.add(READ_STRING());

		 for (unsigned int i = 0; i < m_Scripts.size(); i++) 
		{
			CScript *Script = m_Scripts[i];
			if( Script->m.UniqueID != ID )
				continue;

			 for (unsigned int p = 0; p < Parameters.size(); p++) 
				Script->SetVar( msstring("PARAM") + ((int)p+1), Parameters[p].c_str() );
			Script->RunScriptEventByName( EventName, Parameters.size() ? &Parameters : NULL );
			break;
		}
	}
	else	//Remove script
	{
		 for (unsigned int i = 0; i < m_Scripts.size(); i++) 
		{
			if( m_Scripts[i]->m.UniqueID != ID )
				continue;

			Script_Remove( i );
			break;
		}
	}

	return 1;
}

CScript *CHudScript::CreateScript(const char* ScriptName, msstringlist &Parameters, bool AllowDupe, int UniqueID )
{
	//If I don't allow dupes, try to find a prev copy of this script
	if( !AllowDupe )
	{
		unsigned int events = m_Scripts.size();
		 for (unsigned int i = 0; i < events; i++) 
			if( strstr( m_Scripts[i]->m.ScriptFile.c_str(), ScriptName ) )
			{
				UniqueID = m_Scripts[i]->m.UniqueID;
				return m_Scripts[i];	//Found a prev copy.  Just return it and don't call initialization again
			}
	}

	//Create a new script if not latching onto a prev copy
	CScript *Script = Script_Add( ScriptName, &player );
	if( !Script ) return NULL;


	Script->m.pScriptedInterface = &player;
	Script->RunScriptEvents( );
	Script->RunScriptEventByName( "client_activate", &Parameters );
	Script->m.UniqueID = (UniqueID == -1) ? CScript::m_gLastSendID++ : UniqueID;
	return Script;
}
void CHudScript::HandleAnimEvent(const char* Options, const cl_entity_s *clEntity, hae_e Type )
{
	static msstringlist ParsedOptions;
	ParsedOptions.clearitems();

	TokenizeString( Options, ParsedOptions );
	if( ParsedOptions.size() < 2 ) 
		return;

	const char* ScriptName = ParsedOptions[0];
	const char* EventName = ParsedOptions[1];

	CScript *Script = NULL;

	//Latch onto an existing script
	if( Type == HAE_EITHER || Type == HAE_ATTACH )
	{
		unsigned int events = m_Scripts.size( );
		 for (unsigned int i = 0; i < events; i++) 
			if( strstr( m_Scripts[i]->m.ScriptFile.c_str(), ScriptName ) )
				{ Script = m_Scripts[i]; break; }
	}

	//Create a new script, or latch onto an existing copy
	if( (Type == HAE_NEW) || (Type == HAE_EITHER && !Script) )
	{
		static msstringlist DummyParameters;
		Script = CreateScript( ScriptName, DummyParameters, false );
	}

	if( !Script ) 
		return;

	static msstringlist Params;
	Params.clearitems( );
	 for (unsigned int i = 0; i < ParsedOptions.size()-2; i++) 
		Params.add( ParsedOptions[i+2] );

	Script->RunScriptEventByName( EventName, &Params );
}

void CHudScript::Effects_GetView( ref_params_s *pparams, cl_entity_t *ViewModel )
{
	 for (unsigned int i = 0; i < m_Scripts.size(); i++) 
	{
		CScript *Script = m_Scripts[i];
		Vector &ViewOfs = *(Vector *)&pparams->vieworg;
		Vector &ViewAng = *(Vector *)&pparams->viewangles;
		Vector &ViewMdlOfs = *(Vector *)&ViewModel->origin;
		Vector &ViewMdlAng = *(Vector *)&ViewModel->angles;
		SCRIPT_CONTROLVEC_POS(Script, "game.cleffect.view", ViewOfs );
		SCRIPT_CONTROLVEC_ANG(Script, "game.cleffect.view", ViewAng );
		SCRIPT_CONTROLVEC_POS(Script, "game.cleffect.viewmodel", ViewMdlOfs );
		SCRIPT_CONTROLVEC_ANG(Script, "game.cleffect.viewmodel", ViewMdlAng );
	}
}
Vector CHudScript::Effects_GetMoveScale( )
{
	Vector NewScale( 1.0f, 1.0f, 1.0f );
	 for (unsigned int i = 0; i < m_Scripts.size(); i++) 
	{
		CScript *Script = m_Scripts[i];
		if( Script->VarExists( "game.cleffect.move_scale.forward" ) ) NewScale.x *= atof(Script->GetVar( "game.cleffect.move_scale.forward" ));
		if( Script->VarExists( "game.cleffect.move_scale.right"   ) ) NewScale.y *= atof(Script->GetVar( "game.cleffect.move_scale.right" ));
		if( Script->VarExists( "game.cleffect.move_scale.up"      ) ) NewScale.z *= atof(Script->GetVar( "game.cleffect.move_scale.up" ));
	}

	return NewScale;
}
Vector CHudScript::Effects_GetMove( Vector &OriginalMove )
{
	Vector NewMove = OriginalMove;
	 for (unsigned int i = 0; i < m_Scripts.size(); i++) 
	{
		CScript *Script = m_Scripts[i];
		NewMove.x += atof(Script->GetVar( "game.cleffect.move_ofs.forward" ));
		NewMove.y += atof(Script->GetVar( "game.cleffect.move_ofs.right" ));
		NewMove.z += atof(Script->GetVar( "game.cleffect.move_ofs.up" ));
	}

	return NewMove;
}

void CHudScript::Effects_GetFade( screenfade_t &ScreenFade )
{
	float OldScreenAlpha = ScreenFade.fadealpha;
	ScreenFade.fadeFlags = 0;
	 for (unsigned int i = 0; i < m_Scripts.size(); i++) 
	{
		CScript *Script = m_Scripts[i];
		if( !atoi(Script->GetVar( "game.cleffect.screenfade.newfade" )) )
			continue;

		if( Script->VarExists( "game.cleffect.screenfade.alphalimit" ) &&
			(OldScreenAlpha <= atoi(Script->GetVar( "game.cleffect.screenfade.alphalimit" ))) )
		{
			ScreenFade.fadealpha = atoi(Script->GetVar( "game.cleffect.screenfade.alpha" ));
			Vector Color = StringToVec( Script->GetVar( "game.cleffect.screenfade.color" ) );
			ScreenFade.fader = Color.x;
			ScreenFade.fadeg = Color.y;
			ScreenFade.fadeb = Color.z;
			float Duration = atof(Script->GetVar( "game.cleffect.screenfade.duration" ));
			ScreenFade.fadeSpeed = ScreenFade.fadealpha / (Duration ? Duration : ScreenFade.fadealpha);//atof(Script->GetVar( "game.cleffect.screenfade.speed" ));
			ScreenFade.fadeEnd = gpGlobals->time + Duration;
			ScreenFade.fadeReset = ScreenFade.fadeEnd - atof(Script->GetVar( "game.cleffect.screenfade.blendduration" ));

			msstring FadeFlags = Script->GetVar( "game.cleffect.screenfade.type" );
			if( FadeFlags.find( "fadein" ) != msstring_error ) SetBits( ScreenFade.fadeFlags, FFADE_IN );
			if( FadeFlags.find( "fadeout" ) != msstring_error ) SetBits( ScreenFade.fadeFlags, FFADE_OUT );
			if( FadeFlags.find( "noblend" ) != msstring_error ) SetBits( ScreenFade.fadeFlags, FFADE_MODULATE );
			if( FadeFlags.find( "perm" ) != msstring_error ) SetBits( ScreenFade.fadeFlags, FFADE_STAYOUT );
		}

		Script->SetVar( "game.cleffect.screenfade.newfade", 0 );
	}
}
void CHudScript::Effects_PreRender( )
{
	 for (unsigned int i = 0; i < m_Scripts.size(); i++) 
		if( m_Scripts[i]->m.m_HandleRender )
			m_Scripts[i]->RunScriptEventByName( "game_prerender" );
}

void CHudScript::Effects_Render( cl_entity_t &Ent, bool InMirror )
{
	 for (unsigned int i = 0; i < m_Scripts.size(); i++) 
		if( m_Scripts[i]->m.m_HandleRender )
		{
			static msstringlist Params;
			Params.clearitems( );
			Params.add( UTIL_VarArgs("%i",Ent.index) );				//Index of entity being rendered
			Params.add( InMirror ? "1" : "0" );						//Rendering in a mirror
			m_Scripts[i]->RunScriptEventByName( "game_render", &Params );
		}
}

void CHudScript::Effects_DrawTransPararentTriangles( )
{
	 for (unsigned int i = 0; i < m_Scripts.size(); i++) 
		if( m_Scripts[i]->m.m_HandleRender )
			m_Scripts[i]->RunScriptEventByName( "game_render_transparent" );
}
