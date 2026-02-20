//==========================================================================
// ASModuleSystem.cpp
// 
// AngelScript Module Management System Implementation
//==========================================================================

#include "ASModuleSystem.h"
#include "ASEngineEventManager.h"  // For unregistering event handlers during reload
#include "addons/scriptbuilder/scriptbuilder.h"
#include "addons/scriptmodule/scriptmodule.h"
#include "groupfile.h"
#include "scriptmgr.h"
#include "mslogger.h"  // For MS_INFO and MS_ERROR macros
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

// Platform-specific includes for sleep functionality
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Singleton instance
ASModuleSystem* ASModuleSystem::s_pInstance = nullptr;

//==========================================================================
// Pak File Include Callback for CScriptBuilder
//==========================================================================
int PakFileIncludeCallback(const char *include, const char *from, CScriptBuilder *builder, void *userParam)
{
    if (!include || !builder || !userParam)
    {
        printf("PakFileIncludeCallback: ERROR - Invalid parameters\n");
        return -1;
    }
    
    //CGameGroupFile* groupFile = static_cast<CGameGroupFile*>(userParam);
    
    // Build the full path for the include file within the angelscript directory
    std::string includePath = "angelscript/";
    includePath += include;
    
    printf("PakFileIncludeCallback: Loading include '%s' from pak file\n", includePath.c_str());
    
    // First check if the file exists and get its size
    unsigned long fileSize;
    if (!g_ScriptPack.ReadEntry(includePath.c_str(), nullptr, fileSize))
    {
        printf("PakFileIncludeCallback: ERROR - Include file not found in pak: %s\n", includePath.c_str());
        return -1;
    }
    
    // Allocate buffer and read the file content
    char* scriptContent = new char[fileSize + 1];
    if (!g_ScriptPack.ReadEntry(includePath.c_str(), (unsigned char*)scriptContent, fileSize))
    {
        printf("PakFileIncludeCallback: ERROR - Failed to read include file from pak: %s\n", includePath.c_str());
        delete[] scriptContent;
        return -1;
    }
    scriptContent[fileSize] = '\0';
    
    // Add the script section to the builder
    int result = builder->AddSectionFromMemory(include, scriptContent, fileSize);
    
    // Clean up allocated memory
    delete[] scriptContent;
    
    if (result < 0)
    {
        printf("PakFileIncludeCallback: ERROR - Failed to add script section: %s\n", include);
        return result;
    }
    
    printf("PakFileIncludeCallback: Successfully loaded include '%s' (%lu bytes)\n", include, fileSize);
    return result;
}

//==========================================================================
// Pragma Callback for CScriptBuilder
//==========================================================================
int PragmaCallback(const std::string &pragmaText, CScriptBuilder &builder, void *userParam)
{
    // Trim leading/trailing whitespace from pragma text
    std::string text = pragmaText;
    size_t start = text.find_first_not_of(" \t\r\n");
    size_t end = text.find_last_not_of(" \t\r\n");
    
    if (start == std::string::npos)
    {
        // Empty pragma
        return -1;
    }
    
    text = text.substr(start, end - start + 1);
    
    // Parse the pragma directive
    // Expected format: "context server|client|shared"
    if (text.find("context") == 0)
    {
        // Extract the context type
        size_t contextStart = text.find_first_not_of(" \t", 7); // Skip "context"
        if (contextStart == std::string::npos)
        {
            printf("PragmaCallback: ERROR - Missing context type in #pragma context\n");
            return -1;
        }
        
        std::string contextType = text.substr(contextStart);
        // Trim any trailing whitespace from context type
        size_t contextEnd = contextType.find_first_of(" \t\r\n");
        if (contextEnd != std::string::npos)
        {
            contextType = contextType.substr(0, contextEnd);
        }
        
        // Validate context type
        if (contextType != "server" && contextType != "client" && contextType != "shared")
        {
            printf("PragmaCallback: ERROR - Invalid context type '%s'. Must be 'server', 'client', or 'shared'\n", contextType.c_str());
            return -1;
        }
        
        printf("PragmaCallback: Recognized #pragma context %s (pragma will be stripped from code)\n", contextType.c_str());
        
        // Return 0 to indicate success
        // The script builder will automatically overwrite the pragma with spaces
        return 0;
    }
    
    // Unknown pragma directive
    printf("PragmaCallback: WARNING - Unknown pragma directive: %s\n", text.c_str());
    return -1;
}

//==========================================================================
// Constructor/Destructor
//==========================================================================
ASModuleSystem::ASModuleSystem()
    : m_pEngine(nullptr)
    , m_nModulesLoaded(0)
    , m_nModulesCompiled(0)
    , m_nDependenciesResolved(0)
    , m_pScriptModule(nullptr)
{
    // Add default module search paths
    m_ModulePaths.push_back("scripts/modules/");
    m_ModulePaths.push_back("scripts/");
    
    // Initialize script module discovery system
    m_pScriptModule = new CScriptModule();
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
    
    // Clean up script module system
    if (m_pScriptModule)
    {
        delete m_pScriptModule;
        m_pScriptModule = nullptr;
    }
    
    m_pEngine = nullptr;
}

//==========================================================================
// Module Loading
//==========================================================================
bool ASModuleSystem::LoadASModule(const std::string& filename, const ASModuleLoadOptions& options)
{
    if (!m_pEngine)
    {
        printf("ASModuleSystem::LoadASModule: ERROR - Not initialized\n");
        return false;
    }
    
    // Find the module file
    std::string fullPath = FindModuleFile(filename);
    if (fullPath.empty())
    {
        printf("ASModuleSystem::LoadASModule: ERROR - Module file not found: %s\n", filename.c_str());
        return false;
    }
    
    // Read the file
    std::ifstream file(fullPath);
    if (!file.is_open())
    {
        printf("ASModuleSystem::LoadASModule: ERROR - Failed to open file: %s\n", fullPath.c_str());
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

bool ASModuleSystem::LoadModuleFromMemory(const std::string& name, const std::string& content, CGameGroupFile* pakFile, const ASModuleLoadOptions& options)
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
    
    // Determine script context from content and name
    ASScriptContextManager* pContextMgr = ASScriptContextManager::Instance();
    ScriptContext contextFromContent = pContextMgr->DetermineContextFromContent(content);
    ScriptContext contextFromPath = pContextMgr->DetermineContextFromPath(name);
    
    // Content pragma takes precedence over path-based detection
    if (contextFromContent != ScriptContext::UNKNOWN)
    {
        info.context = contextFromContent;
    }
    else if (contextFromPath != ScriptContext::UNKNOWN)
    {
        info.context = contextFromPath;
    }
    else
    {
        // Default to SHARED if no context specified
        info.context = ScriptContext::SHARED;
        printf("ASModuleSystem: WARNING - No context specified for module '%s', defaulting to SHARED\n", name.c_str());
    }
    
    // Check if module is compatible with current build
    if (!ASScriptContextUtil::CanRunInCurrentContext(info.context))
    {
        printf("ASModuleSystem::LoadModuleFromMemory: ERROR - Module '%s' context (%s) is not compatible with current build (%s)\n",
            name.c_str(),
            ASScriptContextUtil::ContextToString(info.context),
            ASScriptContextUtil::ContextToString(ASScriptContextUtil::GetCurrentBuildContext()));
        return false;
    }
    
    printf("ASModuleSystem: Loading module '%s' with context: %s\n", 
        name.c_str(), 
        ASScriptContextUtil::ContextToString(info.context));
    
    // Register the script context
    pContextMgr->SetScriptContext(name, info.context);
    
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
    
    // Use CScriptBuilder to handle #include directives and preprocessing
    CScriptBuilder builder;
    
    // Set up pak file include callback if pakFile is provided
    if (pakFile)
    {
        builder.SetIncludeCallback(PakFileIncludeCallback, pakFile);
        MS_INFO("ASModuleSystem: Pak file include callback configured for module '%s'", name.c_str());
    }
    else
    {
        MS_INFO("ASModuleSystem: No pak file provided - #include directives will not be supported");
    }

    // Set up pragma callback to handle #pragma context directives
    builder.SetPragmaCallback(PragmaCallback, nullptr);
    MS_INFO("ASModuleSystem: Pragma callback configured for module '%s'", name.c_str());
    
    MS_INFO("ASModuleSystem: Starting new module '%s'...", name.c_str());
    int r = builder.StartNewModule(m_pEngine, name.c_str());
    if (r < 0)
    {
        MS_ERROR("ASModuleSystem::LoadModuleFromMemory: ERROR - Failed to start new module: %s (error code: %d)", name.c_str(), r);
        return false;
    }
    MS_INFO("ASModuleSystem: Successfully started module '%s'", name.c_str());

    // Add the script content to the builder
    MS_INFO("ASModuleSystem: Adding script section for module '%s' (%d bytes)...", name.c_str(), (int)content.length());
    r = builder.AddSectionFromMemory(name.c_str(), content.c_str(), content.length());
    if (r < 0)
    {
        MS_ERROR("ASModuleSystem::LoadModuleFromMemory: ERROR - Failed to add script section for module: %s (error code: %d)", name.c_str(), r);
        return false;
    }
    MS_INFO("ASModuleSystem: Successfully added script section for module '%s'", name.c_str());

    // Build the module if not compile-only (CScriptBuilder handles this)
    if (!options.compileOnly)
    {
        MS_INFO("ASModuleSystem: Building module '%s'...", name.c_str());
        r = builder.BuildModule();
        if (r < 0)
        {
            MS_ERROR("ASModuleSystem::LoadModuleFromMemory: ERROR - Failed to build module: %s (error code: %d)", name.c_str(), r);
            MS_ERROR("ASModuleSystem: Check for [COMPILATION ERROR] messages above for details");
            return false;
        }
        MS_INFO("ASModuleSystem: Successfully built module '%s'", name.c_str());

        
        // Get the module from the builder
        info.pModule = builder.GetModule();
    }
    else
    {
        // Get the module for compile-only mode
        info.pModule = builder.GetModule(); 
        m_nModulesCompiled++;
    }
    
    // Update module registry
    m_Modules[name] = info;
    UpdateDependencyGraph(name, info.dependencies);
    m_nModulesLoaded++;
    
    printf("ASModuleSystem: Module '%s' loaded successfully with pak file support\n", name.c_str());
    
    return true;
}

bool ASModuleSystem::LoadModuleFromMemory(const std::string& name, const std::string& content, const ASModuleLoadOptions& options)
{
    // Delegate to the pak-aware version with no pak file (traditional loading)
    return LoadModuleFromMemory(name, content, nullptr, options);
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
    
    return LoadASModule(modulePath, options);
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
    
    return LoadASModule(moduleFile, options);
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
// Module Discovery (New Functionality)
//==========================================================================
bool ASModuleSystem::DiscoverModulesInPak(CGameGroupFile* pakFile)
{
    if (!m_pScriptModule)
    {
        printf("ASModuleSystem::DiscoverModulesInPak: ERROR - Script module system not initialized\n");
        return false;
    }
    
    if (!pakFile)
    {
        printf("ASModuleSystem::DiscoverModulesInPak: ERROR - NULL pak file\n");
        return false;
    }
    
    printf("ASModuleSystem: Starting module discovery in PAK file...\n");
    
    // Clear any previously discovered modules
    m_pScriptModule->ClearModules();
    
    // Use the script module system to discover modules
    bool result = m_pScriptModule->DiscoverModulesInPak(pakFile);
    
    if (result)
    {
        printf("ASModuleSystem: Module discovery completed successfully\n");
        m_pScriptModule->PrintDiscoveredModules();
    }
    else
    {
        printf("ASModuleSystem: Module discovery failed\n");
    }
    
    return result;
}

std::vector<std::string> ASModuleSystem::GetDiscoveredModules() const
{
    std::vector<std::string> moduleNames;
    
    if (m_pScriptModule)
    {
        const std::vector<ModuleInfo>& discoveredModules = m_pScriptModule->GetDiscoveredModules();
        
        for (const ModuleInfo& moduleInfo : discoveredModules)
        {
            moduleNames.push_back(moduleInfo.name);
        }
    }
    
    return moduleNames;
}

bool ASModuleSystem::LoadDiscoveredModules(CGameGroupFile* pakFile)
{
    if (!m_pScriptModule)
    {
        MS_ERROR("ASModuleSystem::LoadDiscoveredModules: ERROR - Script module system not initialized");
        return false;
    }

    if (!pakFile)
    {
        MS_ERROR("ASModuleSystem::LoadDiscoveredModules: ERROR - NULL pak file");
        return false;
    }
    
    const std::vector<ModuleInfo>& discoveredModules = m_pScriptModule->GetDiscoveredModules();
    
    if (discoveredModules.empty())
    {
        MS_INFO("ASModuleSystem::LoadDiscoveredModules: No modules discovered. Run DiscoverModulesInPak first.");
        return false;
    }

    MS_INFO("ASModuleSystem: Loading %d discovered modules...", (int)discoveredModules.size());

    bool allSuccess = true;
    std::vector<std::string> failedModules;  // Track which modules failed

    // Load each discovered module
    for (const ModuleInfo& moduleInfo : discoveredModules)
    {
        // First, determine the module's context from its ORIGINAL content (not processed)
        // The #pragma context directive must be checked before module->class transformation
        ASScriptContextManager* pContextMgr = ASScriptContextManager::Instance();
        
        // Debug: Check if source is available
        if (moduleInfo.source.empty())
        {
            MS_INFO("ASModuleSystem: WARNING - Module '%s' has empty source! Using file path for context detection",
                   moduleInfo.name.c_str());
        }
        else
        {
            // Debug: Show first 100 chars of source to verify pragma is present
            size_t previewLen = (std::min)(moduleInfo.source.length(), static_cast<size_t>(100));
            std::string preview = moduleInfo.source.substr(0, previewLen);
            MS_INFO("ASModuleSystem: Module '%s' source preview: %s...", moduleInfo.name.c_str(), preview.c_str());
        }
        
        ScriptContext moduleContext = pContextMgr->DetermineContextFromContent(moduleInfo.source);

        MS_INFO("ASModuleSystem: Module '%s' context from content: %s",
               moduleInfo.name.c_str(),
               ASScriptContextUtil::ContextToString(moduleContext));

        // If no context pragma found, try to determine from file path
        if (moduleContext == ScriptContext::UNKNOWN)
        {
            moduleContext = pContextMgr->DetermineContextFromPath(moduleInfo.filePath);
            MS_INFO("ASModuleSystem: Module '%s' context from path: %s",
                   moduleInfo.name.c_str(),
                   ASScriptContextUtil::ContextToString(moduleContext));
        }

        // Default to SHARED if still unknown
        if (moduleContext == ScriptContext::UNKNOWN)
        {
            MS_INFO("ASModuleSystem: Module '%s' defaulting to SHARED context", moduleInfo.name.c_str());
            moduleContext = ScriptContext::SHARED;
        }

        MS_INFO("ASModuleSystem: Module '%s' final context: %s, current build: %s",
               moduleInfo.name.c_str(),
               ASScriptContextUtil::ContextToString(moduleContext),
               ASScriptContextUtil::IsClientBuild() ? "CLIENT" : "SERVER");
        
        // Check if this module can run in the current build context
        if (!ASScriptContextUtil::CanRunInCurrentContext(moduleContext))
        {
            MS_INFO("ASModuleSystem: SKIPPING module '%s' - context %s not compatible with %s build",
                   moduleInfo.name.c_str(),
                   ASScriptContextUtil::ContextToString(moduleContext),
                   ASScriptContextUtil::IsClientBuild() ? "CLIENT" : "SERVER");
            continue;  // Skip this module
        }

        MS_INFO("ASModuleSystem: Loading module '%s' from %s (context: %s)",
               moduleInfo.name.c_str(),
               moduleInfo.filePath.c_str(),
               ASScriptContextUtil::ContextToString(moduleContext));
        
        // Use the processed source (module -> class transformation)
        ASModuleLoadOptions options;
        options.allowOverwrite = true;
        options.resolveDependencies = true;
        
        if (LoadModuleFromMemory(moduleInfo.name, moduleInfo.processed, pakFile, options))
        {
            MS_INFO("ASModuleSystem: Successfully loaded module '%s'", moduleInfo.name.c_str());

            // Call the module's initialization function
            if (moduleInfo.hasMainClass)
            {
                asIScriptModule* pModule = GetModule(moduleInfo.name);
                if (pModule)
                {
                    // Call the auto-generated initialization function
                    std::string initFuncName = moduleInfo.name + "_Initialize";
                    MS_INFO("ASModuleSystem: Looking for initialization function '%s'", initFuncName.c_str());
                    
                    asIScriptFunction* pInitFunc = pModule->GetFunctionByName(initFuncName.c_str());
                    if (pInitFunc)
                    {
                        MS_INFO("ASModuleSystem: Found initialization function, creating context...");
                        asIScriptContext* pContext = m_pEngine->CreateContext();
                        if (pContext)
                        {
                            MS_INFO("ASModuleSystem: Preparing to execute '%s'...", initFuncName.c_str());
                            pContext->Prepare(pInitFunc);
                            int r = pContext->Execute();

                            if (r == asEXECUTION_FINISHED)
                            {
                                MS_INFO("ASModuleSystem: Module '%s' initialized successfully", moduleInfo.name.c_str());
                            }
                            else if (r == asEXECUTION_EXCEPTION)
                            {
                                MS_ERROR("ASModuleSystem: WARNING - Module '%s' initialization threw exception: %s",
                                       moduleInfo.name.c_str(), pContext->GetExceptionString());
                            }
                            else
                            {
                                MS_ERROR("ASModuleSystem: WARNING - Module '%s' initialization failed: %d", moduleInfo.name.c_str(), r);
                            }

                            pContext->Release();
                        }
                        else
                        {
                            MS_ERROR("ASModuleSystem: ERROR - Failed to create context for initialization");
                        }
                    }
                    else
                    {
                        MS_INFO("ASModuleSystem: WARNING - Module '%s' has no initialization function '%s'",
                               moduleInfo.name.c_str(), initFuncName.c_str());

                        // List all functions in the module for debugging
                        MS_INFO("ASModuleSystem: Functions in module '%s':", moduleInfo.name.c_str());
                        for (asUINT i = 0; i < pModule->GetFunctionCount(); i++)
                        {
                            asIScriptFunction* pFunc = pModule->GetFunctionByIndex(i);
                            if (pFunc)
                            {
                                MS_INFO("  - %s", pFunc->GetName());
                            }
                        }
                    }
                }
            }
        }
        else
        {
            MS_ERROR("ASModuleSystem: ERROR - Failed to load module '%s' from %s",
                   moduleInfo.name.c_str(), moduleInfo.filePath.c_str());
            failedModules.push_back(moduleInfo.name + " (" + moduleInfo.filePath + ")");
            allSuccess = false;
        }
    }

    if (allSuccess)
    {
        MS_INFO("ASModuleSystem: All discovered modules loaded successfully");
    }
    else
    {
        MS_ERROR("ASModuleSystem: FAILED TO LOAD %d MODULE(S):", (int)failedModules.size());
        for (const std::string& failed : failedModules)
        {
            MS_ERROR("  - %s", failed.c_str());
        }
    }
    
    return allSuccess;
}

//==========================================================================
// Hot-Reload Functionality
//==========================================================================
bool ASModuleSystem::ReloadAllModules()
{
    if (!m_pEngine)
    {
        printf("ASModuleSystem::ReloadAllModules: ERROR - Not initialized\n");
        return false;
    }
    
    printf("ASModuleSystem: Starting hot-reload of all modules...\n");
    
    // Attempt PAK refresh with retry logic
    const int maxRetries = 3;
    bool pakRefreshSuccess = false;
    
    for (int attempt = 1; attempt <= maxRetries; attempt++)
    {
        printf("ASModuleSystem::ReloadAllModules: PAK refresh attempt %d/%d\n", attempt, maxRetries);
        
        if (RefreshFromPak())
        {
            pakRefreshSuccess = true;
            break;
        }
        
        if (attempt < maxRetries)
        {
            printf("ASModuleSystem::ReloadAllModules: PAK refresh failed, waiting 1 second before retry...\n");
#ifdef _WIN32
            Sleep(1000); // Sleep for 1 second on Windows
#else
            sleep(1); // Sleep for 1 second on Unix-like systems
#endif
        }
    }
    
    if (!pakRefreshSuccess)
    {
        printf("ASModuleSystem::ReloadAllModules: ERROR - Failed to refresh PAK file after %d attempts\n", maxRetries);
        printf("ASModuleSystem::ReloadAllModules: Hot-reload aborted to prevent system instability\n");
        return false;
    }
    
    printf("ASModuleSystem::ReloadAllModules: PAK refresh successful\n");
    
    // Get list of currently loaded modules to preserve order
    std::vector<std::string> loadedModules = GetLoadedModules();
    printf("ASModuleSystem: Found %d loaded modules to reload\n", (int)loadedModules.size());
    
    // CRITICAL: Unregister all event handlers BEFORE unloading modules
    // This prevents stale function pointers from being executed after module unload
    printf("ASModuleSystem::ReloadAllModules: Unregistering event handlers from all modules...\n");
    ASEngineEventManager* pEventManager = ASEngineEventManager::Instance();
    if (pEventManager)
    {
        for (const std::string& moduleName : loadedModules)
        {
            printf("ASModuleSystem: Unregistering event handlers for module '%s'\n", moduleName.c_str());
            pEventManager->UnregisterAllHandlers(moduleName.c_str());
        }
        printf("ASModuleSystem::ReloadAllModules: Event handlers unregistered successfully\n");
    }
    else
    {
        printf("ASModuleSystem::ReloadAllModules: WARNING - Event manager not available\n");
    }
    
    // Store module info before unloading for restoration if needed
    std::map<std::string, ASModuleInfo> moduleBackup;
    for (const std::string& moduleName : loadedModules)
    {
        auto it = m_Modules.find(moduleName);
        if (it != m_Modules.end())
        {
            moduleBackup[moduleName] = it->second;
        }
    }
    
    // Unload all modules in reverse dependency order
    std::vector<std::string> unloadOrder;
    if (TopologicalSort(unloadOrder))
    {
        std::reverse(unloadOrder.begin(), unloadOrder.end());
        
        for (const std::string& moduleName : unloadOrder)
        {
            printf("ASModuleSystem: Unloading module '%s'\n", moduleName.c_str());
            
            // Force unload by temporarily reducing reference count
            auto it = m_Modules.find(moduleName);
            if (it != m_Modules.end())
            {
                int originalRefCount = it->second.refCount;
                it->second.refCount = 0;  // Temporarily set to 0 to allow unload
                it->second.isBuiltin = false;  // Allow unloading even if marked as builtin
                
                if (!UnloadModule(moduleName))
                {
                    printf("ASModuleSystem: WARNING - Failed to unload module '%s'\n", moduleName.c_str());
                    // Restore reference count if unload failed
                    if (m_Modules.find(moduleName) != m_Modules.end())
                    {
                        m_Modules[moduleName].refCount = originalRefCount;
                    }
                }
            }
        }
    }
    
    // Re-discover modules in the refreshed PAK
    printf("ASModuleSystem::ReloadAllModules: Discovering modules in refreshed PAK...\n");
    if (!DiscoverModulesInPak(&g_ScriptPack))
    {
        printf("ASModuleSystem::ReloadAllModules: ERROR - Failed to discover modules after PAK refresh\n");
        printf("ASModuleSystem::ReloadAllModules: This could indicate PAK corruption or missing module files\n");
        
        // Try to restore backed up modules
        printf("ASModuleSystem::ReloadAllModules: Attempting to restore previous module state...\n");
        for (const auto& backup : moduleBackup)
        {
            if (m_Modules.find(backup.first) == m_Modules.end())
            {
                printf("ASModuleSystem::ReloadAllModules: Restoring module: %s\n", backup.first.c_str());
                m_Modules[backup.first] = backup.second;
            }
        }
        
        return false;
    }
    
    // Load all discovered modules with error recovery
    printf("ASModuleSystem::ReloadAllModules: Loading discovered modules...\n");
    bool loadSuccess = LoadDiscoveredModules(&g_ScriptPack);
    
    if (!loadSuccess)
    {
        printf("ASModuleSystem::ReloadAllModules: ERROR - Failed to load discovered modules\n");
        printf("ASModuleSystem::ReloadAllModules: Checking which modules failed to load...\n");
        
        // Check which modules are missing and try to restore them
        int restoredCount = 0;
        for (const auto& backup : moduleBackup)
        {
            if (m_Modules.find(backup.first) == m_Modules.end())
            {
                printf("ASModuleSystem::ReloadAllModules: Module '%s' missing after reload, attempting restore...\n", backup.first.c_str());
                
                // Try to restore the module state
                m_Modules[backup.first] = backup.second;
                restoredCount++;
            }
        }
        
        if (restoredCount > 0)
        {
            printf("ASModuleSystem::ReloadAllModules: Restored %d modules from backup\n", restoredCount);
            printf("ASModuleSystem::ReloadAllModules: Hot-reload partially failed, but system stability maintained\n");
        }
        else
        {
            printf("ASModuleSystem::ReloadAllModules: Unable to restore any modules - system may be unstable\n");
        }
        
        return false;
    }
    
    // Verify that all expected modules were reloaded
    printf("ASModuleSystem::ReloadAllModules: Verifying module reload success...\n");
    int successfulReloads = 0;
    int failedReloads = 0;
    
    for (const std::string& originalModule : loadedModules)
    {
        if (HasModule(originalModule))
        {
            successfulReloads++;
            printf("ASModuleSystem::ReloadAllModules: ✓ Module '%s' reloaded successfully\n", originalModule.c_str());
        }
        else
        {
            failedReloads++;
            printf("ASModuleSystem::ReloadAllModules: ✗ Module '%s' failed to reload\n", originalModule.c_str());
        }
    }
    
    printf("ASModuleSystem: Hot-reload completed\n");
    printf("ASModuleSystem: Modules successfully reloaded: %d\n", successfulReloads);
    printf("ASModuleSystem: Modules failed to reload: %d\n", failedReloads);
    
    if (failedReloads > 0)
    {
        printf("ASModuleSystem: WARNING - Some modules failed to reload, check console for errors\n");
    }
    
    PrintModuleStats();
    
    return failedReloads == 0;
}

bool ASModuleSystem::RefreshFromPak()
{
    // Refresh the scripts.pak file to get any changes
    printf("ASModuleSystem::RefreshFromPak: Starting PAK file refresh process...\n");
    
    // Get current status information
    bool wasOpen = g_ScriptPack.IsOpen();
    size_t currentEntries = g_ScriptPack.GetEntryCount();
    
    printf("ASModuleSystem::RefreshFromPak: Current PAK status - %s, %zu entries\n",
           wasOpen ? "OPEN" : "CLOSED", currentEntries);
    
    // Try multiple common locations for scripts.pak
    const char* pakLocations[] = {
        "scripts.pak",
        "./scripts.pak",
        "../scripts.pak",
        "scripts/scripts.pak"
    };
    const int numLocations = sizeof(pakLocations) / sizeof(pakLocations[0]);
    
    if (!wasOpen)
    {
        printf("ASModuleSystem::RefreshFromPak: PAK file not currently open, searching for scripts.pak...\n");
        
        bool found = false;
        for (int i = 0; i < numLocations; i++)
        {
            printf("ASModuleSystem::RefreshFromPak: Trying location: %s\n", pakLocations[i]);
            
            // Use the enhanced file existence check from CGameGroupFile
            if (g_ScriptPack.Open(pakLocations[i]))
            {
                printf("ASModuleSystem::RefreshFromPak: Successfully opened PAK file at: %s\n", pakLocations[i]);
                found = true;
                break;
            }
        }
        
        if (!found)
        {
            printf("ASModuleSystem::RefreshFromPak: ERROR - Could not find scripts.pak in any location\n");
            printf("ASModuleSystem::RefreshFromPak: Searched locations:\n");
            for (int i = 0; i < numLocations; i++)
            {
                printf("  - %s\n", pakLocations[i]);
            }
            return false;
        }
    }
    else
    {
        printf("ASModuleSystem::RefreshFromPak: PAK file is open, attempting refresh...\n");
        
        // Get the current filename before refresh
        std::string currentFilename;
        if (g_ScriptPack.GetFilename().length() > 0)
        {
            currentFilename = g_ScriptPack.GetFilename();
            printf("ASModuleSystem::RefreshFromPak: Current PAK filename: %s\n", currentFilename.c_str());
        }
        else
        {
            printf("ASModuleSystem::RefreshFromPak: WARNING - No stored filename, will try default locations\n");
        }
        
        // Attempt to refresh the existing PAK file
        bool refreshSuccess = false;
        
        if (!currentFilename.empty())
        {
            printf("ASModuleSystem::RefreshFromPak: Attempting refresh with stored filename...\n");
            refreshSuccess = g_ScriptPack.Refresh();
        }
        
        // If refresh failed and we don't have a filename, try common locations
        if (!refreshSuccess)
        {
            printf("ASModuleSystem::RefreshFromPak: Initial refresh failed, trying fallback locations...\n");
            
            for (int i = 0; i < numLocations; i++)
            {
                printf("ASModuleSystem::RefreshFromPak: Fallback attempt with: %s\n", pakLocations[i]);
                
                if (g_ScriptPack.Refresh(pakLocations[i]))
                {
                    printf("ASModuleSystem::RefreshFromPak: Refresh successful with fallback location: %s\n", pakLocations[i]);
                    refreshSuccess = true;
                    break;
                }
            }
        }
        
        if (!refreshSuccess)
        {
            printf("ASModuleSystem::RefreshFromPak: ERROR - All refresh attempts failed\n");
            printf("ASModuleSystem::RefreshFromPak: This could indicate:\n");
            printf("  - scripts.pak file is missing or moved\n");
            printf("  - File permission issues\n");
            printf("  - PAK file corruption\n");
            printf("  - File is locked by another process\n");
            return false;
        }
    }
    
    // Verify the refresh was successful
    size_t newEntries = g_ScriptPack.GetEntryCount();
    printf("ASModuleSystem::RefreshFromPak: PAK file refreshed successfully\n");
    printf("ASModuleSystem::RefreshFromPak: Entry count: %zu (was %zu)\n", newEntries, currentEntries);
    
    if (newEntries == 0)
    {
        printf("ASModuleSystem::RefreshFromPak: WARNING - PAK file has no entries, this may indicate an empty or corrupted file\n");
    }
    
    return true;
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
            printf("   OK Module functions registered successfully\n");
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
            printf("   OK Import/export functions registered successfully\n");
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
        r = pEngine->RegisterGlobalFunction("bool LoadASModule(const string &in)", asFUNCTION(LoadASModule), asCALL_CDECL);
        if (r < 0) return false;
        
        r = pEngine->RegisterGlobalFunction("bool UnloadModule(const string &in)", asFUNCTION(UnloadModule), asCALL_CDECL);
        if (r < 0) return false;
        
        r = pEngine->RegisterGlobalFunction("bool ReloadModule(const string &in)", asFUNCTION(ReloadModule), asCALL_CDECL);
        if (r < 0) return false;
        
        r = pEngine->RegisterGlobalFunction("bool HasModule(const string &in)", asFUNCTION(HasModule), asCALL_CDECL);
        if (r < 0) return false;
        
        // Register hot-reload function
        r = pEngine->RegisterGlobalFunction("bool ReloadAllScriptModules()", asFUNCTION(ReloadAllScriptModules), asCALL_CDECL);
        if (r < 0) return false;
        
        return true;
    }
    
    bool RegisterImportExportFunctions(asIScriptEngine* pEngine)
    {
        int r;
        
        // Register import functions
        r = pEngine->RegisterGlobalFunction("bool ImportModule(const string &in)", asFUNCTION(static_cast<bool(*)(const std::string&)>(ImportModule)), asCALL_CDECL);
        if (r < 0) return false;
        
        r = pEngine->RegisterGlobalFunction("bool ImportModule(const string &in, const string &in)", asFUNCTION(static_cast<bool(*)(const std::string&, const std::string&)>(ImportModule)), asCALL_CDECL);
        if (r < 0) return false;
        
        return true;
    }
}

//==========================================================================
// Context Management
//==========================================================================
ScriptContext ASModuleSystem::GetModuleContext(const std::string& name) const
{
    auto it = m_Modules.find(name);
    if (it != m_Modules.end())
    {
        return it->second.context;
    }
    return ScriptContext::UNKNOWN;
}

bool ASModuleSystem::IsModuleCompatibleWithCurrentBuild(const std::string& name) const
{
    ScriptContext context = GetModuleContext(name);
    if (context == ScriptContext::UNKNOWN)
    {
        return false;
    }
    
    return ASScriptContextUtil::CanRunInCurrentContext(context);
}

//==========================================================================
// AngelScript Global Functions Implementation
//==========================================================================
bool LoadASModule(const std::string& filename)
{
    ASModuleSystem* pSystem = ASModuleSystem::Instance();
    if (!pSystem)
    {
        printf("LoadASModule: ERROR - Module system not initialized\n");
        return false;
    }
    
    return pSystem->LoadASModule(filename);
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

bool ReloadAllScriptModules()
{
    ASModuleSystem* pSystem = ASModuleSystem::Instance();
    if (!pSystem)
    {
        printf("ReloadAllScriptModules: ERROR - Module system not initialized\n");
        return false;
    }
    
    return pSystem->ReloadAllModules();
}