#ifndef MS_REQUEST_MANAGER
#define MS_REQUEST_MANAGER

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
	*ISteamHTTP GetHTTPContext(void);
	void SendAndWait(void);

	void Clear(void) { m_vRequests.clear(); }
	void Queue(SteamHTTPRequest* req) { m_vRequests.push_back(req); }

private:
	bool m_bLoaded = false;

	bool m_bSteamLoaded = false;
	static CSteamGameServerAPIContext* m_SteamGameServerAPIContext;
	ISteamHTTP* m_SteamHTTP;

	static std::Vector<SteamHttpRequest*> m_vRequests;
}

class CSteamGameServerAPIContext;
class ISteamHTTP;
class SteamHttpRequest;

extern CRequestManager g_FNRequestManager;

#endif // MS_REQUEST_MANAGER