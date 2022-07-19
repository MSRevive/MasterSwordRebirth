#ifndef HTTP_REQUEST_HANDLER_H
#define HTTP_REQUEST_HANDLER_H

#include "rapidjson/fwd.h" // Rapid JSON Helpers from Infestus!

namespace HTTPRequestHandler
{
	void Initialize(const char* url);
	void Destroy(void);

	bool GetRequest(const char* url);
	bool PostRequest(const char* url, const char* body);
	bool PutRequest(const char* url, const char* body);
	bool DeleteRequest(const char* url);

	JSONDocument* ParseJSON(const char* data);
	JSONDocument* GetRequestAsJson(const char* url);
	JSONDocument* PostRequestAsJson(const char* url, const char* body);
	JSONDocument* PutRequestAsJson(const char* url, const char* body);
	JSONDocument* DeleteRequestAsJson(const char* url);
}

#endif // HTTP_REQUEST_HANDLER_H