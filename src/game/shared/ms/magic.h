#ifndef MAGIC_H
#define MAGIC_H

//This is info for all the memorizable spells that exist
//In precache, each spell script puts itself on this list
//When the player chooses to prepare one, it spawns a GenericItem
//by scriptname and gives it to the player
//The GenericItem handles the casting, etc.
typedef msstringlist spellgroup_v;

class Magic
{
public:
#ifndef VALVE_DLL
	static spellgroup_v Spells;				   //The spell script names
	static void AddSpell(const char *pszName); //Spell script adding itself to the global list at precache time
#endif

	static bool Prepare(const char *pszName, class CBasePlayer *pPlayer); //Create the GenericItem for this spell and give it to the player
};

/*#ifdef VALVE_DLL
	class CMagicSpell {
	public:
			msstring Name;
			msstring ScriptName;

			//static vector<CMagicSpell *> GlobalSpells;			//The global spell list
		#endif

		typedef vector<msstring> Spells;
	};
#endif*/

#endif //MAGIC_H
