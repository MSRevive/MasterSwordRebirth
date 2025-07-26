//==========================================================================
// ASClientEngineImpl.cpp
// 
// Client-side implementation of the AngelScript engine interface
//==========================================================================

#include "ASClientEngineImpl.h"
#include "mslogger.h"
#include "hud.h"
#include "cl_util.h"

//==========================================================================
// Communication Function Implementations (Client-Side Stubs)
//==========================================================================

void ASClientEngine::SendPlayerMessageImpl(const std::string& playerID, const std::string& message)
{
    // For client, if the playerID is empty or refers to the local player, send the message to self
    if (playerID.empty() || message.empty()) {
        MS_ANGEL_DEBUG("SendPlayerMessageImpl: Empty playerID or message");
        return;
    }
    
    // Send message to client console (this client can only receive messages for itself)
    ConsolePrint("[MESSAGE] ");
    ConsolePrint(message.c_str());
    ConsolePrint("\n");
    
    MS_ANGEL_DEBUG("SendPlayerMessageImpl: Sent message to client: %s", message.c_str());
}

void ASClientEngine::SendConsoleMessageImpl(const std::string& playerID, const std::string& message)
{
    MS_ANGEL_DEBUG("SendConsoleMessageImpl: Client cannot send console messages to other players");
}

void ASClientEngine::SendMessageToAllPlayersImpl(const std::string& color, const std::string& message)
{
    MS_ANGEL_DEBUG("SendMessageToAllPlayersImpl: Client cannot send messages to all players");
}

void ASClientEngine::SendInfoMessageToAllImpl(const std::string& title, const std::string& message)
{
    MS_ANGEL_DEBUG("SendInfoMessageToAllImpl: Client cannot send info messages to all players");
}