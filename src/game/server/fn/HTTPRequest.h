//
// Steam HTTP Request Handler Class
//

#ifndef HTTP_BASE_REQUEST_H
#define HTTP_BASE_REQUEST_H

#include <string>
#include <rapidjson/fwd.h> // Rapid JSON Helpers from Infestus!
#include <curl/curl.h>
#ifdef _WIN32
#include <PlatformWin.h>
#endif
#include <Platform.h>

#define REQUEST_URL_SIZE 512
#define HTTP_CONTENT_TYPE "application/json"
#define ID64 unsigned long long
//using uint8 = unsigned char; // Same thing as byte

class HTTPRequest
{
public:
	enum HTTPMethod {
		GET = 0,
		POST,
		DEL, //DELETE is reserved by windows.
		PUT
	};

	enum RequestState
	{
		QUEUED = 0,
		EXECUTED,
		FINISHED,
	};

	HTTPRequest(HTTPMethod method, const char* url, const char* body = nullptr, size_t bodySize = 0, ID64 steamID64 = 0ULL, ID64 slot = 0ULL);
	virtual ~HTTPRequest();

	virtual const char* GetName() { return "N/A"; }
	virtual void OnResponse(int iRespCode = 200) { }

	static void SetBaseURL(const char* url);

	bool SendRequest();
	bool AsyncSendRequest();
	void AsyncSendRequestDiscard();
	static JSONDocument ParseJSON(const char* data);

	int m_iRequestState;
	bool m_bNoCallback = false;
	std::string m_sResponseBody;

protected: // Expose data to inheriting classes.
	char m_sPchAPIUrl[REQUEST_URL_SIZE];

	char* m_sRequestBody;
	size_t m_iRequestBodySize;
	std::string m_sRequestBuffer;

	ID64 m_iSteamID64;
	ID64 m_iSlot;

private: // Keep this private.
	static size_t WriteCallbackDispatcher(void* buf, size_t sz, size_t n, void* curlGet);
	size_t WriteCallback(void* ptr, size_t size, size_t nmemb);

	void ResponseCallback(int httpCode);
	void Cleanup();

	void SetupRequest();
	bool PerformRequest();

	HTTPMethod m_eHTTPMethod;
	CURL* m_Handle;

	//std::promise<bool> m_Promise;

private:
	HTTPRequest(const HTTPRequest&); // No copy-constructor pls.
};

#endif // HTTP_BASE_REQUEST_H