#ifndef MS_REQUEST_MANAGER
#define MS_REQUEST_MANAGER

#include "HTTPRequest.h"
#include <vector>
#include <curl/curl.h>

class CRequestManager
{
public:
	CRequestManager() = default;
	~CRequestManager() = default;
	void Init();
	void Think(bool skipCallback = false);
	void Shutdown(void);

	void Clear(void) { m_vRequests.clear(); }
	void QueueRequest(HTTPRequest* req);

	CURLSH* GetShareHandle() const { return m_pShareHandle; }

private:
	void ProcessMultiCompleted();

	bool m_bLoaded = false;

	CURLM* m_pMultiHandle = nullptr;
	CURLSH* m_pShareHandle = nullptr;

	int m_iRunningTransfers = 0; // this is for curl_multi_perform to keep track of handles.
	std::vector<HTTPRequest*> m_vRequests;
};

extern CRequestManager g_FNRequestManager;

#endif // MS_REQUEST_MANAGER