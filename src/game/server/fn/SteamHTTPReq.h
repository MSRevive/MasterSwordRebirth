//
// Steam HTTP Request Handler Class
//

#ifndef HTTP_BASE_REQUEST_H
#define HTTP_BASE_REQUEST_H

#include "rapidjson/fwd.h" // Rapid JSON Helpers from Infestus!
#include "steam/steam_api.h"
#include "steam/isteamhttp.h"

#define REQUEST_URL_SIZE 256
#define HTTP_CONTENT_TYPE "application/json"
#define ID64 unsigned long long

JSONDocument* ParseJSON(const char* data, size_t length = 0);

enum RequestState
{
	REQUEST_QUEUED = 0,
	REQUEST_EXECUTED,
	REQUEST_FINISHED,
};

class SteamHttpRequest
{
public:
	SteamHttpRequest(EHTTPMethod method, const char* url, uint8* body = NULL, size_t bodySize = 0, ID64 param1 = 0ULL, ID64 param2 = 0ULL);
	virtual ~SteamHttpRequest();

	virtual const char* GetName() { return "N/A"; }
	virtual void OnResponse(bool bSuccessful) { }

	static void SetBaseUrl(const char* url);
	static void Think(void);
	static void SendAndWait(void);

protected: // Expose data to inheriting classes.
	char pchApiUrl[REQUEST_URL_SIZE];

	uint8* requestBody;
	size_t requestBodySize;

	uint8* responseBody;
	size_t responseBodySize;

	JSONDocument* pJSONData;

	ID64 param1;
	ID64 param2;

private: // Keep this private.
	void SendRequest();
	void Cleanup();
	void ReleaseHandle();
	void OnHTTPRequestCompleted(HTTPRequestCompleted_t* p, bool bError);

	CCallResult<SteamHttpRequest, HTTPRequestCompleted_t> m_CallbackOnHTTPRequestCompleted;
	HTTPRequestHandle handle;
	EHTTPMethod httpMethod;
	int requestState;

private:
	SteamHttpRequest(const SteamHttpRequest&); // No copy-constructor pls.
};

#endif // HTTP_BASE_REQUEST_H