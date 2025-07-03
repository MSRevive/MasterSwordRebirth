#ifndef ASTEST_H
#define ASTEST_H

//==========================================================================
// ASTest - Simple test functions for AngelScript integration
//==========================================================================
namespace ASTest
{
    // Test basic AngelScript functionality
    bool TestBasicScript();
    
    // Test memory tracking
    bool TestMemoryTracking();
    
    // Test compilation and execution
    bool TestScriptExecution();
    
    // Run all tests
    bool RunAllTests();
}

#endif // ASTEST_H