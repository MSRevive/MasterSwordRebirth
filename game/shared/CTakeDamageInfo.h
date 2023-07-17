#ifndef GAME_SHARED_CTAKEDAMAGEINFO_H
#define GAME_SHARED_CTAKEDAMAGEINFO_H

class CBaseEntity;

/**
*	Stores damage info.
*/
class CTakeDamageInfo final
{
public:
	CTakeDamageInfo() = default;
	CTakeDamageInfo( const CTakeDamageInfo& other ) = default;
	CTakeDamageInfo& operator=( const CTakeDamageInfo& other ) = default;

	/**
	*	Constructor.
	*	@param pInflictor The entity that is responsible for dealing the damage.
	*	@param pAttacker The entity that owns pInflictor. This may be identical to pInflictor if it directly attacked this entity.
	*	@param flDamage Amount of damage to deal.
	*	@param iBitsDamageTypes Bit vector of damage types.
	*/
	CTakeDamageInfo( CBaseEntity* pInflictor, CBaseEntity* pAttacker, const float flDamage, const int iBitsDamageTypes )
		: m_pInflictor( pInflictor )
		, m_pAttacker( pAttacker )
		, m_flDamage( flDamage )
		, m_iBitsDamageTypes( iBitsDamageTypes )
	{
	}

	/**
	*	Constructor. Inflictor is the same as the attacker.
	*	@param pAttacker The entity that initiated the attack.
	*	@param flDamage Amount of damage to deal.
	*	@param iBitsDamageTypes Bit vector of damage types.
	*/
	CTakeDamageInfo( CBaseEntity* pAttacker, const float flDamage, const int iBitsDamageTypes )
		: m_pInflictor( pAttacker )
		, m_pAttacker( pAttacker )
		, m_flDamage( flDamage )
		, m_iBitsDamageTypes( iBitsDamageTypes )
	{
	}

	/**
	*	@return The inflictor.
	*/
	CBaseEntity* GetInflictor() const { return m_pInflictor; }

	/**
	*	Sets the inflictor.
	*	@param pInflictor Inflictor.
	*/
	void SetInflictor( CBaseEntity* pInflictor )
	{
		m_pInflictor = pInflictor;
	}

	/**
	*	@return The attacker.
	*/
	CBaseEntity* GetAttacker() const { return m_pAttacker; }

	/**
	*	Sets the attacker.
	*	@param pAttacker Attacker.
	*/
	void SetAttacker( CBaseEntity* pAttacker )
	{
		m_pAttacker = pAttacker;
	}

	/**
	*	@return The damage amount.
	*/
	float GetDamage() const { return m_flDamage; }

	/**
	*	@return The damage amount, in a mutable form.
	*/
	float& GetMutableDamage() { return m_flDamage; }

	/**
	*	Sets the damage amount.
	*	@param flDamage Damage.
	*/
	void SetDamage( const float flDamage )
	{
		m_flDamage = flDamage;
	}

	/**
	*	@return The damage types bit vector.
	*/
	int GetDamageTypes() const { return m_iBitsDamageTypes; }

	/**
	*	@return The damage types bit vector, in a mutable form.
	*/
	int& GetMutableDamageTypes() { return m_iBitsDamageTypes; }

	/**
	*	Sets the damage types bit vector.
	*	@param iBitsDamageTypes Damage types.
	*/
	void SetDamageTypes( const int iBitsDamageTypes )
	{
		m_iBitsDamageTypes = iBitsDamageTypes;
	}

private:
	CBaseEntity* m_pInflictor = nullptr;
	CBaseEntity* m_pAttacker = nullptr;
	float m_flDamage = 0;
	int m_iBitsDamageTypes = 0;
};

#endif //GAME_SHARED_CTAKEDAMAGEINFO_H