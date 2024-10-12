//
// Steam HTTP Request Handler Class
//

#include <string>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include "base64/base64.h"
#include "HTTPRequest.h"
#include "FNSharedDefs.h"
#include "msdllheaders.h"
#include "global.h"
#include "player.h"
#include "FNSharedDefs.h"

static char g_szBaseUrl[REQUEST_URL_SIZE];

HTTPRequest::HTTPRequest(EHTTPMethod method, const char* url, uint8* body, size_t bodySize, ID64 steamID64, ID64 slot)
{
	m_eHTTPMethodhttpMethod = method;
	m_iRequestState = RequestState::REQUEST_QUEUED;
	_snprintf(m_sPchAPIUrl, REQUEST_URL_SIZE, "http://%s%s", g_szBaseUrl, url);

	m_sRequestBody = responseBody = nullptr;
	m_iRequestBodySize = responseBodySize = 0;
	m_pJSONData = nullptr;
	handle = NULL;

	m_iSteamID64 = steamID64;
	m_iSlot = slot;

	if ((body != nullptr) && (bodySize > 0))
	{
		m_iRequestBodySize = bodySize;
		m_sRequestBody = new uint8[m_iRequestBodySize];
		memcpy(m_sRequestBody, body, m_sRequestBodySize);
	}

	m_sResponseBody.clear();
}

HTTPRequest::~HTTPRequest()
{
	Cleanup();
}

bool HTTPRequest::AsyncSendRequest()
{
	m_iRequestState = RequestState::REQUEST_EXECUTED;
}

bool HTTPRequest::SendRequest()
{
	m_iRequestState = RequestState::REQUEST_EXECUTED;

	m_Handle = curl_easy_init();
	curl_easy_setopt(m_Handle, CURLOPT_URL, m_sPchAPIUrl);
	curl_easy_setopt(m_Handle, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(m_Handle, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(m_Handle, CURLOPT_NOSIGNAL, 1L);

	switch (m_eHTTPMethod)
	{
		case HTTPRequest::GET:
			break;
		case HTTPRequest::POST:
			curl_easy_setopt(m_Handle, CURLOPT_POST, 1);
			break;
		case HTTPRequest::DELETE:
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
			break;
		case HTTPRequest::PUT:
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
			break;
	}

	// Process request body.
	if (m_sRequestBody != nullptr)
	{
		char steamID64String[REQUEST_URL_SIZE];
		_snprintf(steamID64String, REQUEST_URL_SIZE, "%llu", m_iSteamID64);

		rapidjson::StringBuffer s;
		rapidjson::Writer<rapidjson::StringBuffer> writer(s);

		writer.StartObject();

		writer.Key("steamid");
		writer.String(steamID64String);

		writer.Key("slot");
		writer.Int(m_iSlot);

		writer.Key("size");
		writer.Int(m_iRequestBodySize);

		writer.Key("data");
		writer.String(base64_encode(m_sRequestBody, m_iRequestBodySize).c_str());

		writer.EndObject();

		const std::string buffer = s.GetString();
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer.c_str());
	}

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &DataCallbackEvent);
	CURLcode result = curl_easy_perform(curl);
	if (result == CURLE_OK)
	{
		int httpCode = 200;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		ResponseCallback(httpCode, false)
	}else{
		ResponseCallback(0, true)
	}
	curl_easy_cleanup(curl);

	return (result == CURLE_OK)
}

void HTTPRequest::DataCallbackEvent(char* buf, size_t size, size_t nmemb, void* up)
{
	m_sResponseBody.append(buf, size * nmemb)
	return (size * nmemb);
}

void HTTPRequest::ResponseCallback(int httpCode, bool bError = false)
{
	if (bError)
	{
		FNShared::Print("Request Failed. %s, '%s'\n", GetName(), g_szBaseUrl);
		m_iRequestState = RequestState::REQUEST_FINISHED;
		return;
	}

	if (httpCode < 200 || httpCode > 299)
	{
		if (httpCode == 401)
		{
			FNShared::Print("FN Authorization failed! %s\n", GetName());
			m_iRequestState = RequestState::REQUEST_FINISHED;
			return;
		}

		FNShared::Print("FN Server Error. %s Code: %d\n", GetName(), httpCode);
		m_iRequestState = RequestState::REQUEST_FINISHED;
		return;
	}

	if (httpCode == 204)
	{
		OnResponse(true, nullptr, httpCode);
		m_iRequestState = RequestState::REQUEST_FINISHED;
		return;
	}

	JSONDocument* jsonDoc = ParseJSON(g_pDataBuffer.c_str());
	if (!jsonResp)
	{
		FNShared::Print("The data hasn't been received. HTTP code: %d\n", httpCode);
		OnResponse(true, nullptr, httpCode);
		m_iRequestState = RequestState::REQUEST_FINISHED;
		return;
	}

	OnResponse(true, jsonDoc);
	delete jsonDoc;
	m_iRequestState = RequestState::REQUEST_FINISHED;
}

JSONDocument* HTTPRequest::ParseJSON(const char* data, size_t length)
{
	if (!(data && data[0]))
		return nullptr;

	JSONDocument* document = new JSONDocument;

	if (length > 0)
		document->Parse(data, length);
	else
		document->Parse(data);

	if (document->HasParseError())
	{
		delete document;
		return nullptr;
	}

	return document;
}

void Cleanup()
{
	delete m_sRequestBody;
	m_sResponseBody.clear();
}

/*
void HTTPRequest::OnHTTPRequestCompleted(HTTPRequestCompleted_t* p, bool bError)
{
	if (suppressResponse || (handle == NULL) || (p == nullptr) || (p->m_hRequest != handle))
	{
		ReleaseHandle();
		return;
	}

	if (bError || p->m_eStatusCode < 200 || p->m_eStatusCode > 299)
	{
		if (p->m_eStatusCode == 401)
		{
			FNShared::Print("FN Authorization failed! %s\n", GetName());
			ReleaseHandle();
			return;
		}

		if (!p->m_bRequestSuccessful)
		{
			FNShared::Print("The data hasn't been received. No response from the server. %s, '%s'\n", GetName(), g_szBaseUrl);
			OnResponse(false, p->m_eStatusCode);
			ReleaseHandle();
			return;
		}

		FNShared::Print("FN Server Error. %s Code: %d\n", GetName(), p->m_eStatusCode);
		ReleaseHandle();
		return;
	}

	size_t unBytes = 0;
	if ((responseBody == nullptr) && g_SteamHTTPContext->GetHTTPResponseBodySize(handle, &unBytes))
	{
		// it should've never gotten to this point but okay.
		if (p->m_eStatusCode == 204)
		{
			OnResponse(true, p->m_eStatusCode);
			ReleaseHandle();
			return;
		}

		if (unBytes <= 0)
		{
			FNShared::Print("The data hasn't been received. HTTP code: %d\n", p->m_eStatusCode);
			ReleaseHandle();
			return;
		}

		responseBodySize = unBytes;
		responseBody = new uint8[responseBodySize];

		if (g_SteamHTTPContext->GetHTTPResponseBodyData(handle, responseBody, unBytes))
			pJSONData = ParseJSON(reinterpret_cast<char*>(responseBody), responseBodySize);
	}

	OnResponse(true, p->m_eStatusCode);
	ReleaseHandle();
}*/

/* static */ void HTTPRequest::SetBaseURL(const char* url)
{
	_snprintf(g_szBaseUrl, REQUEST_URL_SIZE, "%s", url);
}