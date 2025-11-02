//==============================================================================
// scriptmodule.cpp
//
// AngelScript Module System Addon Implementation
//==============================================================================

#include "scriptmodule.h"
#ifndef SCRIPTPACK_TOOL
#include "../../../groupfile.h" // this just... feels wrong
#endif
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>

#ifdef SCRIPTPACK_TOOL
// Standalone mode for scriptpack tool
#define BEGIN_AS_NAMESPACE
#define END_AS_NAMESPACE
// Use cout instead of printf for scriptpack tool
#include <iostream>
#define SCRIPT_PRINT(msg) std::cout << msg << std::endl
#else
BEGIN_AS_NAMESPACE
#define SCRIPT_PRINT(msg) printf(msg)
#endif

//==============================================================================
// Constructor/Destructor
//==============================================================================
CScriptModule::CScriptModule()
{
}

CScriptModule::~CScriptModule()
{
    ClearModules();
}

//==============================================================================
// Module Discovery
//==============================================================================
bool CScriptModule::DiscoverModulesInPak(CGameGroupFile* pakFile)
{
#ifdef SCRIPTPACK_TOOL
    // Not available in scriptpack tool mode
    return false;
#else
    if (!pakFile)
    {
        SCRIPT_PRINT("CScriptModule::DiscoverModulesInPak: ERROR - NULL pak file");
        return false;
    }
    
    SCRIPT_PRINT("CScriptModule: Discovering modules in PAK file using real enumeration...");
    
    // Get all AngelScript files from the PAK using proper enumeration
    std::vector<std::string> angelscriptFiles;
    int fileCount = pakFile->EnumerateAngelScriptFiles(angelscriptFiles);
    
    printf("CScriptModule: Found %d .as files in PAK\n", fileCount);
    
    if (fileCount == 0)
    {
        SCRIPT_PRINT("CScriptModule: No AngelScript files found in PAK");
        return false;
    }
    
    bool foundAny = false;
    
    // Process each AngelScript file to look for module declarations
    for (const std::string& filePath : angelscriptFiles)
    {
#ifdef SCRIPTPACK_TOOL        
        printf("CScriptModule: Examining file: %s\n", filePath.c_str());
#endif
        // Get file size first
        unsigned long fileSize;
        if (!pakFile->ReadEntry(filePath.c_str(), nullptr, fileSize))
        {
            printf("CScriptModule: WARNING - Could not get size for %s\n", filePath.c_str());
            continue;
        }
        
        // Read the file content
        char* content = new char[fileSize + 1];
        if (pakFile->ReadEntry(filePath.c_str(), (unsigned char*)content, fileSize))
        {
            content[fileSize] = '\0';
            
            // Process this file for module declarations
            if (ProcessModuleFile(filePath, std::string(content)))
            {
                foundAny = true;
                printf("CScriptModule: Found module in %s\n", filePath.c_str());
            }
        }
        else
        {
            printf("CScriptModule: WARNING - Could not read content for %s\n", filePath.c_str());
        }
        
        delete[] content;
    }
    
    printf("CScriptModule: Discovery complete. Found %d modules.\n", (int)m_discoveredModules.size());
    return foundAny;
#endif
}

bool CScriptModule::DiscoverModulesInDirectory(const std::string& directory)
{
    // TODO: Implement directory scanning for non-PAK scenarios
    printf("CScriptModule::DiscoverModulesInDirectory: Not implemented yet\n");
    return false;
}

//==============================================================================
// Module Processing
//==============================================================================
bool CScriptModule::ProcessModuleFile(const std::string& filePath, const std::string& source)
{
#ifdef SCRIPTPACK_TOOL
    printf("CScriptModule: Processing file: %s\n", filePath.c_str());
#endif
    
    // Input validation
    if (filePath.empty())
    {
        printf("CScriptModule: ERROR - Empty file path provided\n");
        return false;
    }
    
    if (source.empty())
    {
        printf("CScriptModule: WARNING - Empty source content for file: %s\n", filePath.c_str());
        return false;
    }
    
    std::string moduleName;
    std::string processedSource;
    std::string namespaceName;

    // Try to preprocess the source to extract module information
    if (PreprocessModuleSource(source, processedSource, moduleName, namespaceName))
    {
        printf("CScriptModule: Successfully preprocessed module '%s' from file: %s\n",
               moduleName.c_str(), filePath.c_str());

        // Check for duplicate module names
        if (HasModule(moduleName))
        {
            printf("CScriptModule: ERROR - Duplicate module name '%s' found in file: %s\n",
                   moduleName.c_str(), filePath.c_str());
            printf("CScriptModule: Previous module with same name already exists\n");
            return false;
        }

        // Validate module name follows proper conventions
        if (moduleName.length() > 64)
        {
            printf("CScriptModule: ERROR - Module name too long (%zu chars): '%s'\n",
                   moduleName.length(), moduleName.c_str());
            return false;
        }

        // Create module info with comprehensive validation
        ModuleInfo moduleInfo(moduleName, filePath);
        moduleInfo.source = source;
        moduleInfo.processed = processedSource;
        moduleInfo.namespaceName = namespaceName;  // Save the namespace

        // Validate that the module has an Init() function
        size_t moduleStart;
        std::string dummyNamespace;
        if (ParseModuleDeclaration(source, moduleName, moduleStart, dummyNamespace))
        {
            // Perform detailed structure validation
            if (!ValidateModuleStructure(source, moduleStart, moduleInfo.hasMainClass))
            {
                printf("CScriptModule: ERROR - Module structure validation failed for: %s\n", 
                       moduleName.c_str());
                return false;
            }
            
            // Warn if no main class constructor found
            if (!moduleInfo.hasMainClass)
            {
                printf("CScriptModule: WARNING - Module '%s' does not have a main class constructor\n", 
                       moduleName.c_str());
                printf("CScriptModule: Module will be loaded but cannot be automatically instantiated\n");
            }
        }
        else
        {
            printf("CScriptModule: ERROR - Failed to re-parse module declaration for: %s\n", 
                   moduleName.c_str());
            return false;
        }
        
        // Validate processed source is not empty
        if (processedSource.empty())
        {
            printf("CScriptModule: ERROR - Processed source is empty for module: %s\n", 
                   moduleName.c_str());
            return false;
        }
        
        // Add to discovered modules with duplicate check
        if (m_moduleNameMap.find(moduleName) != m_moduleNameMap.end())
        {
            printf("CScriptModule: ERROR - Internal consistency error: module '%s' already in map\n", 
                   moduleName.c_str());
            return false;
        }
        
        m_moduleNameMap[moduleName] = m_discoveredModules.size();
        m_discoveredModules.push_back(moduleInfo);
        
        printf("CScriptModule: Successfully processed module '%s' from file: %s (hasMainClass: %s)\n", 
               moduleName.c_str(), filePath.c_str(), moduleInfo.hasMainClass ? "Yes" : "No");
        
        return true;
    }
    else
    {
#ifdef SCRIPTPACK_TOOL
        printf("CScriptModule: No valid module declaration found in file: %s\n", filePath.c_str());
        printf("CScriptModule: File does not contain 'module ModuleName {' pattern\n");
#endif
        return false;
    }
}

bool CScriptModule::PreprocessModuleSource(const std::string& source, std::string& output, std::string& moduleName, std::string& namespaceName)
{
#ifdef SCRIPTPACK_TOOL
    printf("CScriptModule: Starting preprocessing of source (%zu bytes)\n", source.length());
#endif
    
    // Input validation
    if (source.empty())
    {
#ifndef SCRIPTPACK_TOOL
        printf("CScriptModule: ERROR - Cannot preprocess empty source\n");
#endif
        return false;
    }

    size_t moduleStart;

    // Look for 'module ModuleName {' declaration using enhanced parsing
    if (!ParseModuleDeclaration(source, moduleName, moduleStart, namespaceName))
    {
#ifdef SCRIPTPACK_TOOL
        printf("CScriptModule: No valid module declaration found during preprocessing\n");
#endif
        return false; // No module declaration found
    }

#ifndef SCRIPTPACK_TOOL
    printf("CScriptModule: Found module '%s' at position %zu",
           moduleName.c_str(), moduleStart);
    if (!namespaceName.empty())
    {
        printf(" in namespace '%s'", namespaceName.c_str());
    }
    printf(", starting transformation\n");
#endif
    
    // Validate module name one more time
    if (moduleName.empty())
    {
#ifndef SCRIPTPACK_TOOL
        printf("CScriptModule: ERROR - Module name is empty after parsing\n");
#endif
        return false;
    }
    
    // Transform the module declaration to a class
    try
    {
        output = TransformModuleToClass(source, moduleName, moduleStart);
        
        if (output.empty())
        {
#ifndef SCRIPTPACK_TOOL
            printf("CScriptModule: ERROR - Module transformation resulted in empty output\n");
#endif
            return false;
        }
        
#ifndef SCRIPTPACK_TOOL
        printf("CScriptModule: Successfully transformed module to class (%zu bytes)\n", output.length());
#endif
    }
    catch (const std::exception& e)
    {
        printf("CScriptModule: ERROR - Exception during module transformation: %s\n", e.what());
        return false;
    }
    catch (...)
    {
        printf("CScriptModule: ERROR - Unknown exception during module transformation\n");
        return false;
    }
    
    // Add module wrapper code
    try
    {
        std::string wrapper = GenerateModuleWrapper(moduleName, namespaceName);
        if (wrapper.empty())
        {
            printf("CScriptModule: ERROR - Failed to generate module wrapper\n");
            return false;
        }

        output += "\n" + wrapper;
        printf("CScriptModule: Added module wrapper code (%zu bytes total)\n", output.length());
    }
    catch (const std::exception& e)
    {
        printf("CScriptModule: ERROR - Exception during wrapper generation: %s\n", e.what());
        return false;
    }
    catch (...)
    {
        printf("CScriptModule: ERROR - Unknown exception during wrapper generation\n");
        return false;
    }
    
    printf("CScriptModule: Preprocessing completed successfully for module '%s'\n", moduleName.c_str());
    return true;
}

//==============================================================================
// Module Parsing
//==============================================================================
bool CScriptModule::ParseModuleDeclaration(const std::string& source, std::string& moduleName, size_t& moduleStart, std::string& namespaceName)
{
    // Enhanced AngelScript-style parsing for 'module ModuleName {' pattern
    const char* sourceData = source.c_str();
    size_t sourceLength = source.length();
    size_t pos = 0;

    // Track namespace nesting
    std::vector<std::string> namespaceStack;

    while (pos < sourceLength)
    {
        // Skip whitespace and comments using AngelScript-style approach
        size_t newPos = SkipWhitespaceAndComments(sourceData, sourceLength, pos);
        if (newPos == pos)
        {
            // No whitespace/comments to skip, continue with token detection
        }
        else
        {
            pos = newPos;
            continue;
        }

        // Check for 'namespace' keyword to track nesting
        size_t tokenLength;
        if (IsKeywordAt(sourceData, sourceLength, pos, "namespace", tokenLength))
        {
            pos += tokenLength;
            pos = SkipWhitespaceAndComments(sourceData, sourceLength, pos);

            // Parse namespace name
            size_t identifierLength;
            if (pos < sourceLength && IsIdentifierAt(sourceData, sourceLength, pos, identifierLength))
            {
                std::string nsName = source.substr(pos, identifierLength);
                pos += identifierLength;
                pos = SkipWhitespaceAndComments(sourceData, sourceLength, pos);

                // Look for opening brace
                if (pos < sourceLength && sourceData[pos] == '{')
                {
                    namespaceStack.push_back(nsName);
                    printf("CScriptModule: Entering namespace '%s'\n", nsName.c_str());
                    pos++;
                    continue;
                }
            }
        }

        // Check for closing braces (namespace end)
        if (sourceData[pos] == '}' && !namespaceStack.empty())
        {
            // This could be a namespace closing brace
            // For simplicity, we pop from the namespace stack
            // (More robust parsing would track brace pairs)
            printf("CScriptModule: Exiting namespace '%s'\n", namespaceStack.back().c_str());
            namespaceStack.pop_back();
            pos++;
            continue;
        }

        // Check for 'module' keyword using enhanced token recognition
        if (IsKeywordAt(sourceData, sourceLength, pos, "module", tokenLength))
        {
            // Debug: found module keyword (disabled for scriptpack)

            // Save the start position (beginning of 'module' keyword)
            size_t moduleKeywordStart = pos;
            pos += tokenLength;

            // Skip whitespace after 'module' keyword
            pos = SkipWhitespaceAndComments(sourceData, sourceLength, pos);

            // Parse module name using AngelScript-style identifier parsing
            size_t identifierLength;
            if (pos < sourceLength && IsIdentifierAt(sourceData, sourceLength, pos, identifierLength))
            {
                moduleName = source.substr(pos, identifierLength);
                printf("CScriptModule: Extracted module name: '%s'\n", moduleName.c_str());

                // Validate that it's a proper identifier
                if (!IsValidIdentifier(moduleName))
                {
                    printf("CScriptModule: ERROR - Invalid module name: '%s'\n", moduleName.c_str());
                    pos++;
                    continue;
                }

                pos += identifierLength;

                // Skip whitespace after module name
                pos = SkipWhitespaceAndComments(sourceData, sourceLength, pos);

                // Look for opening brace
                if (pos < sourceLength && sourceData[pos] == '{')
                {
                    moduleStart = moduleKeywordStart;

                    // Build the full namespace path
                    if (!namespaceStack.empty())
                    {
                        for (size_t i = 0; i < namespaceStack.size(); i++)
                        {
                            if (i > 0) namespaceName += "::";
                            namespaceName += namespaceStack[i];
                        }
                    }

                    printf("CScriptModule: Successfully parsed module declaration: '%s' at position %zu",
                           moduleName.c_str(), moduleStart);
                    if (!namespaceName.empty())
                    {
                        printf(" in namespace '%s'", namespaceName.c_str());
                    }
                    printf("\n");
                    return true;
                }
                else
                {
                    printf("CScriptModule: ERROR - Expected '{' after module name '%s', found '%c' at position %zu\n",
                           moduleName.c_str(),
                           pos < sourceLength ? sourceData[pos] : '?',
                           pos);
                }
            }
            else
            {
                printf("CScriptModule: ERROR - Expected identifier after 'module' keyword at position %zu\n", pos);
            }
        }

        // Move to next character if no valid module declaration found
        pos++;
    }

#ifdef SCRIPTPACK_TOOL
    printf("CScriptModule: No module declaration found in source\n");
#endif
    return false;
}

bool CScriptModule::ValidateModuleStructure(const std::string& source, size_t moduleStart, bool& hasMainClass)
{
    hasMainClass = false;
    
    // First, extract the module name from the module declaration
    std::string moduleNameToFind;
    
    // Find module name from the module declaration at moduleStart
    const char* sourceData = source.c_str();
    size_t sourceLength = source.length();
    size_t moduleKeywordPos = source.find("module", moduleStart);
    if (moduleKeywordPos != std::string::npos)
    {
        size_t nameStart = moduleKeywordPos + 6; // Skip "module"
        nameStart = SkipWhitespaceAndComments(sourceData, sourceLength, nameStart);
        
        size_t nameLength;
        if (IsIdentifierAt(sourceData, sourceLength, nameStart, nameLength))
        {
            moduleNameToFind = source.substr(nameStart, nameLength);
        }
    }
    
    if (moduleNameToFind.empty())
    {
        printf("CScriptModule: Could not extract module name for constructor validation\n");
        return false;
    }
    
    printf("CScriptModule: Looking for constructor '%s()' in module\n", moduleNameToFind.c_str());
    
    // Debug: Let's see what we're actually looking for
    printf("CScriptModule: DEBUG - Module name extracted: '%s' (length: %zu)\n", moduleNameToFind.c_str(), moduleNameToFind.length());
    
    // Search for all occurrences of the constructor pattern
    std::string constructorPattern = moduleNameToFind + "(";
    size_t searchPos = moduleStart;
    bool foundValidConstructor = false;
    
    while (searchPos < source.length())
    {
        size_t constructorPos = source.find(constructorPattern, searchPos);
        
        if (constructorPos == std::string::npos)
        {
            break; // No more occurrences found
        }
        
        printf("CScriptModule: Found constructor pattern '%s' at position %zu\n", constructorPattern.c_str(), constructorPos);
        
        // Verify it's a proper constructor (not just a function call)
        // Check that it's not preceded by a dot or other non-whitespace characters that would indicate a function call
        bool isValidConstructor = true;
        if (constructorPos > 0)
        {
            char prevChar = source[constructorPos - 1];
            if (prevChar != ' ' && prevChar != '\t' && prevChar != '\n' && prevChar != '\r' && prevChar != '{' && prevChar != ';')
            {
                isValidConstructor = false;
                printf("CScriptModule: Constructor pattern found but preceded by '%c' - likely a function call\n", prevChar);
            }
        }
        
        if (isValidConstructor)
        {
            foundValidConstructor = true;
            hasMainClass = true;
            printf("CScriptModule: Confirmed main class constructor '%s()' found at position %zu\n", moduleNameToFind.c_str(), constructorPos);
            break; // Found a valid constructor, no need to continue searching
        }
        
        // Move search position to continue looking for more occurrences
        searchPos = constructorPos + constructorPattern.length();
    }
    
    if (!foundValidConstructor)
    {
        printf("CScriptModule: No valid constructor pattern '%s' found in module\n", constructorPattern.c_str());
    }
    
    printf("CScriptModule: Module validation complete. Has main class: %s\n", hasMainClass ? "Yes" : "No");
    return true;
}

//==============================================================================
// Module Transformation
//==============================================================================
std::string CScriptModule::TransformModuleToClass(const std::string& source, const std::string& moduleName, size_t moduleStart)
{
    // Replace 'module ModuleName {' with 'class ModuleName {'
    std::string result = source;
    
    // Find the exact position of 'module'
    size_t moduleKeywordPos = result.find("module", moduleStart);
    if (moduleKeywordPos != std::string::npos)
    {
        result.replace(moduleKeywordPos, 6, "class");
    }
    
    return result;
}

std::string CScriptModule::GenerateModuleWrapper(const std::string& moduleName, const std::string& namespaceName)
{
    std::ostringstream wrapper;

    // Build the full qualified class name
    std::string qualifiedName = moduleName;
    if (!namespaceName.empty())
    {
        qualifiedName = namespaceName + "::" + moduleName;
    }

    wrapper << "\n// Auto-generated module wrapper for " << moduleName;
    if (!namespaceName.empty())
    {
        wrapper << " (in namespace " << namespaceName << ")";
    }
    wrapper << "\n";

    wrapper << qualifiedName << "@ g_" << moduleName << " = null;\n";
    wrapper << "\n";
    wrapper << "void " << moduleName << "_Initialize()\n";
    wrapper << "{\n";
    wrapper << "    LogMessage(\"[MODULE] " << moduleName << "_Initialize() called\");\n";
    wrapper << "    if (g_" << moduleName << " is null)\n";
    wrapper << "    {\n";
    wrapper << "        LogMessage(\"[MODULE] Creating " << moduleName << " instance...\");\n";
    wrapper << "        @g_" << moduleName << " = " << qualifiedName << "();\n";
    wrapper << "        LogMessage(\"[MODULE] " << moduleName << " instance created successfully\");\n";
    wrapper << "        if (g_" << moduleName << " !is null)\n";
    wrapper << "        {\n";
    wrapper << "            LogMessage(\"[MODULE] Instance verification: SUCCESS - g_" << moduleName << " is valid\");\n";
    wrapper << "        }\n";
    wrapper << "        else\n";
    wrapper << "        {\n";
    wrapper << "            LogMessage(\"[MODULE] Instance verification: FAILED - g_" << moduleName << " is null after creation!\");\n";
    wrapper << "        }\n";
    wrapper << "    }\n";
    wrapper << "    else\n";
    wrapper << "    {\n";
    wrapper << "        LogMessage(\"[MODULE] " << moduleName << " instance already exists\");\n";
    wrapper << "    }\n";
    wrapper << "}\n";
    wrapper << "\n";
    wrapper << "void " << moduleName << "_Shutdown()\n";
    wrapper << "{\n";
    wrapper << "    LogMessage(\"[MODULE] " << moduleName << "_Shutdown() called\");\n";
    wrapper << "    @g_" << moduleName << " = null;\n";
    wrapper << "}\n";

    return wrapper.str();
}

//==============================================================================
// Module Information
//==============================================================================
ModuleInfo* CScriptModule::GetModuleInfo(const std::string& moduleName)
{
    auto it = m_moduleNameMap.find(moduleName);
    if (it != m_moduleNameMap.end())
    {
        return &m_discoveredModules[it->second];
    }
    return nullptr;
}

bool CScriptModule::HasModule(const std::string& moduleName) const
{
    return m_moduleNameMap.find(moduleName) != m_moduleNameMap.end();
}

void CScriptModule::ClearModules()
{
    m_discoveredModules.clear();
    m_moduleNameMap.clear();
}

//==============================================================================
// Debug and Utilities
//==============================================================================
void CScriptModule::PrintDiscoveredModules() const
{
    printf("\n=== Discovered Modules ===\n");
    printf("Total modules found: %d\n", (int)m_discoveredModules.size());
    
    for (size_t i = 0; i < m_discoveredModules.size(); i++)
    {
        const ModuleInfo& info = m_discoveredModules[i];
        printf("  [%d] %s\n", (int)i, info.name.c_str());
        printf("      File: %s\n", info.filePath.c_str());
        printf("      Has main class: %s\n", info.hasMainClass ? "Yes" : "No");
    }
    printf("==========================\n\n");
}

std::string CScriptModule::TrimWhitespace(const std::string& str) const
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool CScriptModule::IsValidIdentifier(const std::string& identifier) const
{
    if (identifier.empty())
        return false;
    
    if (!std::isalpha(identifier[0]) && identifier[0] != '_')
        return false;
    
    for (size_t i = 1; i < identifier.length(); i++)
    {
        if (!std::isalnum(identifier[i]) && identifier[i] != '_')
            return false;
    }
    
    return true;
}

//==============================================================================
// Enhanced Parsing Functions (AngelScript-style)
//==============================================================================
bool CScriptModule::IsKeywordAt(const char* source, size_t sourceLength, size_t pos, const char* keyword, size_t& tokenLength) const
{
    size_t keywordLen = strlen(keyword);
    
    // Check if we have enough characters remaining
    if (pos + keywordLen > sourceLength)
        return false;
    
    // Check if the keyword matches at this position
    if (strncmp(source + pos, keyword, keywordLen) != 0)
        return false;
    
    // Verify word boundaries - similar to AngelScript's approach
    // Check character before keyword (if exists)
    if (pos > 0)
    {
        char prevChar = source[pos - 1];
        if ((prevChar >= 'a' && prevChar <= 'z') ||
            (prevChar >= 'A' && prevChar <= 'Z') ||
            (prevChar >= '0' && prevChar <= '9') ||
            prevChar == '_')
        {
            return false; // Not a word boundary
        }
    }
    
    // Check character after keyword (if exists)
    if (pos + keywordLen < sourceLength)
    {
        char nextChar = source[pos + keywordLen];
        if ((nextChar >= 'a' && nextChar <= 'z') ||
            (nextChar >= 'A' && nextChar <= 'Z') ||
            (nextChar >= '0' && nextChar <= '9') ||
            nextChar == '_')
        {
            return false; // Not a word boundary
        }
    }
    
    tokenLength = keywordLen;
    return true;
}

bool CScriptModule::IsIdentifierAt(const char* source, size_t sourceLength, size_t pos, size_t& tokenLength) const
{
    if (pos >= sourceLength)
        return false;
    
    char c = source[pos];
    
    // Starting with letter or underscore (similar to AngelScript's IsIdentifier)
    if (!((c >= 'a' && c <= 'z') ||
          (c >= 'A' && c <= 'Z') ||
          c == '_'))
    {
        return false;
    }
    
    tokenLength = 1;
    
    // Continue with letters, digits, or underscores
    for (size_t n = pos + 1; n < sourceLength; n++)
    {
        c = source[n];
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '_')
        {
            tokenLength++;
        }
        else
        {
            break;
        }
    }
    
    return true;
}

size_t CScriptModule::SkipWhitespaceAndComments(const char* source, size_t sourceLength, size_t pos) const
{
    while (pos < sourceLength)
    {
        char c = source[pos];
        
        // Skip whitespace
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        {
            pos++;
            continue;
        }
        
        // Skip single-line comments (//)
        if (pos < sourceLength - 1 && source[pos] == '/' && source[pos + 1] == '/')
        {
            pos += 2;
            while (pos < sourceLength && source[pos] != '\n')
                pos++;
            continue;
        }
        
        // Skip multi-line comments (/* */)
        if (pos < sourceLength - 1 && source[pos] == '/' && source[pos + 1] == '*')
        {
            pos += 2;
            while (pos < sourceLength - 1)
            {
                if (source[pos] == '*' && source[pos + 1] == '/')
                {
                    pos += 2;
                    break;
                }
                pos++;
            }
            continue;
        }
        
        // No more whitespace or comments
        break;
    }
    
    return pos;
}

bool CScriptModule::IsWordBoundary(const char* source, size_t pos, size_t sourceLength) const
{
    if (pos == 0 || pos >= sourceLength)
        return true; // Start/end of source is always a word boundary
    
    char prevChar = source[pos - 1];
    char currChar = source[pos];
    
    // Check if transition from non-identifier to identifier character or vice versa
    bool prevIsIdent = (prevChar >= 'a' && prevChar <= 'z') ||
                       (prevChar >= 'A' && prevChar <= 'Z') ||
                       (prevChar >= '0' && prevChar <= '9') ||
                       prevChar == '_';
                       
    bool currIsIdent = (currChar >= 'a' && currChar <= 'z') ||
                       (currChar >= 'A' && currChar <= 'Z') ||
                       (currChar >= '0' && currChar <= '9') ||
                       currChar == '_';
    
    // Word boundary exists when transitioning between identifier and non-identifier chars
    return prevIsIdent != currIsIdent;
}

#ifdef SCRIPTPACK_TOOL
// Standalone mode
#else
END_AS_NAMESPACE
#endif