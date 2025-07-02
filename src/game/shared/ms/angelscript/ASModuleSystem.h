//==========================================================================
// ASModuleSystem.h
// 
// AngelScript Module Management System
// Handles module loading, dependencies, imports/exports, and management
//==========================================================================

#ifndef AS_MODULE_SYSTEM_H
#define AS_MODULE_SYSTEM_H

#include <angelscript.h>
#include <string>
#include <map>
#include <vector>
#include <set>

// Forward declarations
class CGameGroupFile;
class CScriptModule;

//==========================================================================
// Module dependency information
//==========================================================================
struct ASModuleDependency
{
    std::string moduleName;
    std::string version;    // Optional version requirement
    bool isOptional;        // If true, module can load without this dependency
    
    ASModuleDependency() : isOptional(false) {}
    ASModuleDependency(const std::string& name, const std::string& ver = "", bool opt = false)
        : moduleName(name), version(ver), isOptional(opt) {}
};

//==========================================================================
// Module metadata
//==========================================================================
struct ASModuleInfo
{
    std::string name;
    std::string version;
    std::string description;
    std::vector<ASModuleDependency> dependencies;
    std::vector<std::string> exports;    // Exported functions/types
    asIScriptModule* pModule;
    int refCount;
    bool isBuiltin;    // True for system modules that can't be unloaded
    
    ASModuleInfo() : pModule(nullptr), refCount(0), isBuiltin(false) {}
};

//==========================================================================
// Module load options
//==========================================================================
struct ASModuleLoadOptions
{
    bool compileOnly;           // Only compile, don't build
    bool allowOverwrite;        // Allow overwriting existing module
    bool resolveDependencies;   // Automatically load dependencies
    bool validateExports;       // Check that all declared exports exist
    
    ASModuleLoadOptions() 
        : compileOnly(false)
        , allowOverwrite(false)
        , resolveDependencies(true)
        , validateExports(true) {}
};

//==========================================================================
// ASModuleSystem - Singleton module manager
//==========================================================================
class ASModuleSystem
{
private:
    // Module storage
    std::map<std::string, ASModuleInfo> m_Modules;
    
    // Dependency graph for topological sorting
    std::map<std::string, std::set<std::string>> m_DependencyGraph;
    
    // Module search paths
    std::vector<std::string> m_ModulePaths;
    
    // Engine reference
    asIScriptEngine* m_pEngine;
    
    // Statistics
    int m_nModulesLoaded;
    int m_nModulesCompiled;
    int m_nDependenciesResolved;
    
    // Module discovery system
    CScriptModule* m_pScriptModule;
    
    // Singleton instance
    static ASModuleSystem* s_pInstance;
    
    // Private constructor for singleton
    ASModuleSystem();
    ~ASModuleSystem();
    
public:
    // Singleton access
    static ASModuleSystem* Instance();
    static void Shutdown();
    
    // Initialization
    bool Initialize(asIScriptEngine* pEngine);
    void Destroy();
    
    // Module loading
    bool LoadModule(const std::string& filename, const ASModuleLoadOptions& options = ASModuleLoadOptions());
    bool LoadModuleFromMemory(const std::string& name, const std::string& content, const ASModuleLoadOptions& options = ASModuleLoadOptions());
    bool LoadModuleFromMemory(const std::string& name, const std::string& content, CGameGroupFile* pakFile, const ASModuleLoadOptions& options = ASModuleLoadOptions());
    
    // Module management
    bool UnloadModule(const std::string& name);
    asIScriptModule* GetModule(const std::string& name);
    bool HasModule(const std::string& name) const;
    bool ReloadModule(const std::string& name);
    
    // Module information
    const ASModuleInfo* GetModuleInfo(const std::string& name) const;
    std::vector<std::string> GetLoadedModules() const;
    std::vector<std::string> GetModuleDependents(const std::string& name) const;
    
    // Dependency management
    bool ResolveDependencies(const std::string& moduleName);
    std::vector<std::string> GetDependencyOrder() const;
    bool CheckCircularDependencies(const std::string& moduleName) const;
    
    // Import/Export system
    bool ImportModule(asIScriptModule* fromModule, const std::string& importModuleName, const std::string& importNamespace = "");
    bool ExportFunction(const std::string& moduleName, const std::string& functionDecl);
    bool ExportType(const std::string& moduleName, const std::string& typeName);
    
    // Module paths
    void AddModulePath(const std::string& path);
    void RemoveModulePath(const std::string& path);
    const std::vector<std::string>& GetModulePaths() const { return m_ModulePaths; }
    
    // Module discovery (new functionality)
    bool DiscoverModulesInPak(CGameGroupFile* pakFile);
    std::vector<std::string> GetDiscoveredModules() const;
    bool LoadDiscoveredModules(CGameGroupFile* pakFile);
    
    // Utility functions
    bool CompileAllModules();
    void PrintModuleStats() const;
    void PrintDependencyGraph() const;
    
private:
    // Internal helpers
    bool ParseModuleMetadata(const std::string& content, ASModuleInfo& info);
    bool BuildModule(ASModuleInfo& info);
    bool LoadDependency(const ASModuleDependency& dep);
    std::string FindModuleFile(const std::string& moduleName);
    bool TopologicalSort(std::vector<std::string>& sorted) const;
    void UpdateDependencyGraph(const std::string& moduleName, const std::vector<ASModuleDependency>& deps);
    bool HasCircularDependencyDFS(const std::string& current, std::set<std::string>& visited, std::set<std::string>& recursionStack) const;
    
    // Reference counting
    void AddReference(const std::string& moduleName);
    void ReleaseReference(const std::string& moduleName);
    
    // Prevent copying
    ASModuleSystem(const ASModuleSystem&) = delete;
    ASModuleSystem& operator=(const ASModuleSystem&) = delete;
};

//==========================================================================
// Global registration functions
//==========================================================================
namespace ASModuleSystemBindings
{
    // Register module management functions with AngelScript
    bool RegisterAll(asIScriptEngine* pEngine);
    
    // Individual registration functions
    bool RegisterModuleFunctions(asIScriptEngine* pEngine);
    bool RegisterImportExportFunctions(asIScriptEngine* pEngine);
}

//==========================================================================
// AngelScript global functions for module management
//==========================================================================
bool LoadModule(const std::string& filename);
bool UnloadModule(const std::string& name);
bool ReloadModule(const std::string& name);
bool HasModule(const std::string& name);
bool ImportModule(const std::string& moduleName);
bool ImportModule(const std::string& moduleName, const std::string& asNamespace);

#endif // AS_MODULE_SYSTEM_H