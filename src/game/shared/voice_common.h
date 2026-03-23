//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#ifndef VOICE_COMMON_H
#define VOICE_COMMON_H
#ifdef _WIN32
#pragma once
#endif

#include "bitvec.h"

constexpr unsigned int VOICE_MAX_PLAYERS = 32; // (todo: this should just be set to MAX_CLIENTS).
constexpr unsigned int VOICE_MAX_PLAYERS_DW = ((VOICE_MAX_PLAYERS / 32) + !!(VOICE_MAX_PLAYERS & 31));

typedef CBitVec<VOICE_MAX_PLAYERS> CPlayerBitVec;

#endif // VOICE_COMMON_H
