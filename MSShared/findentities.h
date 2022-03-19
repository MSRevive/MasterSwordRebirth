// MiB - Utility functions for finding and filtering entities
#ifndef FINDENTITIES_H
#define FINDENTITIES_H

#include "../dlls/Half-Life/extdll.h"
#include "../engine/eiface.h"
#include "../dlls/Half-Life/util.h"
#include "iscript.h"

#ifndef VALVE_DLL
    #include "../cl_dll/hud_iface.h"
    #include "../cl_dll/wrect.h"
    #include "../engine/cdll_int.h"
    #include "../common/cl_entity.h"
    extern cl_enginefunc_t gEngfuncs;
#endif

// Wrapper class to make dealing with client/server
// differences slightly less annoying
class CFindEntity
{
public:

#ifdef VALVE_DLL
    bool IsMSItem();
    bool IsMSMonster();
    typedef CBaseEntity  YWrapType;
#else
    typedef cl_entity_t YWrapType;
#endif

    CFindEntity(YWrapType * pEntity = NULL);

    static CFindEntity GetEntity(int vIndx, bool & bValid);

    BOOL IsPlayer();
    Vector GetOrigin();
    int GetEntIndex();
    msstring AsString();
    Vector GetTraceLineSource();
    Vector GetAbsMin();
    Vector GetAbsMax();
    Vector Center();
    YWrapType * GetWrapped();

protected:
    YWrapType * mpEntity;
};

class CEntityFilter
{
public:
    virtual bool Allow(CFindEntity & vEntity) const = 0;
};

int UtilFindEntities(mslist<CFindEntity>& rFound, const mslist<CEntityFilter *>& vFilters, CFindEntity::YWrapType * pIgnoreEntity   = NULL, int vMax = 0);

class CScriptFilter : public CEntityFilter
{
protected:
    IScripted *mpScripted;
    msstring msEventName;
public:
    CScriptFilter(IScripted * pScripted, const msstring & vsEventName);
    virtual bool Allow(CFindEntity & vEntity);
};

class CTraceLineFilter : public CEntityFilter
{
protected:
    #ifdef VALVE_DLL
        CFindEntity::YWrapType * mpIgnoreEntity;
    #endif
    Vector mOrigin;
public:
    CTraceLineFilter(const Vector& vOrigin);
    #ifdef VALVE_DLL
        CTraceLineFilter(CFindEntity::YWrapType * pEntity);
        CTraceLineFilter(CFindEntity::YWrapType * pEntity, const Vector& vOrigin);
    #endif
    virtual bool Allow(CFindEntity & vEntity) const;
};

class CShapeFilter : public CEntityFilter
{
protected:
    CTraceLineFilter *mpTraceLine;
    Vector mOrigin;
    mutable msstring msAsString;
    virtual void ResetStringCache();
    virtual void CreateStringCache() const = 0;
public:
    CShapeFilter();
    virtual ~CShapeFilter();
    virtual void SetTraceLine(CTraceLineFilter * pTraceLine);
    virtual bool Allow(const Vector& vPoint) const = 0;
    virtual bool Allow(CFindEntity & vEntity) const;
    virtual const msstring& AsString();
    virtual const Vector& GetOrigin();
    virtual bool SetFromString(msstring vsString) = 0;
    static CShapeFilter *CreateFromString(const msstring & vsShape, bool bTraceLine = false, CBaseEntity * pIgnoreEntity = NULL);
};

class CRectangleFilter : public CShapeFilter
{
protected:
    float mXSize;
    float mYSize;
    float mZSize;

    mutable bool mbCalculatedPoints;
    mutable Vector mMinPoint;
    mutable Vector mMaxPoint;
    mutable bool mbCheckZ;
    virtual void CreateStringCache() const;
public:
    CRectangleFilter();
    CRectangleFilter(const Vector& vOrigin, float vXSize, float vYSize, float vZSize  = 0);
    virtual bool Allow(const Vector& vPoint) const;
    virtual bool SetFromString(msstring vsString);
    virtual void CalcPoints() const;
};

class CSphereFilter : public CShapeFilter
{
protected:
    float mRadius;
    float mRadiusSquared; // Keep cached for efficiency
    virtual void CreateStringCache() const;
public:
    CSphereFilter();
    CSphereFilter(const Vector& vOrigin, float vRadius, float vRadiusSquared = -1);
    virtual bool Allow(const Vector& vPoint) const;
    virtual bool SetFromString(msstring vsString);
};

//class CConeFilter : public CShapeFilter
//{
//protected:
//    Vector                              mAngle;
//    virtual void CreateStringCache(
//        ) const;
//public:
//    CConeFilter(
//          const Vector &                vOrigin
//        , const Vector &                vAngle
//        );
//    virtual bool Allow(
//          const Vector&                 vPoint
//        ) const;
//    virtual bool SetFromString(
//          msstring                      vsString
//        );
//};

class CCylinderFilter : public CShapeFilter
{
protected:
    float mRadius;
    float mRadiusSquared; // Keep cached for efficiency
    float mPosZ;
    float mNegZ;
    bool mbCheckHeight;
    virtual void CreateStringCache() const;
public:
    CCylinderFilter();
    CCylinderFilter(Vector vOrigin, float vRadius, float vPosZ = 0, float vNegZ = 0);
    virtual bool Allow(const Vector& vPoint) const;
    virtual bool SetFromString(msstring vsString);
};

// Include non-shared filters
#ifdef VALVE_DLL
    #include "../dlls/serverfindentities.h"
#else
    #include "../cl_dll/clientfindentities.h"
#endif

#endif
