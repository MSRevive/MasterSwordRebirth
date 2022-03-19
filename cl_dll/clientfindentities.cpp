#include "../MSShared/findentities.h"

Vector CFindEntity::GetAbsMin()
{
    return GetWrapped()->curstate.mins;
}

Vector CFindEntity::GetAbsMax()
{
    return GetWrapped()->curstate.maxs;
}

Vector CFindEntity::GetOrigin()
{
    return GetWrapped()->origin;
}

BOOL CFindEntity::IsPlayer()
{
    return GetWrapped()->player;
}

int CFindEntity::GetEntIndex()
{
    return GetWrapped()->index;
}

msstring CFindEntity::AsString()
{
    char pszRet[16];
    _snprintf(pszRet, sizeof(pszRet), "%i", GetEntIndex());
    return msstring(pszRet);
}

Vector CFindEntity::GetTraceLineSource()
{
    return GetOrigin();
}

CEntityTypeFilter::CEntityTypeFilter(bool bAllowPlayer)
{
    mbAllowPlayer = bAllowPlayer;
}

CFindEntity CFindEntity::GetEntity(int vIndx, bool &bValid)
{
    cl_entity_s *pEntity = gEngfuncs.GetEntityByIndex( vIndx );
    CFindEntity vEntity( pEntity );
    bValid = pEntity != NULL;
    return vEntity;
}
