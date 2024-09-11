/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  parsemsg.h
//

#ifndef DEBUG
#define ASSERT(x)
#endif

#define READ_BOOL() ( READ_BYTE() == 1 )		// MIB FEB2019_23 [LOCAL_PANEL_IMAGE]

void BEGIN_READ(void *buf, int size);
int READ_CHAR(void);
int READ_BYTE(void);
int READ_SHORT(void);
int READ_WORD(void);
int READ_LONG(void);
float READ_FLOAT(void);
char *READ_STRING(void);
float READ_COORD(void);
float READ_ANGLE(void);
float READ_HIRESANGLE(void);

//Master Sword
void READ_REWIND_LONG(void);
void READ_REWIND_CHAR(void);
void READ_REWIND_SHORT(void);
void READ_REWIND_COORD(void);
