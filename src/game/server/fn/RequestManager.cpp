#include "RequestManager.h"
#include "strhelper.h"
#include "steam/steam_api.h"
#include "steam/steam_gameserver.h"
#include "msdllheaders.h"
#include "SteamHTTPReq.h"

void CRequestManager::Init(void)
{
	// FN Doesn't work on listen servers.
	if (!IS_DEDICATED_SERVER())
		return;

	if (!m_bLoaded) 
	{
		m_SteamGameServerAPIContext.Init();
		m_SteamHTTP = m_SteamGameServerAPIContext->SteamHTTP();
		m_bLoaded = true;
	}
}

void CRequestManager::Think(void)
{
	if (m_bLoaded)
	{
		if (m_bSteamLoaded && !m_SteamHTTP)
		{
			m_SteamHTTP = m_SteamGameServerAPIContext->SteamHTTP();
		}

		for (int i = (m_vRequests.size() - 1); i >= 0; i--)
		{
			SteamHttpRequest* req = m_vRequests[i];
			switch (req->requestState)
			{
			case REQUEST_QUEUED:
				req->SendRequest();
				break;

			case REQUEST_FINISHED:
				delete req;
				m_vRequests.erase(m_vRequests.begin() + i);
				break;
			}
		}
	}
}

void CRequestManager::Shutdown(void)
{	
	m_SteamGameServerAPIContext.Shutdown();
	m_SteamHTTP	= nullptr;
	m_vRequests.clear()
	m_bLoaded = false;
}

void CRequestManager::RunCallbacks(void) 
{
	if (m_SteamGameServerAPIContext->SteamHTTP())
		SteamGameServer_RunCallbacks();
}

ISteamHTTP* GetHTTPContext(void)
{
	if (m_bLoaded) 
	{
		if (m_bSteamLoaded && !m_SteamHTTP)
			m_SteamHTTP = m_SteamGameServerAPIContext->SteamHTTP();

		return m_SteamHTTP;
	}

	return nullptr;
}

extern void wait(unsigned long ms);
void SendAndWait(void)
{
	if (m_bLoaded)
	{
		g_bSuppressResponse = true;

		do
		{
			Think();
			RunCallbacks();
			wait(10);
		} while ((m_SteamHTTP != nullptr) && m_vRequests.size());

		Shutdown();
		g_bSuppressResponse = false;
	}
}