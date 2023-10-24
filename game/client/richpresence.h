#pragma once

class CRichPresence
{
public:
	CRichPresence() = default;
	~CRichPresence() = default;

	void Init();
	void Shutdown();
	void Update();

private:
	bool m_bLoaded = false;
	float m_fLastUpdate = 0.0f;
	int64_t m_numStartTime;

	char m_cBufferDetails[128];
	char m_cBufferState[128];
};

extern CRichPresence gRichPresence;