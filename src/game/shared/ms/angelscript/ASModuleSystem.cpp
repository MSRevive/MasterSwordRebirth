//==========================================================================
// ASModuleSystem.cpp
// 
// AngelScript Module Management System Implementation
//==========================================================================

#include "ASModuleSystem.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

// Singleton instance
ASModuleSystem* ASModuleSystem::s_pInstance = nullptr;

//==========================================================================
// Constructor/Destructor
//==========================================================================
ASModuleSystem::ASModuleSystem()
    : m_pEngine(nullptr)
    , m_nModulesLoaded(0)
    , m_nModulesCompiled(0)
    , m_nDependenciesResolved(0)
{
    // Add default module search paths
    m_ModulePaths.push_back("scripts/modules/");
    m_ModulePaths.push_back("scripts/");
}

ASModuleSystem::~ASModuleSystem()
{
    Destroy();
}

//==========================================================================
// Singleton Management
//==========================================================================
ASModuleSystem* ASModuleSystem::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new ASModuleSystem();
    }
    return s_pInstance;
}

void ASModuleSystem::Shutdown()
{
    if (s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

//==========================================================================
// Initialization
//==========================================================================
bool ASModuleSystem::Initialize(asIScriptEngine* pEngine)
{
    if (!pEngine)
    {
        printf("ASModuleSystem::Initialize: ERROR - NULL engine pointer\n");
        return false;
    }
    
    m_pEngine = pEngine;
    
    printf("ASModuleSystem initialized with %d module search paths\n", (int)m_ModulePaths.size());
    
    return true;
}

void ASModuleSystem::Destroy()
{
    // Unload all modules in reverse dependency order
    std::vector<std::string> unloadOrder;
    if (TopologicalSort(unloadOrder))
    {
        // Reverse the order for unloading
        std::reverse(unloadOrder.begin(), unloadOrder.end());
        
        for (const std::string& moduleName : unloadOrder)
        {
            UnloadModule(moduleName);
        }
    }
    
    m_Modules.clear();
    m_DependencyGraph.clear();
    m_pEngine = nullptr;
}

//==========================================================================
// Module Loading
//==========================================================================
bool ASModuleSystem::LoadModule(const std::string& filename, const ASModuleLoadOptions& options)
{
    if (!m_pEngine)
    {
        printf("ASModuleSystem::LoadModule: ERROR - Not initialized\n");
        return false;
    }
    
    // Find the module file
    std::string fullPath = FindModuleFile(filename);
    if (fullPath.empty())
    {
        printf("ASModuleSystem::LoadModule: ERROR - Module file not found: %s\n", filename.c_str());
        return false;
    }
    
    // Read the file
    std::ifstream file(fullPath);
    if (!file.is_open())
    {
        printf("ASModuleSystem::LoadModule: ERROR - Failed to open file: %s\n", fullPath.c_str());
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    // Extract module name from metadata or filename
    ASModuleInfo info;
    if (!ParseModuleMetadata(content, info))
    {
        // Use filename as module name if no metadata
        size_t lastSlash = filename.find_last_of("/\\");
        size_t lastDot = filename.find_last_of(".");
        if (lastDot != std::string::npos)
        {
            info.name = filename.substr(lastSlash + 1, lastDot - lastSlash - 1);
        }
        else
        {
            info.name = filename.substr(lastSlash + 1);
        }
    }
    
    return LoadModuleFromMemory(info.name, content, options);
}

bool ASModuleSystem::LoadModuleFromMemory(const std::string& name, const std::string& content, const ASModuleLoadOptions& options)
{
    if (!m_pEngine)
    {
        printf("ASModuleSystem::LoadModuleFromMemory: ERROR - Not initialized\n");
        return false;
    }
    
    // Check if module already exists
    if (HasModule(name) && !options.allowOverwrite)
    {
        printf("ASModuleSystem::LoadModuleFromMemory: ERROR - Module already loaded: %s\n", name.c_str());
        return false;
    }
    
    // Parse module metadata
    ASModuleInfo info;
    info.name = name;
    ParseModuleMetadata(content, info);
    
    // Resolve dependencies if requested
    if (options.resolveDependencies)
    {
        for (const ASModuleDependency& dep : info.dependencies)
        {
            if (!HasModule(dep.moduleName))
            {
                if (!LoadDependency(dep) && !dep.isOptional)
                {
                    printf("ASModuleSystem::LoadModuleFromMemory: ERROR - Failed to load dependency: %s\n", dep.moduleName.c_str());
                    return false;
                }
            }
        }
    }
    
    // Create or get the module
    info.pModule = m_pEngine->GetModule(name.c_str(), asGM_CREATE_IF_NOT_EXISTS);
    if (!info.pModule)
    {
        printf("ASModuleSystem::LoadModuleFromMemory: ERROR - Failed to create module: %s\n", name.c_str());
        return false;
    }
    
    // Add the script section
    int r = info.pModule->AddScriptSection(name.c_str(), content.c_str(), content.length());
    if (r < 0)
    {
        printf("ASModuleSystem::LoadModuleFromMemory: ERROR - Failed to add script section for module: %s\n", name.c_str());
        return false;
    }
    
    // Build the module if not compile-only
    if (!options.compileOnly)
    {
        if (!BuildModule(info))
        {
            return false;
        }
    }
    else
    {
        m_nModulesCompiled++;
    }
    
    // Update module registry
    m_Modules[name] = info;
    UpdateDependencyGraph(name, info.dependencies);
    m_nModulesLoaded++;
    
    printf("ASModuleSystem: Module '%s' loaded successfully\n", name.c_str());
    
    return true;
}

//==========================================================================
// Module Management
//==========================================================================
bool ASModuleSystem::UnloadModule(const std::string& name)
{
    if (!HasModule(name))
    {
        printf("ASModuleSystem::UnloadModule: WARNING - Module not found: %s\n", name.c_str());
        return false;
    }
    
    ASModuleInfo& info = m_Modules[name];
    
    // Check if module is builtin
    if (info.isBuiltin)
    {
        printf("ASModuleSystem::UnloadModule: ERROR - Cannot unload builtin module: %s\n", name.c_str());
        return false;
    }
    
    // Check reference count
    if (info.refCount > 0)
    {
        printf("ASModuleSystem::UnloadModule: ERROR - Module still has %d references: %s\n", info.refCount, name.c_str());
        return false;
    }
    
    // Check dependents
    std::vector<std::string> dependents = GetModuleDependents(name);
    if (!dependents.empty())
    {
        printf("ASModuleSystem::UnloadModule: ERROR - Module has dependents: %s\n", name.c_str());
        for (const std::string& dep : dependents)
        {
            printf("  - %s\n", dep.c_str());
        }
        return false;
    }
    
    // Discard the module
    if (info.pModule)
    {
        info.pModule->Discard();
    }
    
    // Remove from registry
    m_Modules.erase(name);
    m_DependencyGraph.erase(name);
    m_nModulesLoaded--;
    
    printf("ASModuleSystem: Module '%s' unloaded\n", name.c_str());
    
    return true;
}

asIScriptModule* ASModuleSystem::GetModule(const std::string& name)
{
    auto it = m_Modules.find(name);
    if (it != m_Modules.end())
    {
        return it->second.pModule;
    }
    return nullptr;
}

bool ASModuleSystem::HasModule(const std::string& name) const
{
    return m_Modules.find(name) != m_Modules.end();
}

bool ASModuleSystem::ReloadModule(const std::string& name)
{
    if (!HasModule(name))
    {
        printf("ASModuleSystem::ReloadModule: ERROR - Module not found: %s\n", name.c_str());
        return false;
    }
    
    // Save module info
    ASModuleInfo oldInfo = m_Modules[name];
    
    // Unload the module
    if (!UnloadModule(name))
    {
        return false;
    }
    
    // Try to find and reload the module
    std::string modulePath = FindModuleFile(name + ".as");
    if (modulePath.empty())
    {
        printf("ASModuleSystem::ReloadModule: ERROR - Module file not found for reload: %s\n", name.c_str());
        return false;
    }
    
    ASModuleLoadOptions options;
    options.allowOverwrite = true;
    options.resolveDependencies = true;
    
    return LoadModule(modulePath, options);
}

//==========================================================================
// Module Information
//==========================================================================
const ASModuleInfo* ASModuleSystem::GetModuleInfo(const std::string& name) const
{
    auto it = m_Modules.find(name);
    if (it != m_Modules.end())
    {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> ASModuleSystem::GetLoadedModules() const
{
    std::vector<std::string> modules;
    for (const auto& pair : m_Modules)
    {
        modules.push_back(pair.first);
    }
    return modules;
}

std::vector<std::string> ASModuleSystem::GetModuleDependents(const std::string& name) const
{
    std::vector<std::string> dependents;
    
    for (const auto& pair : m_Modules)
    {
        const ASModuleInfo& info = pair.second;
        for (const ASModuleDependency& dep : info.dependencies)
        {
            if (dep.moduleName == name)
            {
                dependents.push_back(pair.first);
                break;
            }
        }
    }
    
    return dependents;
}

//==========================================================================
// Dependency Management
//==========================================================================
bool ASModuleSystem::ResolveDependencies(const std::string& moduleName)
{
    if (!HasModule(moduleName))
    {
        printf("ASModuleSystem::ResolveDependencies: ERROR - Module not found: %s\n", moduleName.c_str());
        return false;
    }
    
    const ASModuleInfo& info = m_Modules[moduleName];
    
    for (const ASModuleDependency& dep : info.dependencies)
    {
        if (!HasModule(dep.moduleName))
        {
            if (!LoadDependency(dep))
            {
                if (!dep.isOptional)
                {
                    printf("ASModuleSystem::ResolveDependencies: ERROR - Failed to resolve dependency: %s\n", dep.moduleName.c_str());
                    return false;
                }
                else
                {
                    printf("ASModuleSystem::ResolveDependencies: WARNING - Optional dependency not found: %s\n", dep.moduleName.c_str());
                }
            }
        }
        
        m_nDependenciesResolved++;
    }
    
    return true;
}

std::vector<std::string> ASModuleSystem::GetDependencyOrder() const
{
    std::vector<std::string> order;
    TopologicalSort(order);
    return order;
}

bool ASModuleSystem::CheckCircularDependencies(const std::string& moduleName) const
{
    std::set<std::string> visited;
    std::set<std::string> recursionStack;
    
    return HasCircularDependencyDFS(moduleName, visited, recursionStack);
}

//==========================================================================
// Import/Export System
//==========================================================================
bool ASModuleSystem::ImportModule(asIScriptModule* fromModule, const std::string& importModuleName, const std::string& importNamespace)
{
    if (!fromModule || !m_pEngine)
    {
        printf("ASModuleSystem::ImportModule: ERROR - Invalid parameters\n");
        return false;
    }
    
    asIScriptModule* importModule = GetModule(importModuleName);
    if (!importModule)
    {
        printf("ASModuleSystem::ImportModule: ERROR - Import module not found: %s\n", importModuleName.c_str());
        return false;
    }
    
    // Get module info for exports
    const ASModuleInfo* info = GetModuleInfo(importModuleName);
    if (!info)
    {
        return false;
    }
    
    // Import functions
    for (const std::string& exportDecl : info->exports)
    {
        asIScriptFunction* func = importModule->GetFunctionByDecl(exportDecl.c_str());
        if (func)
        {
            // Note: In real implementation, would need to properly bind imported functions
            // This is a simplified version
            printf("ASModuleSystem::ImportModule: Imported function: %s\n", exportDecl.c_str());
        }
    }
    
    // Update reference count
    AddReference(importModuleName);
    
    return true;
}

bool ASModuleSystem::ExportFunction(const std::string& moduleName, const std::string& functionDecl)
{
    auto it = m_Modules.find(moduleName);
    if (it == m_Modules.end())
    {
        printf("ASModuleSystem::ExportFunction: ERROR - Module not found: %s\n", moduleName.c_str());
        return false;
    }
    
    it->second.exports.push_back(functionDecl);
    return true;
}

bool ASModuleSystem::ExportType(const std::string& moduleName, const std::string& typeName)
{
    auto it = m_Modules.find(moduleName);
    if (it == m_Modules.end())
    {
        printf("ASModuleSystem::ExportType: ERROR - Module not found: %s\n", moduleName.c_str());
        return false;
    }
    
    it->second.exports.push_back("type:" + typeName);
    return true;
}

//==========================================================================
// Module Paths
//==========================================================================
void ASModuleSystem::AddModulePath(const std::string& path)
{
    // Ensure path ends with separator
    std::string normalizedPath = path;
    if (!normalizedPath.empty() && normalizedPath.back() != '/' && normalizedPath.back() != '\\')
    {
        normalizedPath += '/';
    }
    
    // Check if already exists
    auto it = std::find(m_ModulePaths.begin(), m_ModulePaths.end(), normalizedPath);
    if (it == m_ModulePaths.end())
    {
        m_ModulePaths.push_back(normalizedPath);
    }
}

void ASModuleSystem::RemoveModulePath(const std::string& path)
{
    auto it = std::find(m_ModulePaths.begin(), m_ModulePaths.end(), path);
    if (it != m_ModulePaths.end())
    {
        m_ModulePaths.erase(it);
    }
}

//==========================================================================
// Utility Functions
//==========================================================================
bool ASModuleSystem::CompileAllModules()
{
    bool success = true;
    
    // Get dependency order
    std::vector<std::string> buildOrder;
    if (!TopologicalSort(buildOrder))
    {
        printf("ASModuleSystem::CompileAllModules: ERROR - Circular dependencies detected\n");
        return false;
    }
    
    // Build modules in dependency order
    for (const std::string& moduleName : buildOrder)
    {
        ASModuleInfo& info = m_Modules[moduleName];
        if (!BuildModule(info))
        {
            printf("ASModuleSystem::CompileAllModules: ERROR - Failed to build module: %s\n", moduleName.c_str());
            success = false;
        }
    }
    
    return success;
}

void ASModuleSystem::PrintModuleStats() const
{
    printf("\n=== Module System Statistics ===\n");
    printf("Modules loaded: %d\n", m_nModulesLoaded);
    printf("Modules compiled: %d\n", m_nModulesCompiled);
    printf("Dependencies resolved: %d\n", m_nDependenciesResolved);
    printf("Module search paths: %d\n", (int)m_ModulePaths.size());
    
    printf("\nLoaded modules:\n");
    for (const auto& pair : m_Modules)
    {
        const ASModuleInfo& info = pair.second;
        printf("  - %s (v%s) [Refs: %d, Deps: %d, Exports: %d]\n",
               info.name.c_str(),
               info.version.empty() ? "1.0" : info.version.c_str(),
               info.refCount,
               (int)info.dependencies.size(),
               (int)info.exports.size());
    }
    printf("================================\n");
}

void ASModuleSystem::PrintDependencyGraph() const
{
    printf("\n=== Module Dependency Graph ===\n");
    
    for (const auto& pair : m_DependencyGraph)
    {
        printf("%s depends on:\n", pair.first.c_str());
        for (const std::string& dep : pair.second)
        {
            printf("  -> %s\n", dep.c_str());
        }
    }
    
    printf("\nDependency order:\n");
    std::vector<std::string> order = GetDependencyOrder();
    for (size_t i = 0; i < order.size(); i++)
    {
        printf("  %d. %s\n", (int)(i + 1), order[i].c_str());
    }
    
    printf("===============================\n");
}

//==========================================================================
// Internal Helpers
//==========================================================================
bool ASModuleSystem::ParseModuleMetadata(const std::string& content, ASModuleInfo& info)
{
    // Simple metadata parser looking for comments at the top of the file
    // Format:
    // // @module ModuleName
    // // @version 1.0
    // // @description Module description
    // // @depends OtherModule
    // // @export FunctionName
    
    std::istringstream stream(content);
    std::string line;
    bool foundMetadata = false;
    
    while (std::getline(stream, line))
    {
        // Trim whitespace
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        
        // Check for metadata comments
        if (line.substr(start, 2) == "//")
        {
            size_t atPos = line.find('@', start + 2);
            if (atPos != std::string::npos)
            {
                foundMetadata = true;
                
                size_t keyStart = atPos + 1;
                size_t keyEnd = line.find(' ', keyStart);
                if (keyEnd == std::string::npos) continue;
                
                std::string key = line.substr(keyStart, keyEnd - keyStart);
                std::string value = line.substr(keyEnd + 1);
                
                // Trim value
                size_t valueStart = value.find_first_not_of(" \t");
                size_t valueEnd = value.find_last_not_of(" \t\r\n");
                if (valueStart != std::string::npos && valueEnd != std::string::npos)
                {
                    value = value.substr(valueStart, valueEnd - valueStart + 1);
                }
                
                // Process metadata
                if (key == "module")
                {
                    info.name = value;
                }
                else if (key == "version")
                {
                    info.version = value;
                }
                else if (key == "description")
                {
                    info.description = value;
                }
                else if (key == "depends")
                {
                    // Parse dependency (format: "ModuleName [version] [optional]")
                    std::istringstream depStream(value);
                    std::string depName;
                    depStream >> depName;
                    
                    ASModuleDependency dep(depName);
                    
                    std::string next;
                    while (depStream >> next)
                    {
                        if (next == "optional")
                        {
                            dep.isOptional = true;
                        }
                        else
                        {
                            dep.version = next;
                        }
                    }
                    
                    info.dependencies.push_back(dep);
                }
                else if (key == "export")
                {
                    info.exports.push_back(value);
                }
            }
        }
        else if (foundMetadata)
        {
            // End of metadata section
            break;
        }
    }
    
    return foundMetadata;
}

bool ASModuleSystem::BuildModule(ASModuleInfo& info)
{
    if (!info.pModule)
    {
        printf("ASModuleSystem::BuildModule: ERROR - NULL module pointer\n");
        return false;
    }
    
    int r = info.pModule->Build();
    if (r < 0)
    {
        printf("ASModuleSystem::BuildModule: ERROR - Failed to build module: %s\n", info.name.c_str());
        
        // Module build failed
        printf("  Module compilation failed\n");
        
        return false;
    }
    
    m_nModulesCompiled++;
    return true;
}

bool ASModuleSystem::LoadDependency(const ASModuleDependency& dep)
{
    // Try to find the dependency module file
    std::string moduleFile = FindModuleFile(dep.moduleName + ".as");
    if (moduleFile.empty())
    {
        printf("ASModuleSystem::LoadDependency: Module file not found: %s\n", dep.moduleName.c_str());
        return false;
    }
    
    ASModuleLoadOptions options;
    options.resolveDependencies = true;
    
    return LoadModule(moduleFile, options);
}

std::string ASModuleSystem::FindModuleFile(const std::string& moduleName)
{
    // Check each search path
    for (const std::string& path : m_ModulePaths)
    {
        std::string fullPath = path + moduleName;
        
        // Try to open the file
        std::ifstream file(fullPath);
        if (file.good())
        {
            file.close();
            return fullPath;
        }
        
        // Try with .as extension if not already present
        if (moduleName.find(".as") == std::string::npos)
        {
            fullPath = path + moduleName + ".as";
            file.open(fullPath);
            if (file.good())
            {
                file.close();
                return fullPath;
            }
        }
    }
    
    return "";
}

bool ASModuleSystem::TopologicalSort(std::vector<std::string>& sorted) const
{
    std::map<std::string, int> inDegree;
    std::vector<std::string> queue;
    
    // Initialize in-degrees
    for (const auto& pair : m_Modules)
    {
        inDegree[pair.first] = 0;
    }
    
    // Calculate in-degrees
    for (const auto& pair : m_DependencyGraph)
    {
        for (const std::string& dep : pair.second)
        {
            inDegree[dep]++;
        }
    }
    
    // Find nodes with no dependencies
    for (const auto& pair : inDegree)
    {
        if (pair.second == 0)
        {
            queue.push_back(pair.first);
        }
    }
    
    // Process queue
    sorted.clear();
    while (!queue.empty())
    {
        std::string current = queue.back();
        queue.pop_back();
        sorted.push_back(current);
        
        // Reduce in-degree for dependents
        auto it = m_DependencyGraph.find(current);
        if (it != m_DependencyGraph.end())
        {
            for (const std::string& dep : it->second)
            {
                inDegree[dep]--;
                if (inDegree[dep] == 0)
                {
                    queue.push_back(dep);
                }
            }
        }
    }
    
    // Check if all modules were processed (no cycles)
    return sorted.size() == m_Modules.size();
}

void ASModuleSystem::UpdateDependencyGraph(const std::string& moduleName, const std::vector<ASModuleDependency>& deps)
{
    m_DependencyGraph[moduleName].clear();
    
    for (const ASModuleDependency& dep : deps)
    {
        m_DependencyGraph[moduleName].insert(dep.moduleName);
    }
}

bool ASModuleSystem::HasCircularDependencyDFS(const std::string& current, std::set<std::string>& visited, std::set<std::string>& recursionStack) const
{
    visited.insert(current);
    recursionStack.insert(current);
    
    auto it = m_DependencyGraph.find(current);
    if (it != m_DependencyGraph.end())
    {
        for (const std::string& dep : it->second)
        {
            if (recursionStack.find(dep) != recursionStack.end())
            {
                return true; // Circular dependency found
            }
            
            if (visited.find(dep) == visited.end())
            {
                if (HasCircularDependencyDFS(dep, visited, recursionStack))
                {
                    return true;
                }
            }
        }
    }
    
    recursionStack.erase(current);
    return false;
}

void ASModuleSystem::AddReference(const std::string& moduleName)
{
    auto it = m_Modules.find(moduleName);
    if (it != m_Modules.end())
    {
        it->second.refCount++;
    }
}

void ASModuleSystem::ReleaseReference(const std::string& moduleName)
{
    auto it = m_Modules.find(moduleName);
    if (it != m_Modules.end() && it->second.refCount > 0)
    {
        it->second.refCount--;
    }
}

//==========================================================================
// Global Registration Functions
//==========================================================================
namespace ASModuleSystemBindings
{
    bool RegisterAll(asIScriptEngine* pEngine)
    {
        if (!pEngine)
        {
            printf("ASModuleSystem::RegisterAll: ERROR - NULL engine pointer\n");
            return false;
        }
        
        printf("\n=== ASModuleSystem: Registering Module Management System ===\n");
        
        bool success = true;
        
        // Initialize the module system singleton
        ASModuleSystem* pModuleSystem = ASModuleSystem::Instance();
        if (!pModuleSystem->Initialize(pEngine))
        {
            printf("   ERROR: Module system initialization failed!\n");
            success = false;
        }
        
        // Register module functions
        printf("[1/2] Registering module management functions...\n");
        if (!RegisterModuleFunctions(pEngine))
        {
            printf("   ERROR: Module function registration failed!\n");
            success = false;
        }
        else
        {
            printf("   ✓ Module functions registered successfully\n");
        }
        
        // Register import/export functions
        printf("[2/2] Registering import/export functions...\n");
        if (!RegisterImportExportFunctions(pEngine))
        {
            printf("   ERROR: Import/export function registration failed!\n");
            success = false;
        }
        else
        {
            printf("   ✓ Import/export functions registered successfully\n");
        }
        
        if (success)
        {
            printf("=== ASModuleSystem: Registration Complete ===\n\n");
        }
        else
        {
            printf("=== ASModuleSystem: Registration FAILED ===\n\n");
        }
        
        return success;
    }
    
    bool RegisterModuleFunctions(asIScriptEngine* pEngine)
    {
        int r;
        
        // Register module loading functions
        r = pEngine->RegisterGlobalFunction("bool LoadModule(const string &in)", asFUNCTION(::LoadModule), asCALL_CDECL);
        if (r < 0) return false;
        
        r = pEngine->RegisterGlobalFunction("bool UnloadModule(const string &in)", asFUNCTION(::UnloadModule), asCALL_CDECL);
        if (r < 0) return false;
        
        r = pEngine->RegisterGlobalFunction("bool ReloadModule(const string &in)", asFUNCTION(::ReloadModule), asCALL_CDECL);
        if (r < 0) return false;
        
        r = pEngine->RegisterGlobalFunction("bool HasModule(const string &in)", asFUNCTION(::HasModule), asCALL_CDECL);
        if (r < 0) return false;
        
        return true;
    }
    
    bool RegisterImportExportFunctions(asIScriptEngine* pEngine)
    {
        int r;
        
        // Register import functions
        r = pEngine->RegisterGlobalFunction("bool ImportModule(const string &in)", asFUNCTION(static_cast<bool(*)(const std::string&)>(::ImportModule)), asCALL_CDECL);
        if (r < 0) return false;
        
        r = pEngine->RegisterGlobalFunction("bool ImportModule(const string &in, const string &in)", asFUNCTION(static_cast<bool(*)(const std::string&, const std::string&)>(::ImportModule)), asCALL_CDECL);
        if (r < 0) return false;
        
        return true;
    }
}

//==========================================================================
// AngelScript Global Functions Implementation
//==========================================================================
bool LoadModule(const std::string& filename)
{
    ASModuleSystem* pSystem = ASModuleSystem::Instance();
    if (!pSystem)
    {
        printf("LoadModule: ERROR - Module system not initialized\n");
        return false;
    }
    
    return pSystem->LoadModule(filename);
}

bool UnloadModule(const std::string& name)
{
    ASModuleSystem* pSystem = ASModuleSystem::Instance();
    if (!pSystem)
    {
        printf("UnloadModule: ERROR - Module system not initialized\n");
        return false;
    }
    
    return pSystem->UnloadModule(name);
}

bool ReloadModule(const std::string& name)
{
    ASModuleSystem* pSystem = ASModuleSystem::Instance();
    if (!pSystem)
    {
        printf("ReloadModule: ERROR - Module system not initialized\n");
        return false;
    }
    
    return pSystem->ReloadModule(name);
}

bool HasModule(const std::string& name)
{
    ASModuleSystem* pSystem = ASModuleSystem::Instance();
    if (!pSystem)
    {
        printf("HasModule: ERROR - Module system not initialized\n");
        return false;
    }
    
    return pSystem->HasModule(name);
}

bool ImportModule(const std::string& moduleName)
{
    // Note: In a real implementation, this would need to get the current module context
    printf("ImportModule: Importing module '%s' into current context\n", moduleName.c_str());
    
    ASModuleSystem* pSystem = ASModuleSystem::Instance();
    if (!pSystem)
    {
        printf("ImportModule: ERROR - Module system not initialized\n");
        return false;
    }
    
    return pSystem->HasModule(moduleName);
}

bool ImportModule(const std::string& moduleName, const std::string& asNamespace)
{
    // Note: In a real implementation, this would need to get the current module context
    printf("ImportModule: Importing module '%s' into namespace '%s'\n", moduleName.c_str(), asNamespace.c_str());
    
    ASModuleSystem* pSystem = ASModuleSystem::Instance();
    if (!pSystem)
    {
        printf("ImportModule: ERROR - Module system not initialized\n");
        return false;
    }
    
    return pSystem->HasModule(moduleName);
}