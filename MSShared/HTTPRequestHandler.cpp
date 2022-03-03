//
// Simple HTTP Request handling using cUrl.
//

#include "HTTPRequestHandler.h"
#include "curl/curl.h"
#include "rapidjson/document_safe.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#if defined(_WIN32) && defined(USE_VS2022)
FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }
#endif

static char g_pDataBuffer[REQUEST_BUFFER_SIZE];

static size_t DataCallbackEvent(char* buf, size_t size, size_t nmemb, void* up)
{
	strncat(g_pDataBuffer, buf, REQUEST_BUFFER_SIZE - strlen(g_pDataBuffer) - 1);
	return (size * nmemb);
}

bool HTTPRequestHandler::GetRequest(const char* url)
{
	g_pDataBuffer[0] = 0;

	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &DataCallbackEvent);
	CURLcode result = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return (result == CURLE_OK);
}

bool HTTPRequestHandler::PostRequest(const char* url, const char* body)
{
	g_pDataBuffer[0] = 0;

	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &DataCallbackEvent);
	CURLcode result = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return (result == CURLE_OK);
}

bool HTTPRequestHandler::PutRequest(const char* url, const char* body)
{
	g_pDataBuffer[0] = 0;

	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &DataCallbackEvent);
	CURLcode result = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return (result == CURLE_OK);
}

bool HTTPRequestHandler::DeleteRequest(const char* url)
{
	g_pDataBuffer[0] = 0;

	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &DataCallbackEvent);
	CURLcode result = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return (result == CURLE_OK);
}

const char* HTTPRequestHandler::GetRequestResponse(void) { return g_pDataBuffer; }

// Parses a JSON formatted char array, returns a JSON document, see https://rapidjson.org/index.html for documentation!
JSONDocument* HTTPRequestHandler::ParseJSON(const char* data)
{
	if (!(data && data[0]))
		return NULL;

	JSONDocument* document = new JSONDocument;
	document->Parse(data);
	if (document->HasParseError())
	{
		delete document;
		return NULL;
	}

	return document;
}

JSONDocument* HTTPRequestHandler::GetRequestAsJson(const char* url)
{
	return (GetRequest(url) ? ParseJSON(g_pDataBuffer) : NULL);
}

JSONDocument* HTTPRequestHandler::PostRequestAsJson(const char* url, const char* body)
{
	return (PostRequest(url, body) ? ParseJSON(g_pDataBuffer) : NULL);
}

JSONDocument* HTTPRequestHandler::PutRequestAsJson(const char* url, const char* body)
{
	return (PutRequest(url, body) ? ParseJSON(g_pDataBuffer) : NULL);
}

JSONDocument* HTTPRequestHandler::DeleteRequestAsJson(const char* url)
{
	return (DeleteRequest(url) ? ParseJSON(g_pDataBuffer) : NULL);
}

void HTTPRequestHandler::PrintJSONDocument(const JSONDocument* pDocument)
{
	if (pDocument == NULL) return;

	rapidjson::StringBuffer buff;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buff);
	pDocument->Accept(writer);

	printf("%s\n", buff.GetString());
}