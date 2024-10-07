#ifndef INC_HUDITEM_H
#define INC_HUDITEM_H

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

void Print(const char *szFmt, ...);
char* UTIL_VarArgs(const char *format, ...);
inline const char* Localized(const char *pszText) { return CHudTextMessage::BufferedLocaliseTextString(pszText); }

#endif