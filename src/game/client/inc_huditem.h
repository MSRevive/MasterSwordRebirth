#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#define INC_HUDITEM_H

void Print(char *szFmt, ...);
char *UTIL_VarArgs(char *format, ...);
inline const char *Localized(const char *pszText) { return CHudTextMessage::BufferedLocaliseTextString(pszText); }
