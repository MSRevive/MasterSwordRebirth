#include "RequestManager.h"
#include "msdllheaders.h"

void CRequestManager::Init(ISteamHTTP* steamHTTP)
{
	// FN Doesn't work on listen servers.
	if (!IS_DEDICATED_SERVER())
	{
		return;
	}

	if (!m_bLoaded) 
	{
		m_SteamHTTP = steamHTTP;
		m_bLoaded = true;
	}
}

void CRequestManager::Think(bool suppressResp)
{
	if (m_bLoaded)
	{
		// if (!m_SteamHTTP)
		// {
		// 	m_SteamHTTP = SteamGameServerHTTP();
		// }

		for (int i = (m_vRequests.size() - 1); i >= 0; i--)
		{
			HTTPRequest* req = m_vRequests[i];
			req->SuppressResponse(suppressResp);
			req->SetHTTPContext(m_SteamHTTP);
			switch (req->requestState)
			{
			case HTTPRequest::RequestState::REQUEST_QUEUED:
				req->SendRequest();
				break;

			case  HTTPRequest::RequestState::REQUEST_FINISHED:
				delete req;
				m_vRequests.erase(m_vRequests.begin() + i);
				break;
			}
		}
	}
}

void CRequestManager::Shutdown(void)
{	
	m_vRequests.clear();
	m_bLoaded = false;
}

extern void wait(unsigned long ms);
void CRequestManager::SendAndWait(void)
{
	if (m_bLoaded)
	{
		do
		{
			Think(true);
			SteamGameServer_RunCallbacks();
			wait(10);
		} while ((m_SteamHTTP != nullptr) && m_vRequests.size());

		Shutdown();
	}
}

void CRequestManager::QueueRequest(HTTPRequest* req)
{
	req->SetHTTPContext(m_SteamHTTP);
	m_vRequests.push_back(req);
}