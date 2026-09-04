#include "RequestManager.h"
#include "msdllheaders.h"

void CRequestManager::Init()
{
	Shutdown();

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
		{
			// Don't leak the share handle if the multi handle failed.
			if (m_pShareHandle)
			{
				curl_share_cleanup(m_pShareHandle);
				m_pShareHandle = nullptr;
			}
			return;
		}

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

void CRequestManager::Think(bool bForceDiscard)
{
	if (!m_bLoaded || !m_pMultiHandle)
		return;

	// ---- Pass 1: start queued transfers (or drop them outright if tearing down).
	for (size_t i = 0; i < m_vRequests.size(); )
	{
		HTTPRequest* req = m_vRequests[i];

		if (req->m_iRequestState != HTTPRequest::RequestState::QUEUED)
		{
			++i;
			continue;
		}

		if (bForceDiscard)
		{
			// Never handed to curl, so plain destruction is safe.
			delete req;
			m_vRequests.erase(m_vRequests.begin() + i);
			continue;
		}

		CURL* easy = req->PrepareForMulti();
		if (!easy || (curl_multi_add_handle(m_pMultiHandle, easy) != CURLM_OK))
		{
			// Not registered with the multi handle, so the destructor's
			// curl_easy_cleanup() is safe here.
			delete req;
			m_vRequests.erase(m_vRequests.begin() + i);
			continue;
		}

		++i;
	}

	curl_multi_perform(m_pMultiHandle, &m_iRunningTransfers);

	ProcessMultiCompleted();

	// ---- Pass 2: reap finished (and, when tearing down, still-running) requests.
	for (size_t i = 0; i < m_vRequests.size(); )
	{
		HTTPRequest* req = m_vRequests[i];

		const bool bFinished = (req->m_iRequestState == HTTPRequest::RequestState::FINISHED);
		const bool bAbort    = bForceDiscard &&
		                       (req->m_iRequestState == HTTPRequest::RequestState::EXECUTED);

		if (!bFinished && !bAbort)
		{
			++i;
			continue;
		}

		if (bAbort)
		{
			// Removes from the multi stack *before* easy_cleanup, which the old
			// code never did at all.
			req->AbortTransfer(m_pMultiHandle);
		}

		delete req;
		m_vRequests.erase(m_vRequests.begin() + i);
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
			// Orphaned handle -- shouldn't happen, but clean up defensively.
			curl_easy_cleanup(easy);
		}
	}
}

extern void wait(unsigned long ms);

void CRequestManager::Shutdown(void)
{
	if (!m_bLoaded)
		return;

	constexpr int kDrainTimeoutMs = 5000;
	constexpr int kStepMs = 50;

	for (int waited = 0; !m_vRequests.empty() && (waited < kDrainTimeoutMs); waited += kStepMs)
	{
		Think(false);

		if (m_vRequests.empty())
			break;

		wait(kStepMs);
	}

	// Anything still outstanding is aborted safely (remove-then-cleanup).
	Think(true);

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

	// Think(true) leaves this empty; belt-and-braces in case a request survived.
	for (size_t i = 0; i < m_vRequests.size(); i++)
		delete m_vRequests[i];

	m_vRequests.clear();
	m_iRunningTransfers = 0;
	m_bLoaded = false;
}

void CRequestManager::Clear(void)
{
	for (size_t i = 0; i < m_vRequests.size(); i++)
	{
		HTTPRequest* req = m_vRequests[i];

		if (req->m_iRequestState == HTTPRequest::RequestState::EXECUTED)
			req->AbortTransfer(m_pMultiHandle);

		delete req;
	}

	m_vRequests.clear();
}

bool CRequestManager::QueueRequest(HTTPRequest* req)
{
	if (req == nullptr)
		return false;

	if (!m_bLoaded)
	{
		delete req; // We were handed ownership; don't silently drop it.
		return false;
	}

	m_vRequests.push_back(req);
	return true;
}