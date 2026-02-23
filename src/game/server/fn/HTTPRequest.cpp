//
// Steam HTTP Request Handler Class
//

#ifndef _WIN32
#define CURL_PULL_SYS_TYPES_H
#define CURL_PULL_STDINT_H
#define CURL_PULL_INTTYPES_H
#define HAVE_SYS_SOCKET_H
#endif

#include <future>
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

HTTPRequest::HTTPRequest(HTTPMethod method, const char* url, const char* body, size_t bodySize, ID64 steamID64, ID64 slot)
{
	m_eHTTPMethod = method;
	m_iRequestState = RequestState::QUEUED;
	_snprintf(m_sPchAPIUrl, REQUEST_URL_SIZE, "http://%s%s", g_szBaseUrl, url);

	m_sRequestBody = nullptr;
	m_iRequestBodySize = 0;
	m_Handle = nullptr;
	m_pHeaderList = nullptr;
	m_pShareHandle = nullptr;

	m_iSteamID64 = steamID64;
	m_iSlot = slot;

	if (body && (bodySize > 0))
	{
		m_iRequestBodySize = bodySize;
		m_sRequestBody = new char[bodySize];
		memcpy(m_sRequestBody, body, m_iRequestBodySize);
	}

	m_sResponseBody.clear();
}

HTTPRequest::~HTTPRequest()
{
	Cleanup();
}

void HTTPRequest::Cleanup()
{
	delete[] m_sRequestBody;
	m_sRequestBody = nullptr;
	m_sRequestBuffer.clear();

	m_sResponseBody.clear();

	if (m_pHeaderList)
	{
		curl_slist_free_all(m_pHeaderList);
		m_pHeaderList = nullptr;
	}
	
	// just incase it's not cleaned up already.
	if (m_Handle)
	{
		curl_easy_cleanup(m_Handle);
		m_Handle = nullptr;
	}
}

void HTTPRequest::SetupRequest()
{
	// might be better to use assert here instead.
	if (!m_Handle)
		return;

	switch (m_eHTTPMethod)
	{
		case HTTPRequest::GET:
			break;
		case HTTPRequest::POST:
			curl_easy_setopt(m_Handle, CURLOPT_POST, 1);
			break;
		case HTTPRequest::DEL:
			curl_easy_setopt(m_Handle, CURLOPT_CUSTOMREQUEST, "DELETE");
			break;
		case HTTPRequest::PUT:
			curl_easy_setopt(m_Handle, CURLOPT_CUSTOMREQUEST, "PUT");
			break;
	}
	
	curl_easy_setopt(m_Handle, CURLOPT_URL, m_sPchAPIUrl);
	curl_easy_setopt(m_Handle, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(m_Handle, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(m_Handle, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(m_Handle, CURLOPT_USERAGENT, "MSR Game Server");

	if (m_pShareHandle)
		curl_easy_setopt(m_Handle, CURLOPT_SHARE, m_pShareHandle);

	if (m_sRequestBody != nullptr)
	{
		m_pHeaderList = curl_slist_append(m_pHeaderList, "Content-Type: application/json; charset=UTF-8");
		curl_easy_setopt(m_Handle, CURLOPT_HTTPHEADER, m_pHeaderList);

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
		writer.String(base64_encode(reinterpret_cast<byte*>(m_sRequestBody), m_iRequestBodySize).c_str());

		writer.EndObject();

		std::string buffer = s.GetString();
		m_sRequestBuffer = buffer; //we have to make a copy of the data because it will go out of scope otherwise.
		curl_easy_setopt(m_Handle, CURLOPT_POSTFIELDSIZE, m_sRequestBuffer.size());
		curl_easy_setopt(m_Handle, CURLOPT_POSTFIELDS, m_sRequestBuffer.c_str());
	}
}

// This is a blocking call.
bool HTTPRequest::SendRequest()
{
	if (m_Handle) 
		return false;
		
	m_iRequestState = RequestState::EXECUTED;
	m_Handle = curl_easy_init();
	SetupRequest();
	
	curl_easy_setopt(m_Handle, CURLOPT_WRITEFUNCTION, HTTPRequest::WriteCallbackDispatcher);
	curl_easy_setopt(m_Handle, CURLOPT_WRITEDATA, this);
	CURLcode curlResult = curl_easy_perform(m_Handle);
	bool result = false;
	if (curlResult == CURLE_OK)
		result = true;
	else
		result = false;

	curl_easy_cleanup(m_Handle);
	m_Handle = nullptr;
	return result;
}

// This will pause the main thread until the async thread finishes it's task.
// This will return true/false if the async thread completed it's task sucessfully.
bool HTTPRequest::AsyncSendRequest()
{
	if (m_Handle)
		return false;

	m_Handle = curl_easy_init();
	SetupRequest();

	std::future<bool> future = std::async(std::launch::async, &HTTPRequest::PerformRequest, this);

	bool result = false;
	if (future.get() == true)
	{
		int httpCode = 200;
		curl_easy_getinfo(m_Handle, CURLINFO_RESPONSE_CODE, &httpCode);
		ResponseCallback(httpCode);
		result = true;
	}else{
		ResponseCallback(0);
		result = false;
	}
	
	return result;
}

CURL* HTTPRequest::PrepareForMulti()
{
	if (m_Handle)
		return nullptr;

	m_Handle = curl_easy_init();
	if (!m_Handle)
		return nullptr;

	SetupRequest();

	curl_easy_setopt(m_Handle, CURLOPT_WRITEFUNCTION, HTTPRequest::WriteCallbackDispatcher);
	curl_easy_setopt(m_Handle, CURLOPT_WRITEDATA, this);

	// Store a back-pointer so we can recover 'this' from a completed easy handle.
	curl_easy_setopt(m_Handle, CURLOPT_PRIVATE, this);

	m_iRequestState = RequestState::EXECUTED;
	return m_Handle;
}

// Called by CRequestManager when curl_multi reports this transfer finished.
void HTTPRequest::OnMultiComplete(CURLcode result)
{
	if (result == CURLE_OK)
	{
		int httpCode = 200;
		curl_easy_getinfo(m_Handle, CURLINFO_RESPONSE_CODE, &httpCode);
		ResponseCallback(httpCode);
	}
	else
	{
		ResponseCallback(0);
	}

	curl_easy_cleanup(m_Handle);
	m_Handle = nullptr;
}

bool HTTPRequest::PerformRequest()
{
	if (!m_Handle) 
		return false;

	curl_easy_setopt(m_Handle, CURLOPT_WRITEFUNCTION, HTTPRequest::WriteCallbackDispatcher);
	curl_easy_setopt(m_Handle, CURLOPT_WRITEDATA, this);
	CURLcode result = curl_easy_perform(m_Handle);
	m_iRequestState = RequestState::EXECUTED;
	if (result == CURLE_OK)
		return true;
	else
		return false;
}

size_t HTTPRequest::WriteCallbackDispatcher(void* buf, size_t sz, size_t n, void* curlGet)
{
	return static_cast<HTTPRequest*>(curlGet)->WriteCallback(buf, sz, n);
}

size_t HTTPRequest::WriteCallback(void* ptr, size_t size, size_t nmemb)
{
	m_sResponseBody.append((char*)ptr, size * nmemb);
	return size * nmemb;
}

void HTTPRequest::ResponseCallback(int httpCode)
{
	if (m_bNoCallback)
	{
		m_iRequestState = RequestState::FINISHED;
		return;
	}
	
	if (httpCode == 0)
	{
		FNShared::Print("Request Failed. %s, '%s'\n", GetName(), g_szBaseUrl);
		m_iRequestState = RequestState::FINISHED;
		return;
	}

	if (httpCode < 200 || httpCode > 299)
	{
		if (httpCode == 401)
		{
			FNShared::Print("FN Authorization failed! %s\n", GetName());
			OnResponse(httpCode);
			m_iRequestState = RequestState::FINISHED;
			return;
		}

		FNShared::Print("FN Server Error. %s Code: %d\n", GetName(), httpCode);
		OnResponse(httpCode);
		m_iRequestState = RequestState::FINISHED;
		return;
	}

	if (httpCode == 204)
	{
		OnResponse(httpCode);
		m_iRequestState = RequestState::FINISHED;
		return;
	}

	if (m_sResponseBody.empty())
	{
		FNShared::Print("The data hasn't been received. HTTP code: %d\n", httpCode);
		OnResponse(httpCode);
		m_iRequestState = RequestState::FINISHED;
		return;
	}

	OnResponse();
	
	m_iRequestState = RequestState::FINISHED;
}

JSONDocument HTTPRequest::ParseJSON(const char* data)
{
	JSONDocument doc;

	if (!data)
		return doc;

	doc.Parse(data);

	return doc;
}

/* static */ void HTTPRequest::SetBaseURL(const char* url)
{
	_snprintf(g_szBaseUrl, REQUEST_URL_SIZE, "%s", url);
}