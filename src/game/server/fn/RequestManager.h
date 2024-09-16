#ifndef MS_REQUEST_MANAGER
#define MS_REQUEST_MANAGER

#include "steam/steam_api.h"
#include "steam/steam_gameserver.h"
#include "HTTPRequest.h"
#include <vector>

class CRequestManager
{
public:
	CRequestManager() = default;
	~CRequestManager() = default;
	void Init();
	void Think(bool suppressResp = false);
	void Shutdown(void);
	void SendAndWait(void);
	static void SetBaseURL(const char* url);

	void Clear(void) { m_vRequests.clear(); }
	void QueueRequest(HTTPRequest* req);

private:
	bool m_bLoaded = false;

	std::vector<HTTPRequest*> m_vRequests;
};

extern CRequestManager g_FNRequestManager;

#endif // MS_REQUEST_MANAGER