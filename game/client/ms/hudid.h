#ifndef __HUDID
#define __HUDID
#include "hudbase.h"
struct entinfo_t;

class CHudID : public CHudBase
{
public:
	int Init(void);
	int Draw(float flTime);
	void Reset(void);
	void InitHUDData(void);
	int MsgFunc_EntInfo(const char *pszName, int iSize, void *pbuf);

	void SearchThink(void);
	entinfo_t *GetEntInFrontOfMe(float Range);

	entinfo_t *pActiveInfo, *pDrawInfo;
	float Alpha;
	float TimeDecAlpha;
};
#endif //__HUDID
