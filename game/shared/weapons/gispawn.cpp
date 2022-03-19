#include "msdllheaders.h"
#include "weapons/weapons.h"
#include "weapons/genericitem.h"

class CBaseGISpawn : public CBaseEntity
{
public:
	string_t sScriptFile;
	char cContainer[128];
	bool m_fSpawnOnTrigger;

	void Spawn();
	void SpawnItem();
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	bool PutItemInPack();
	void PutItemInPackThink();
	CBaseGISpawn::CBaseGISpawn() { cContainer[0] = 0; }
};

LINK_ENTITY_TO_CLASS(msitem_spawn, CBaseGISpawn);

void CBaseGISpawn ::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	SpawnItem();
}
void CBaseGISpawn::Spawn()
{
	if (!m_fSpawnOnTrigger)
	{
		SetThink(&CBaseGISpawn::SpawnItem);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CBaseGISpawn::SpawnItem()
{
	CGenericItem *pItem = NewGenericItem((char *)STRING(sScriptFile));
	if (!pItem)
		return;
	pItem->pev->targetname = pev->targetname;

	bool fIsInPack = PutItemInPack();

	if (cContainer[0] && !fIsInPack)
	{
		pItem->SUB_Remove();
		pItem = NULL;
		SetThink(&CBaseGISpawn::PutItemInPackThink);
		pev->nextthink = gpGlobals->time + 0.2;
		return;
	}

	if (!fIsInPack)
	{ //Normal world spawn
		pItem->pev->origin = pev->origin;
		pItem->pev->angles = pev->angles;
	}
}
void CBaseGISpawn::PutItemInPackThink()
{
	PutItemInPack();
	SetThink(NULL);
}
bool CBaseGISpawn::PutItemInPack()
{
	if (cContainer[0])
	{
		CGenericItem *pItem = NewGenericItem((char *)STRING(sScriptFile));
		if (!pItem)
			return false;
		CBaseEntity *pEntContainer = UTIL_FindEntityByTargetname(NULL, cContainer);

		if (pEntContainer && pEntContainer->IsMSItem())
		{
			CGenericItem *pContainer = (CGenericItem *)pEntContainer;
			if (pContainer->PackData)
			{
				pItem->PutInPack(pContainer);
				return true;
			}
		}
	}
	return false;
}
void CBaseGISpawn::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "scriptfile"))
	{
		sScriptFile = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "container"))
	{
		 strncpy(cContainer,  pkvd->szValue, sizeof(cContainer) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnstart"))
	{
		m_fSpawnOnTrigger = (atoi(pkvd->szValue)) ? true : false;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}
