#include "RequestManager.h"
#include "msdllheaders.h"
#include "SteamServerHelper.h"
#include <steam/steam_api.h>
#include <steam/steam_gameserver.h>

void CRequestManager::Init()
{
	// FN Doesn't work on listen servers.
	if (!IS_DEDICATED_SERVER())
	{
		return;
	}

	if (!m_bLoaded) 
	{
		m_bLoaded = true;
	}
}

void CRequestManager::Think()
{
	if (m_bLoaded)
	{
		for (int i = (m_vRequests.size() - 1); i >= 0; i--)
		{
			HTTPRequest* req = m_vRequests[i];
			
			switch (req->requestState)
			{
			case HTTPRequest::RequestState::REQUEST_QUEUED:
				req->SendRequest();
				break;

			case HTTPRequest::RequestState::REQUEST_FINISHED:
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
			g_SteamServerHelper->RunCallbacks();
			wait(10);
		} while ((g_SteamHTTPContext != nullptr) && m_vRequests.size());

		Shutdown();
	}
}

void CRequestManager::QueueRequest(HTTPRequest* req)
{
	m_vRequests.push_back(req);
}