//
// Steam HTTP Request Handler Class
//

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include "base64/base64.h"
#include "HTTPRequest.h"
#include "FNSharedDefs.h"
#include "msdllheaders.h"
#include "global.h"
#include "player.h"
#include "SteamServerHelper.h"
#include "FNSharedDefs.h"
#include <string>

static char g_szBaseUrl[REQUEST_URL_SIZE];

JSONDocument* ParseJSON(const char* data, size_t length)
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

HTTPRequest::HTTPRequest(EHTTPMethod method, const char* url, uint8* body, size_t bodySize, ID64 steamID64, ID64 slot)
{
	httpMethod = method;
	requestState = RequestState::REQUEST_QUEUED;
	_snprintf(pchApiUrl, REQUEST_URL_SIZE, "http://%s%s", g_szBaseUrl, url);

	requestBody = responseBody = nullptr;
	requestBodySize = responseBodySize = 0;
	pJSONData = nullptr;
	handle = NULL;

	this->steamID64 = steamID64;
	this->slot = slot;

	if ((body != nullptr) && (bodySize > 0))
	{
		requestBodySize = bodySize;
		requestBody = new uint8[requestBodySize];
		memcpy(requestBody, body, requestBodySize);
	}
}

HTTPRequest::~HTTPRequest()
{
	Cleanup();
}

void HTTPRequest::SendRequest()
{
	requestState = RequestState::REQUEST_EXECUTED;
	handle = g_SteamHTTPContext->CreateHTTPRequest(httpMethod, pchApiUrl);
	g_SteamHTTPContext->SetHTTPRequestHeaderValue(handle, "Cache-Control", "no-cache");
	g_SteamHTTPContext->SetHTTPRequestHeaderValue(handle, "User-Agent", "MSRebith SteamHTTP");
	if (handle == NULL)
	{
		Cleanup();
		return;
	}

	if (requestBody != nullptr)
	{
		char steamID64String[REQUEST_URL_SIZE];
		_snprintf(steamID64String, REQUEST_URL_SIZE, "%llu", steamID64);

		rapidjson::StringBuffer s;
		rapidjson::Writer<rapidjson::StringBuffer> writer(s);

		writer.StartObject();

		writer.Key("steamid");
		writer.String(steamID64String);

		writer.Key("slot");
		writer.Int(slot);

		writer.Key("size");
		writer.Int(requestBodySize);

		writer.Key("data");
		writer.String(base64_encode(requestBody, requestBodySize).c_str());

		writer.EndObject();

		const char* buffer = s.GetString();

		g_SteamHTTPContext->SetHTTPRequestRawPostBody(handle, HTTP_CONTENT_TYPE, (uint8*)buffer, strlen(buffer));
	}

	SteamAPICall_t apiCall = k_uAPICallInvalid;
	if (g_SteamHTTPContext->SendHTTPRequest(handle, &apiCall) && apiCall)
		m_CallbackOnHTTPRequestCompleted.Set(apiCall, this, &HTTPRequest::OnHTTPRequestCompleted);
	else
		Cleanup();
}

void HTTPRequest::Cleanup()
{
	delete[] requestBody;
	delete[] responseBody;

	requestBody = responseBody = nullptr;
	requestBodySize = responseBodySize = 0;

	delete pJSONData;
	pJSONData = nullptr;

	ReleaseHandle();
}

void HTTPRequest::ReleaseHandle()
{
	if (handle)
	{
		g_SteamHTTPContext->ReleaseHTTPRequest(handle);
		handle = NULL;
	}
	requestState = RequestState::REQUEST_FINISHED;
}

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

		// Specifically let the children handle 204 errors so they can do what they want.
		if (p->m_eStatusCode == 204)
		{
			OnResponse(true, p->m_eStatusCode);
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
}

/* static */ void HTTPRequest::SetBaseURL(const char* url)
{
	_snprintf(g_szBaseUrl, REQUEST_URL_SIZE, "%s", url);
}