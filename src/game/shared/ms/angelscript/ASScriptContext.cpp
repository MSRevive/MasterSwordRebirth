//==========================================================================
// ASScriptContext.cpp
// 
// Script execution context management implementation
//==========================================================================

#include "ASScriptContext.h"
#include "mslogger.h"
#include <algorithm>
#include <cstring>

// Singleton instance
ASScriptContextManager* ASScriptContextManager::s_pInstance = nullptr;

//==========================================================================
// ASScriptContextManager Implementation
//==========================================================================

ASScriptContextManager::ASScriptContextManager()
{
    MS_ANGEL_INFO("ASScriptContextManager initialized for %s build", 
        ASScriptContextUtil::IsClientBuild() ? "CLIENT" : "SERVER");
}

ASScriptContextManager::~ASScriptContextManager()
{
}

ASScriptContextManager* ASScriptContextManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new ASScriptContextManager();
    }
    return s_pInstance;
}

void ASScriptContextManager::Shutdown()
{
    if (s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

void ASScriptContextManager::SetScriptContext(const std::string& scriptName, ScriptContext context)
{
    m_ScriptContexts[scriptName] = context;
    MS_ANGEL_DEBUG("Set script context: %s = %s", 
        scriptName.c_str(), 
        ASScriptContextUtil::ContextToString(context));
}

ScriptContext ASScriptContextManager::GetScriptContext(const std::string& scriptName) const
{
    auto it = m_ScriptContexts.find(scriptName);
    if (it != m_ScriptContexts.end())
    {
        return it->second;
    }
    return ScriptContext::UNKNOWN;
}

ScriptContext ASScriptContextManager::DetermineContextFromPath(const std::string& path) const
{
    // Convert path to lowercase for case-insensitive comparison
    std::string lowerPath = path;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
    
    // Strategy 1: Directory-based detection
    // Check for /server/, /client/, /shared/ directories
    if (lowerPath.find("/server/") != std::string::npos ||
        lowerPath.find("\\server\\") != std::string::npos ||
        lowerPath.find("angelscript/server/") != std::string::npos)
    {
        return ScriptContext::SERVER_ONLY;
    }
    
    if (lowerPath.find("/client/") != std::string::npos ||
        lowerPath.find("\\client\\") != std::string::npos ||
        lowerPath.find("angelscript/client/") != std::string::npos)
    {
        return ScriptContext::CLIENT_ONLY;
    }
    
    if (lowerPath.find("/shared/") != std::string::npos ||
        lowerPath.find("\\shared\\") != std::string::npos ||
        lowerPath.find("angelscript/shared/") != std::string::npos)
    {
        return ScriptContext::SHARED;
    }
    
    // Strategy 2: File extension detection
    // .server.as, .client.as, .shared.as
    if (lowerPath.find(".server.as") != std::string::npos)
    {
        return ScriptContext::SERVER_ONLY;
    }
    
    if (lowerPath.find(".client.as") != std::string::npos)
    {
        return ScriptContext::CLIENT_ONLY;
    }
    
    if (lowerPath.find(".shared.as") != std::string::npos)
    {
        return ScriptContext::SHARED;
    }
    
    // Strategy 3: Filename prefix detection
    // server_, client_, shared_
    size_t lastSlash = lowerPath.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? 
        lowerPath.substr(lastSlash + 1) : lowerPath;
    
    if (filename.find("server_") == 0)
    {
        return ScriptContext::SERVER_ONLY;
    }
    
    if (filename.find("client_") == 0)
    {
        return ScriptContext::CLIENT_ONLY;
    }
    
    if (filename.find("shared_") == 0)
    {
        return ScriptContext::SHARED;
    }
    
    // Default: If in angelscript directory and no specific markers, assume SHARED
    if (lowerPath.find("angelscript/") != std::string::npos)
    {
        return ScriptContext::SHARED;
    }
    
    // Unknown context
    return ScriptContext::UNKNOWN;
}

ScriptContext ASScriptContextManager::DetermineContextFromContent(const std::string& content) const
{
    // Look for #pragma context directives
    // Supported formats:
    //   #pragma context server
    //   #pragma context client
    //   #pragma context shared
    //   // @context server
    //   // @context client
    //   // @context shared
    
    size_t pragmaPos = content.find("#pragma context");
    if (pragmaPos != std::string::npos)
    {
        // Find the context keyword after the pragma
        size_t lineEnd = content.find('\n', pragmaPos);
        if (lineEnd == std::string::npos)
            lineEnd = content.length();
        
        std::string pragmaLine = content.substr(pragmaPos, lineEnd - pragmaPos);
        
        if (pragmaLine.find("server") != std::string::npos)
            return ScriptContext::SERVER_ONLY;
        if (pragmaLine.find("client") != std::string::npos)
            return ScriptContext::CLIENT_ONLY;
        if (pragmaLine.find("shared") != std::string::npos)
            return ScriptContext::SHARED;
    }
    
    // Also check for comment-based context markers
    size_t commentPos = content.find("// @context");
    if (commentPos != std::string::npos)
    {
        size_t lineEnd = content.find('\n', commentPos);
        if (lineEnd == std::string::npos)
            lineEnd = content.length();
        
        std::string commentLine = content.substr(commentPos, lineEnd - commentPos);
        
        if (commentLine.find("server") != std::string::npos)
            return ScriptContext::SERVER_ONLY;
        if (commentLine.find("client") != std::string::npos)
            return ScriptContext::CLIENT_ONLY;
        if (commentLine.find("shared") != std::string::npos)
            return ScriptContext::SHARED;
    }
    
    return ScriptContext::UNKNOWN;
}

bool ASScriptContextManager::ValidateFunctionCall(const std::string& functionName, ScriptContext requiredContext) const
{
    // If the function doesn't have a specific context requirement, allow it
    if (requiredContext == ScriptContext::UNKNOWN)
        return true;
    
    // Check if the function can run in the current build context
    return ASScriptContextUtil::CanRunInCurrentContext(requiredContext);
}

void ASScriptContextManager::RegisterFunctionContext(const std::string& functionName, ScriptContext context)
{
    m_FunctionContexts[functionName] = context;
    MS_ANGEL_DEBUG("Registered function context: %s = %s", 
        functionName.c_str(), 
        ASScriptContextUtil::ContextToString(context));
}

ScriptContext ASScriptContextManager::GetCurrentExecutionContext() const
{
    return ASScriptContextUtil::GetCurrentBuildContext();
}

bool ASScriptContextManager::IsContextCompatible(ScriptContext scriptContext, ScriptContext functionContext) const
{
    // SHARED functions can be called from anywhere
    if (functionContext == ScriptContext::SHARED)
        return true;
    
    // SHARED scripts can call anything
    if (scriptContext == ScriptContext::SHARED)
        return true;
    
    // Otherwise, contexts must match
    return scriptContext == functionContext;
}

void ASScriptContextManager::LogContextInfo() const
{
    MS_ANGEL_INFO("=== Script Context Manager Status ===");
    MS_ANGEL_INFO("Current Build: %s", 
        ASScriptContextUtil::IsClientBuild() ? "CLIENT" : "SERVER");
    
    MS_ANGEL_INFO("Registered Scripts: %d", (int)m_ScriptContexts.size());
    for (const auto& pair : m_ScriptContexts)
    {
        MS_ANGEL_INFO("  %s: %s", 
            pair.first.c_str(), 
            ASScriptContextUtil::ContextToString(pair.second));
    }
    
    MS_ANGEL_INFO("Registered Functions: %d", (int)m_FunctionContexts.size());
    int serverFuncs = 0, clientFuncs = 0, sharedFuncs = 0;
    for (const auto& pair : m_FunctionContexts)
    {
        switch (pair.second)
        {
            case ScriptContext::SERVER_ONLY: serverFuncs++; break;
            case ScriptContext::CLIENT_ONLY: clientFuncs++; break;
            case ScriptContext::SHARED: sharedFuncs++; break;
            default: break;
        }
    }
    
    MS_ANGEL_INFO("  Server-only: %d", serverFuncs);
    MS_ANGEL_INFO("  Client-only: %d", clientFuncs);
    MS_ANGEL_INFO("  Shared: %d", sharedFuncs);
    MS_ANGEL_INFO("=====================================");
}

