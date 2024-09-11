#ifndef MSBASIC_H
#define MSBASIC_H

#include "buildcontrol.h"

#ifdef TRACK_INPUTS
#define DBG_INPUT DbgInputs(__FUNCTION__, __FILE__, __LINE__)
void DbgInputs(const char *Function, const char *File, long Line);
#else
#define DBG_INPUT
#endif

#endif