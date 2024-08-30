//
// Steam HTTP Request Handler Class
//

#include "rapidjson/document_safe.h"
#include "base64/base64.h"
#include "SteamHttpRequest.h"
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
		return NULL;

	JSONDocument* document = new JSONDocument;

	if (length > 0)
		document->Parse(data, length);
	else
		document->Parse(data);

	if (document->HasParseError())
	{
		delete document;
		return NULL;
	}

	return document;
}

SteamHttpRequest::SteamHttpRequest(EHTTPMethod method, const char* url, uint8* body, size_t bodySize, ID64 param1, ID64 param2)
{
	g_FNRequestManager.Queue(this);

	httpMethod = method;
	requestState = REQUEST_QUEUED;
	_snprintf(pchApiUrl, REQUEST_URL_SIZE, "%s%s", g_szBaseUrl, url);

	requestBody = responseBody = NULL;
	requestBodySize = responseBodySize = 0;
	pJSONData = NULL;
	handle = NULL;

	this->param1 = param1;
	this->param2 = param2;

	if ((body != NULL) && (bodySize > 0))
	{
		requestBodySize = bodySize;
		requestBody = new uint8[requestBodySize];
		memcpy(requestBody, body, requestBodySize);
	}
}

SteamHttpRequest::~SteamHttpRequest()
{
	Cleanup();
}

void SteamHttpRequest::SendRequest()
{
	requestState = REQUEST_EXECUTED;
	handle = g_FNRequestManager.GetHTTPContext()->CreateHTTPRequest(httpMethod, pchApiUrl);
	if (handle == NULL)
	{
		Cleanup();
		return;
	}

	if (requestBody != NULL)
	{
		char steamID64String[REQUEST_URL_SIZE];
		_snprintf(steamID64String, REQUEST_URL_SIZE, "%llu", param1);

		rapidjson::StringBuffer s;
		rapidjson::Writer<rapidjson::StringBuffer> writer(s);

		writer.StartObject();

		writer.Key("steamid");
		writer.String(steamID64String);

		writer.Key("slot");
		writer.Int(param2);

		writer.Key("size");
		writer.Int(requestBodySize);

		writer.Key("data");
		writer.String(base64_encode(requestBody, requestBodySize).c_str());

		writer.EndObject();

		std::string buffer = s.GetString();

		g_FNRequestManager.GetHTTPContext()->SetHTTPRequestRawPostBody(handle, HTTP_CONTENT_TYPE, (uint8*)buffer.data(), buffer.length());
	}

	SteamAPICall_t apiCall = NULL;
	if (g_FNRequestManager.GetHTTPContext()->SendHTTPRequest(handle, &apiCall) && apiCall)
		m_CallbackOnHTTPRequestCompleted.Set(apiCall, this, &SteamHttpRequest::OnHTTPRequestCompleted);
	else
		Cleanup();
}

void SteamHttpRequest::Cleanup()
{
	delete[] requestBody;
	delete[] responseBody;

	requestBody = responseBody = NULL;
	requestBodySize = responseBodySize = 0;

	delete pJSONData;
	pJSONData = NULL;

	ReleaseHandle();
}

void SteamHttpRequest::ReleaseHandle()
{
	if (handle)
	{
		g_FNRequestManager.GetHTTPContext()->ReleaseHTTPRequest(handle);
		handle = NULL;
	}
	requestState = REQUEST_FINISHED;
}

void SteamHttpRequest::OnHTTPRequestCompleted(HTTPRequestCompleted_t* p, bool bError)
{
	if (g_bSuppressResponse || (handle == NULL) || (p == NULL) || (p->m_hRequest != handle))
	{
		ReleaseHandle();
		return;
	}

	size_t unBytes = 0;
	if (!bError && (responseBody == NULL) && g_FNRequestManager.GetHTTPContext()->GetHTTPResponseBodySize(handle, &unBytes) && (unBytes != 0))
	{
		responseBodySize = unBytes;
		responseBody = new uint8[responseBodySize];
		if (g_FNRequestManager.GetHTTPContext()->GetHTTPResponseBodyData(handle, responseBody, unBytes))
			pJSONData = ParseJSON((char*)responseBody, responseBodySize);
	}

	if (bError)
		FNShared::Print("Error: %s '%s'\n", GetName(), pchApiUrl);

	OnResponse(bError == false);
	ReleaseHandle();
}

/*static*/ void SteamHttpRequest::SetBaseUrl(const char* url)
{
	_snprintf(g_szBaseUrl, REQUEST_URL_SIZE, "%s", url);
}