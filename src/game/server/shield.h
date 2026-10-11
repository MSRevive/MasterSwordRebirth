
class CShieldArea : public CBaseEntity
{
public:
	CShieldArea *pNext;
	int MSProperties() { return MS_SHIELD; }
	void Spawn();
	float TraceAttack(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int iAccuracyRoll);
	void FollowThink();
	void CounterEffect(CBaseEntity *pInflictor, int iEffect, void *pExtraData = NULL);
};
