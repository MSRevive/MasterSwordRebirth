//
// Steam HTTP Request Handler Class
//

#ifndef HTTP_BASE_REQUEST_H
#define HTTP_BASE_REQUEST_H

#include <rapidjson/fwd.h> // Rapid JSON Helpers from Infestus!

#define REQUEST_URL_SIZE 512
#define HTTP_CONTENT_TYPE "application/json"
#define ID64 unsigned long long

enum HTTPMethod {
	GET
	POST
	DELETE
	PUT
};

class HTTPRequest
{
public:
	HTTPRequest(HTTPMethod method, const char* url, uint8* body = nullptr, size_t bodySize = 0, ID64 steamID64 = 0ULL, ID64 slot = 0ULL);
	virtual ~HTTPRequest();

	virtual const char* GetName() { return "N/A"; }
	virtual void OnResponse(bool bSuccessful, int iRespCode = 200) { }

	static void SetBaseURL(const char* url);

	bool SendRequest();
	void SuppressResponse(bool suppressResp) { m_bSuppressResponse = suppressResp; }

	int m_iRequestState;

	enum RequestState
	{
		REQUEST_QUEUED = 0,
		REQUEST_EXECUTED,
		REQUEST_FINISHED,
	};

protected: // Expose data to inheriting classes.
	char m_sPchAPIUrl[REQUEST_URL_SIZE];

	uint8* m_sRequestBody;
	size_t m_iRequestBodySize;

	std::string m_sResponseBody;
	size_t m_iResponseBodySize;

	JSONDocument* m_pJSONData;

	ID64 m_iSteamID64;
	ID64 m_iSlot;

private: // Keep this private.
	void Cleanup();
	void DataCallbackEvent(char* buf, size_t size, size_t nmemb, void* up);
	void ResponseCallback(int httpCode);
	JSONDocument* ParseJSON(const char* data, size_t length = 0);

	HTTPMethod m_eHTTPMethod;

	bool m_bSuppressResponse = false;

private:
	HTTPRequest(const HTTPRequest&); // No copy-constructor pls.
};

#endif // HTTP_BASE_REQUEST_H