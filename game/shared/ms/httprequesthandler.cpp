//
// Simple HTTP Request handling using httplib (https://github.com/yhirose/cpp-httplib)!
//

#include "httplib/httplib.h"
#include "httprequesthandler.h"
#include "rapidjson/document_safe.h"
#include <string>

#define HTTP_CODE_OK 200

static std::string g_pDataBuffer;
static httplib::Client* g_pHttpClient = NULL;

static bool IsValidResponse(const httplib::Result& result)
{
	g_pDataBuffer.clear();
	if (result && (result->status == HTTP_CODE_OK))
	{
		g_pDataBuffer = result->body;
		return true;
	}
	return false;
}

void HTTPRequestHandler::Initialize(const char* url)
{
	if (g_pHttpClient != NULL)
		return;

	g_pHttpClient = new httplib::Client(url);
}

void HTTPRequestHandler::Destroy(void)
{
	delete g_pHttpClient;
	g_pHttpClient = NULL;
}

bool HTTPRequestHandler::GetRequest(const char* url)
{
	auto result = g_pHttpClient->Get(url);
	return IsValidResponse(result);
}

bool HTTPRequestHandler::PostRequest(const char* url, const char* body)
{
	auto result = g_pHttpClient->Post(url, body, "application/json");
	return IsValidResponse(result);
}

bool HTTPRequestHandler::PutRequest(const char* url, const char* body)
{
	auto result = g_pHttpClient->Put(url, body, "application/json");
	return IsValidResponse(result);
}

bool HTTPRequestHandler::DeleteRequest(const char* url)
{
	auto result = g_pHttpClient->Delete(url);
	return IsValidResponse(result);
}

// Parses a JSON formatted char array, returns a JSON document
// see https://rapidjson.org/index.html for documentation!
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
	return (GetRequest(url) ? ParseJSON(g_pDataBuffer.c_str()) : NULL);
}

JSONDocument* HTTPRequestHandler::PostRequestAsJson(const char* url, const char* body)
{
	return (PostRequest(url, body) ? ParseJSON(g_pDataBuffer.c_str()) : NULL);
}

JSONDocument* HTTPRequestHandler::PutRequestAsJson(const char* url, const char* body)
{
	return (PutRequest(url, body) ? ParseJSON(g_pDataBuffer.c_str()) : NULL);
}

JSONDocument* HTTPRequestHandler::DeleteRequestAsJson(const char* url)
{
	return (DeleteRequest(url) ? ParseJSON(g_pDataBuffer.c_str()) : NULL);
}