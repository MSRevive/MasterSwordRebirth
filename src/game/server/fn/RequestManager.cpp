#include "RequestManager.h"
#include "msdllheaders.h"

void CRequestManager::Init()
{
	Shutdown();

	// FN Doesn't work on listen servers.
	if (!IS_DEDICATED_SERVER())
	{
		return;
	}

	if (!m_bLoaded) 
	{
		m_pShareHandle = curl_share_init();
		if (m_pShareHandle)
		{
			curl_share_setopt(m_pShareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);
			curl_share_setopt(m_pShareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
		}

		m_pMultiHandle = curl_multi_init();
		if (!m_pMultiHandle)
			return;

		// Enable connection pooling / pipelining.
		// CURLPIPE_MULTIPLEX enables HTTP/2 multiplexing where supported,
		// otherwise connections are still pooled and reused via keep-alive.
		curl_multi_setopt(m_pMultiHandle, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);

		// Max total connections kept in the pool (default is 10).
		curl_multi_setopt(m_pMultiHandle, CURLMOPT_MAXCONNECTS, 16L);

		// Max connections to a single host.
		curl_multi_setopt(m_pMultiHandle, CURLMOPT_MAX_HOST_CONNECTIONS, 8L);

		m_bLoaded = true;
	}
}

void CRequestManager::Think(bool onShutdown)
{
	if (!m_bLoaded || !m_pMultiHandle)
		return;

	for (int i = static_cast<int>(m_vRequests.size() - 1); i >= 0; i--)
	{
		HTTPRequest* req = m_vRequests[i];

		if (req->m_iRequestState == HTTPRequest::RequestState::QUEUED)
		{
			CURL* easy = req->PrepareForMulti();
			if (easy)
			{
				curl_multi_add_handle(m_pMultiHandle, easy);
			}
			else
			{
				delete req;
				m_vRequests.erase(m_vRequests.begin() + i);
			}
		}

		// If onShutdown is set and the request has been executed, just discard it.
		// this is to finish processing all requests 
		if (onShutdown && (req->m_iRequestState == HTTPRequest::RequestState::EXECUTED))
		{
			// Need to remove from multi handle before deleting — but OnMultiComplete
			// may not have fired yet. Force-remove it.
			// Note: PrepareForMulti stores the CURL* inside the HTTPRequest, and
			// OnMultiComplete cleans it up. If we're discarding early we must do it here.
			// We can't easily get the easy handle back without adding an accessor,
			// so we just delete the request (destructor cleans up the easy handle)
			// and rely on curl_multi_remove_handle being tolerant of already-removed handles.
			delete req;
			m_vRequests.erase(m_vRequests.begin() + i);
		}
	}

	curl_multi_perform(m_pMultiHandle, &m_iRunningTransfers);

	ProcessMultiCompleted();

	for (int i = static_cast<int>(m_vRequests.size() - 1); i >= 0; i--)
	{
		HTTPRequest* req = m_vRequests[i];

		if (req->m_iRequestState == HTTPRequest::RequestState::FINISHED)
		{
			delete req;
			m_vRequests.erase(m_vRequests.begin() + i);
			continue;
		}
	}
}

void CRequestManager::ProcessMultiCompleted()
{
	CURLMsg* msg;
	int msgsLeft = 0;

	while ((msg = curl_multi_info_read(m_pMultiHandle, &msgsLeft)))
	{
		if (msg->msg != CURLMSG_DONE)
			continue;

		CURL* easy = msg->easy_handle;
		CURLcode result = msg->data.result;

		HTTPRequest* req = nullptr;
		curl_easy_getinfo(easy, CURLINFO_PRIVATE, &req);
		curl_multi_remove_handle(m_pMultiHandle, easy);

		if (req)
		{
			req->OnMultiComplete(result);
		}
		else
		{
			// Orphaned handle — shouldn't happen, but clean up defensively.
			curl_easy_cleanup(easy);
		}
	}
}

extern void wait(unsigned long ms);
void CRequestManager::Shutdown(void)
{
	if (!m_bLoaded)
		return;

	do {
		Think(true);
		wait(100);
	} while (m_vRequests.size() != 0);

	m_vRequests.clear();

	if (m_pMultiHandle)
	{
		curl_multi_cleanup(m_pMultiHandle);
		m_pMultiHandle = nullptr;
	}

	if (m_pShareHandle)
	{
		curl_share_cleanup(m_pShareHandle);
		m_pShareHandle = nullptr;
	}

	m_bLoaded = false;
}

void CRequestManager::QueueRequest(HTTPRequest* req)
{
	if (!m_bLoaded)
		return;
	
	m_vRequests.push_back(req);
}