#ifndef CLIENT_DISCORD_RPC_H
#define CLIENT_DISCORD_RPC_H

#ifdef _WIN32
#pragma once
#endif

void DiscordRPCInitialize();
void DiscordRPCShutdown();
void DiscordRPCUpdate();

#endif // CLIENT_DISCORD_RPC_H