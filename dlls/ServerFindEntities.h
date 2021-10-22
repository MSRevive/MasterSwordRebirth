class CRelationFilter : public CEntityFilter
{
protected:
    CFindEntity::YWrapType *            mpEntity;
public:
    CRelationFilter(
          CFindEntity::YWrapType *      pEntity
        );
    virtual bool AllowRelation(
          int                           vRelation
        ) const = 0;
    virtual bool Allow(
          CFindEntity &                 vEntity
        ) const;
};

class CAllyFilter : public CRelationFilter
{
public:
    CAllyFilter(
          CFindEntity::YWrapType *      pEntity
        );
    virtual bool AllowRelation(
          int                           vRelation
        ) const;
};

class CEnemyFilter : public CRelationFilter
{
public:
    CEnemyFilter(
          CFindEntity::YWrapType *      pEntity
        );
    virtual bool AllowRelation(
          int                           vRelation
        ) const;
};

// Technically in both, but they differ
class CEntityTypeFilter : public CEntityFilter
{
protected:
    bool                                mbAllowNpc;
    bool                                mbAllowPlayer;
    bool                                mbAllowItem;
public:
    CEntityTypeFilter(
          bool                          bAllowNpc
        , bool                          bAllowPlayer
        , bool                          bAllowItem
        );
    virtual bool Allow(
          CFindEntity &                 vEntity
        ) const;
};
