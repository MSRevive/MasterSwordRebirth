#include "findentities.h"
#include "cbase.h"
#include "monsters/msmonster.h"
#include "stats/races.h"

Vector CFindEntity::GetAbsMin(
)
{
    return GetWrapped()->pev->absmin;
}

Vector CFindEntity::GetAbsMax(
)
{
    return GetWrapped()->pev->absmax;
}

Vector CFindEntity::GetOrigin(
)
{
    return GetWrapped()->pev->origin;
}

BOOL CFindEntity::IsPlayer(
)
{
    return GetWrapped()->IsPlayer();
}

bool CFindEntity::IsMSMonster(
)
{
    return GetWrapped()->IsMSMonster();
}

bool CFindEntity::IsMSItem(
)
{
    return GetWrapped()->IsMSItem();
}

int CFindEntity::GetEntIndex(
)
{
    return GetWrapped()->entindex();
}

msstring CFindEntity::AsString(
)
{
    return EntToString(GetWrapped());
}


CFindEntity CFindEntity::GetEntity(
  int                                   vIndx
, bool &                                bValid
)
{
    edict_t *                           pEdict = g_engfuncs.pfnPEntityOfEntIndex( vIndx );
    CBaseEntity *                       pEntity = NULL;
    if ( pEdict && pEdict->pvPrivateData )
    {
        pEntity = CBaseEntity::Instance( pEdict );
    }

    CFindEntity                         vEntity( pEntity );
    bValid = pEntity != NULL;
    return vEntity;
}

CTraceLineFilter::CTraceLineFilter(
  CFindEntity::YWrapType *              pEntity
)
{
    mpIgnoreEntity = pEntity;
    mOrigin = pEntity->EyePosition();
}

CRelationFilter::CRelationFilter(
  CFindEntity::YWrapType *              pEntity
)
{
    mpEntity = pEntity;
}

bool CRelationFilter::Allow(
  CFindEntity &                         vEntity
) const
{
    if ( !vEntity.IsMSMonster() ) return false;

    return AllowRelation( ((CMSMonster*)vEntity.GetWrapped())->IRelationship( mpEntity ) );
}

CAllyFilter::CAllyFilter(
  CFindEntity::YWrapType *              pEntity
) : CRelationFilter( pEntity )
{
}

bool CAllyFilter::AllowRelation(
  int                                   vRelation
) const
{
    return vRelation == RELATIONSHIP_AL;
}

CEnemyFilter::CEnemyFilter(
  CFindEntity::YWrapType *              pEntity
) : CRelationFilter( pEntity )
{
}

bool CEnemyFilter::AllowRelation(
  int                                   vRelation
) const
{
    return vRelation == RELATIONSHIP_HT; // Seems to me this should allow anything <= -2, but going off existing code...
}

CEntityTypeFilter::CEntityTypeFilter(
  bool                                  bAllowNpc
, bool                                  bAllowPlayer
, bool                                  bAllowItem
)
{
    mbAllowNpc = bAllowNpc;
    mbAllowPlayer = bAllowPlayer;
    mbAllowItem = bAllowItem;
}
