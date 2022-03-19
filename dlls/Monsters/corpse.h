#define SKELETON_MODEL "models/monsters/skeleton.mdl"

class CCorpse : public CMSMonster
{
public:
	void CreateCorpse(CMSMonster *pSource, float LoseGoldPercent = 100.0);

	//Overridden
	int ObjectCaps(void) { return FCAP_DONT_SAVE; }
	int MSProperties() { return MS_CORPSE; }
	void Spawn();
	bool AddItem(CGenericItem *pItem, bool ToHand, bool CheckWeight, int ForceHand = -1);
	bool RemoveItem(CGenericItem *pItem);
	BOOL HasHumanGibs() { return TRUE; }
	BOOL HasAlienGibs() { return FALSE; }
	int BloodColor(void) { return BLOOD_COLOR_RED; }
	bool ShouldGibMonster(int iGib) { return true; }
	int Classify() { return CLASS_NONE; }
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
	float DamageForce(float damage);
	void Deactivate();
};
