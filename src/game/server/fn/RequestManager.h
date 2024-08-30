#ifndef MS_REQUEST_MANAGER
#define MS_REQUEST_MANAGER

#include <vector>
#include "steamtypes"

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

#endif // MS_REQUEST_MANAGER