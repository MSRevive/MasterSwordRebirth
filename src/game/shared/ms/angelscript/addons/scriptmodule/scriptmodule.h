//==============================================================================
// scriptmodule.h
//
// AngelScript Module System Addon
// Provides 'module' keyword support and automatic module discovery
//==============================================================================

#ifndef SCRIPTMODULE_H
#define SCRIPTMODULE_H

#ifndef SCRIPTPACK_TOOL
#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already included before
#include <angelscript.h>
#endif
#endif

#include <string>
#include <vector>
#include <map>

#ifdef SCRIPTPACK_TOOL
// Standalone mode for scriptpack tool
#define BEGIN_AS_NAMESPACE
#define END_AS_NAMESPACE
#else
BEGIN_AS_NAMESPACE
#endif

// Forward declarations
class CScriptModule;
class CGameGroupFile;

// Module discovery information
struct ModuleInfo
{
    std::string name;           // Module name extracted from 'module ModuleName {'
    std::string filePath;       // Path within PAK file
    std::string source;         // Original source code
    std::string processed;      // Processed source code (module -> class)
    bool hasMainClass;         // Whether the module has a main class matching module name
    std::string namespaceName;  // Namespace containing the module (empty if global)

    ModuleInfo() : hasMainClass(false) {}
    ModuleInfo(const std::string& moduleName, const std::string& path)
        : name(moduleName), filePath(path), hasMainClass(false) {}
};

// Module preprocessing and discovery system
class CScriptModule
{
public:
    CScriptModule();
    ~CScriptModule();
    
    // Module discovery
    bool DiscoverModulesInPak(CGameGroupFile* pakFile);
    bool DiscoverModulesInDirectory(const std::string& directory);
    
    // Get discovered modules
    const std::vector<ModuleInfo>& GetDiscoveredModules() const { return m_discoveredModules; }
    ModuleInfo* GetModuleInfo(const std::string& moduleName);
    bool HasModule(const std::string& moduleName) const;
    
    // Module preprocessing
    bool PreprocessModuleSource(const std::string& source, std::string& output, std::string& moduleName, std::string& namespaceName);
    bool ProcessModuleFile(const std::string& filePath, const std::string& source);
    
    // Clear discovered modules
    void ClearModules();
    
    // Debug and statistics
    void PrintDiscoveredModules() const;
    size_t GetModuleCount() const { return m_discoveredModules.size(); }
    
private:
    // Internal helpers
    bool ParseModuleDeclaration(const std::string& source, std::string& moduleName, size_t& moduleStart, std::string& namespaceName);
    bool ValidateModuleStructure(const std::string& source, size_t moduleStart, bool& hasMainClass);
    std::string TransformModuleToClass(const std::string& source, const std::string& moduleName, size_t moduleStart);
    std::string GenerateModuleWrapper(const std::string& moduleName, const std::string& namespaceName);
    
    // Utility functions
    std::string TrimWhitespace(const std::string& str) const;
    bool IsValidIdentifier(const std::string& identifier) const;
    
    // Enhanced parsing functions (AngelScript-style)
    bool IsKeywordAt(const char* source, size_t sourceLength, size_t pos, const char* keyword, size_t& tokenLength) const;
    bool IsIdentifierAt(const char* source, size_t sourceLength, size_t pos, size_t& tokenLength) const;
    size_t SkipWhitespaceAndComments(const char* source, size_t sourceLength, size_t pos) const;
    bool IsWordBoundary(const char* source, size_t pos, size_t sourceLength) const;
    
    // Storage
    std::vector<ModuleInfo> m_discoveredModules;
    std::map<std::string, size_t> m_moduleNameMap; // Name -> index in m_discoveredModules
};

#ifdef SCRIPTPACK_TOOL
// Standalone mode
#else
END_AS_NAMESPACE
#endif

#endif // SCRIPTMODULE_H