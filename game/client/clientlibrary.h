#include "netadr.h"

class CClientLibrary {
public:
	CClientLibrary() = default;
	~CClientLibrary() = default;

	bool Initialize();
	void PostInitialize();

	void HUDInit();
	void Shutdown();
	void RunFrame();
	void ResetClient();

private:
	bool m_IsConnected = false;
	float m_ConnectionTime;
	netadr_t m_ServerAddress;
	const char *m_MapName;
};