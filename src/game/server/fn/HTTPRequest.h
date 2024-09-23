//
// Steam HTTP Request Handler Class
//

#ifndef HTTP_BASE_REQUEST_H
#define HTTP_BASE_REQUEST_H

#include <rapidjson/fwd.h> // Rapid JSON Helpers from Infestus!
#include <steam/steam_api.h>
#include <steam/isteamhttp.h>

#define REQUEST_URL_SIZE 256
#define HTTP_CONTENT_TYPE "application/json"
#define ID64 unsigned long long

JSONDocument* ParseJSON(const char* data, size_t length = 0);

class HTTPRequest
{
public:
	HTTPRequest(EHTTPMethod method, const char* url, uint8* body = NULL, size_t bodySize = 0, ID64 steamID64 = 0ULL, ID64 slot = 0ULL);
	virtual ~HTTPRequest();

	virtual const char* GetName() { return "N/A"; }
	virtual void OnResponse(bool bSuccessful) { }

	static void SetBaseURL(const char* url);

	void SendRequest();
	void SuppressResponse(bool suppressResp) { this->suppressResponse = suppressResp; }

	int requestState;

	enum RequestState
	{
		REQUEST_QUEUED = 0,
		REQUEST_EXECUTED,
		REQUEST_FINISHED,
	};

protected: // Expose data to inheriting classes.
	char pchApiUrl[REQUEST_URL_SIZE];

	uint8* requestBody;
	size_t requestBodySize;

	uint8* responseBody;
	size_t responseBodySize;

	JSONDocument* pJSONData;

	ID64 steamID64;
	ID64 slot;

	bool bPriorityReq = false;

private: // Keep this private.
	void Cleanup();
	void ReleaseHandle();
	
	void OnHTTPRequestCompleted(HTTPRequestCompleted_t* p, bool bError);

	CCallResult<HTTPRequest, HTTPRequestCompleted_t> m_CallbackOnHTTPRequestCompleted;
	HTTPRequestHandle handle;
	EHTTPMethod httpMethod;

	bool suppressResponse = false;

private:
	HTTPRequest(const HTTPRequest&); // No copy-constructor pls.
};

#endif // HTTP_BASE_REQUEST_H