//==========================================================================
// test_base_entity.h
// 
// Mock CBaseEntity for testing AngelScript system
// In the real implementation, this would be the actual game entity class
//==========================================================================

#ifndef TEST_BASE_ENTITY_H
#define TEST_BASE_ENTITY_H

// Minimal CBaseEntity mock for testing
class CBaseEntity
{
public:
    CBaseEntity() {}
    virtual ~CBaseEntity() {}
    
    virtual const char* GetClassname() { return "base_entity"; }
    
    // Add other entity methods as needed for testing
};

#endif // TEST_BASE_ENTITY_H