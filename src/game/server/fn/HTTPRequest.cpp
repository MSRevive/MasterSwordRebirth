//
// Steam HTTP Request Handler Class
//

#include "rapidjson/document_safe.h"
#include "base64/base64.h"
#include "HTTPRequest.h"
#include "FNShareddefs.h"
#include "msdllheaders.h"
#include "global.h"
#include "player.h"
#include <string>

static bool g_bSuppressResponse = false;
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
	_snprintf(pchApiUrl, REQUEST_URL_SIZE, "%s%s", g_szBaseUrl, url);

	requestBody = responseBody = NULL;
	requestBodySize = responseBodySize = 0;
	pJSONData = nullptr;
	handle = NULL;

	this->steamID64 = steamID64;
	this->slot = slot;

	if ((body != NULL) && (bodySize > 0))
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
	handle = steamHTTP->CreateHTTPRequest(httpMethod, pchApiUrl);
	if (handle == NULL)
	{
		Cleanup();
		return;
	}

	if (requestBody != NULL)
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

		std::string buffer = s.GetString();

		steamHTTP->SetHTTPRequestRawPostBody(handle, HTTP_CONTENT_TYPE, (uint8*)buffer.data(), buffer.length());
	}

	SteamAPICall_t apiCall = NULL;
	if (steamHTTP->SendHTTPRequest(handle, &apiCall) && apiCall)
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

	steamHTTP = nullptr;

	ReleaseHandle();
}

void HTTPRequest::ReleaseHandle()
{
	if (handle)
	{
		steamHTTP->ReleaseHTTPRequest(handle);
		handle = NULL;
	}
	requestState = RequestState::REQUEST_FINISHED;
}

void HTTPRequest::OnHTTPRequestCompleted(HTTPRequestCompleted_t* p, bool bError)
{
	if (g_bSuppressResponse || (handle == NULL) || (p == nullptr) || (p->m_hRequest != handle))
	{
		ReleaseHandle();
		return;
	}

	size_t unBytes = 0;
	if (!bError && (responseBody == nullptr) && steamHTTP->GetHTTPResponseBodySize(handle, &unBytes) && (unBytes != 0))
	{
		responseBodySize = unBytes;
		responseBody = new uint8[responseBodySize];
		if (steamHTTP->GetHTTPResponseBodyData(handle, responseBody, unBytes))
			pJSONData = ParseJSON((char*)responseBody, responseBodySize);
	}

	if (bError)
		FNShared::Print("Error: %s '%s'\n", GetName(), pchApiUrl);

	OnResponse(bError == false);
	ReleaseHandle();
}

/* static */ void HTTPRequest::SetBaseURL(const char* url)
{
	_snprintf(g_szBaseUrl, REQUEST_URL_SIZE, "%s", url);
}