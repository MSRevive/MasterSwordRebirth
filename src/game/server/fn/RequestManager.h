#ifndef MS_REQUEST_MANAGER
#define MS_REQUEST_MANAGER

#include "steam/steam_api.h"
#include "steam/steam_gameserver.h"
#include "SteamHTTPReq.h"
#include <vector>

class CRequestManager
{
public:
	CRequestManager() = default;
	~CRequestManager() = default;
	void Init(void);
	void Think(void);
	void Shutdown(void);
	void RunCallbacks(void);
	void SendAndWait(void);

	void Clear(void) { m_vRequests.clear(); }
	void Queue(SteamHttpRequest* req);

private:
	bool m_bLoaded = false;

	ISteamHTTP* m_SteamHTTP;

	static std::vector<SteamHttpRequest*> m_vRequests;
};

extern CRequestManager g_FNRequestManager;

#endif // MS_REQUEST_MANAGER