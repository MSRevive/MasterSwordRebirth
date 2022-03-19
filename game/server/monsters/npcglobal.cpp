#include "inc_weapondefs.h"

// DynamicPrecache - Precaches monsters specified in the ms_dynamicnpc CVAR.
//					 Used for testing monster not on the map or dynamic quests
void CMSMonster::DynamicPrecache()
{
	char *pszString = (char *)CVAR_GET_STRING("ms_dynamicnpc");
	if (!pszString || !*pszString)
		return;

	msstringlist NPCs;

	TokenizeString(pszString, NPCs);

	for (int i = 0; i < NPCs.size(); i++)
	{
		msstring_ref ScriptFile = NPCs[i];

		CMSMonster *pMonster = (CMSMonster *)CREATE_ENT("ms_npc");
		if (!pMonster)
			continue;

		ALERT(at_console, "Loading extra npc: %s\n", ScriptFile);
		pMonster->Spawn(ScriptFile); //Spawns in precache mode (g_fInPrecache == true), so its immediately deleted
	}
}

/*#define NPC_SOUND_FILE "sound/npc_sounds.txt"
void CMSMonster::SpeechPrecache( )
{
    //Precaches monster speech segments listed in NPC_SOUND_FILE.

	byte *pMemFile, *pStringPtr;
	int iFileSize, ret;

	pMemFile = pStringPtr = LOAD_FILE_FOR_ME( NPC_SOUND_FILE, &iFileSize );
	if( !pMemFile ) {
		ALERT( at_console, "!!Missing file: %s\n", NPC_SOUND_FILE );
		return;
	}

//	ALERT( at_console, "Precaching sounds from file: %s...\n", NPC_SOUND_FILE );
	while( *pStringPtr ) {
		while ( *pStringPtr && (isspace(*pStringPtr)||*pStringPtr < 13) ) { // skip over any carriage returns
			pStringPtr++; continue; }
		if( !*pStringPtr ) return;
		//Weed out comments or empty lines
		char cBuf[64];
		ret = sscanf( (char *)pStringPtr, "%s", cBuf );
		if( ret != 1 || *pStringPtr < 13 ) break;
		pStringPtr += strlen( cBuf );

		if( !strstr(cBuf,"*") )
			//Precache needs perm memory so have the engine create it
			PRECACHE_SOUND( (char *)STRING(ALLOC_STRING((char *)cBuf)) );
	}

	FREE_FILE( pMemFile );
}*/