// Technically in both, but they differ
class CEntityTypeFilter : public CEntityFilter
{
protected:
    bool                                mbAllowPlayer;
public:
    CEntityTypeFilter(
          bool                          bAllowPlayer
        );
    virtual bool Allow(
          CFindEntity &                 pEntity
        ) const;
};
