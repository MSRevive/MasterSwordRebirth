#ifndef CLIENT_LIBRARY_H
#define CLIENT_LIBRARY_H

#include "netadr.h"

class CClientLibrary {
public:
	CClientLibrary() = default;
	~CClientLibrary() = default;

	bool Initialize();
	void PostInitialize();

	void HUDInit();
	void Shutdown();
	void VideoInit();
	void RunFrame();
	void ResetClient();

private:
	bool m_IsConnected = false;
	float m_ConnectionTime;
	netadr_t m_ServerAddress;
};

#endif //CLIENT_LIBRARY_H