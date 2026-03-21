/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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




#ifndef _STUDIO_H_
#define _STUDIO_H_

/*
==============================================================================

STUDIO MODELS

Studio models are position independent, so the cache manager can move them.
==============================================================================
*/


constexpr int MAXSTUDIOTRIANGLES	= 20000;	// TODO: tune this
constexpr int MAXSTUDIOVERTS		= 2048;		// TODO: tune this
constexpr int MAXSTUDIOSEQUENCES	= 2048;		// total animation sequences
constexpr int MAXSTUDIOSKINS		= 100;		// total textures
constexpr int MAXSTUDIOSRCBONES		= 512;		// bones allowed at source movement
constexpr int MAXSTUDIOBONES		= 128;		// total bones actually used
constexpr int MAXSTUDIOMODELS		= 32;		// sub-models per model
constexpr int MAXSTUDIOBODYPARTS	= 32;
constexpr int MAXSTUDIOGROUPS		= 16;
constexpr int MAXSTUDIOANIMATIONS	= 2048;		// per sequence
constexpr int MAXSTUDIOMESHES		= 256;
constexpr int MAXSTUDIOEVENTS		= 1024;
constexpr int MAXSTUDIOPIVOTS		= 256;
constexpr int MAXSTUDIOCONTROLLERS	= 8;

typedef struct 
{
	int					id;
	int					version;

	char				name[64];
	int					length;

	Vector 				eyeposition;	// ideal eye position
	Vector 				min;			// ideal movement hull size
	Vector 				max;			

	Vector 				bbmin;			// clipping bounding box
	Vector 				bbmax;		

	int					flags;

	int					numbones;			// bones
	int					boneindex;

	int					numbonecontrollers;		// bone controllers
	int					bonecontrollerindex;

	int					numhitboxes;			// complex bounding boxes
	int					hitboxindex;			
	
	unsigned int					numseq;				// animation sequences
	int					seqindex;

	int					numseqgroups;		// demand loaded sequences
	int					seqgroupindex;

	int					numtextures;		// raw textures
	int					textureindex;
	int					texturedataindex;

	int					numskinref;			// replaceable textures
	int					numskinfamilies;
	int					skinindex;

	int					numbodyparts;		
	int					bodypartindex;

	int					numattachments;		// queryable attachable points
	int					attachmentindex;

	int					soundtable;
	int					soundindex;
	int					soundgroups;
	int					soundgroupindex;

	int					numtransitions;		// animation node to animation node transition graph
	int					transitionindex;
} studiohdr_t;

// header for demand loaded sequence group data
typedef struct 
{
	int					id;
	int					version;

	char				name[64];
	int					length;
} studioseqhdr_t;

// bones
typedef struct 
{
	char				name[32];	// bone name for symbolic links
	int		 			parent;		// parent bone
	int					flags;		// ??
	int					bonecontroller[6];	// bone controller index, -1 == none
	float				value[6];	// default DoF values
	float				scale[6];   // scale for delta DoF values
} mstudiobone_t;


// bone controllers
typedef struct 
{
	int					bone;	// -1 == 0
	int					type;	// X, Y, Z, XR, YR, ZR, M
	float				start;
	float				end;
	int					rest;	// byte index value at rest
	int					index;	// 0-3 user set controller, 4 mouth
} mstudiobonecontroller_t;

// intersection boxes
typedef struct
{
	int					bone;
	int					group;			// intersection group
	Vector 				bbmin;		// bounding box
	Vector 				bbmax;		
} mstudiobbox_t;

#if !defined( CACHE_USER ) && !defined( QUAKEDEF_H )
#define CACHE_USER
typedef struct cache_user_s
{
	void *data;
} cache_user_t;
#endif

// demand loaded sequence groups
typedef struct
{
	char				label[32];	// textual name
	char				name[64];	// file name
	cache_user_t		cache;		// cache index pointer
	int					data;		// hack for group 0
} mstudioseqgroup_t;

// sequence descriptions
typedef struct
{
	char				label[32];	// sequence label

	float				fps;		// frames per second	
	int					flags;		// looping/non-looping flags

	int					activity;
	int					actweight;

	unsigned int		numevents;
	int					eventindex;

	int					numframes;	// number of frames per sequence

	int					numpivots;	// number of foot pivots
	int					pivotindex;

	int					motiontype;	
	int					motionbone;
	Vector 				linearmovement;
	int					automoveposindex;
	int					automoveangleindex;

	Vector 				bbmin;		// per sequence bounding box
	Vector 				bbmax;		

	int					numblends;
	int					animindex;		// mstudioanim_t pointer relative to start of sequence group data
										// [blend][bone][X, Y, Z, XR, YR, ZR]

	int					blendtype[2];	// X, Y, Z, XR, YR, ZR
	float				blendstart[2];	// starting value
	float				blendend[2];	// ending value
	int					blendparent;

	int					seqgroup;		// sequence group for demand loading

	int					entrynode;		// transition node at entry
	int					exitnode;		// transition node at exit
	int					nodeflags;		// transition rules
	
	int					nextseq;		// auto advancing sequences
} mstudioseqdesc_t;

// events
#include "studio_event.h"
/*
typedef struct 
{
	int 				frame;
	int					event;
	int					type;
	char				options[64];
} mstudioevent_t;
*/

// pivots
typedef struct 
{
	Vector 				org;	// pivot point
	int					start;
	int					end;
} mstudiopivot_t;

// attachment
typedef struct 
{
	char				name[32];
	int					type;
	int					bone;
	Vector 				org;	// attachment point
	Vector 				vectors[3];
} mstudioattachment_t;

typedef struct
{
	unsigned short	offset[6];
} mstudioanim_t;

// animation frames
typedef union 
{
	struct {
		byte	valid;
		byte	total;
	} num;
	short		value;
} mstudioanimvalue_t;



// body part index
typedef struct
{
	char				name[64];
	int					nummodels;
	int					base;
	int					modelindex; // index into models array
} mstudiobodyparts_t;



// skin info
typedef struct
{
	char					name[64];
	int						flags;
	int						width;
	int						height;
	int						index;
} mstudiotexture_t;


// skin families
// short	index[skinfamilies][skinref]

// studio models
typedef struct
{
	char				name[64];

	int					type;

	float				boundingradius;

	int					nummesh;
	int					meshindex;

	int					numverts;		// number of unique vertices
	int					vertinfoindex;	// vertex bone info
	int					vertindex;		// vertex Vector 
	int					numnorms;		// number of unique surface normals
	int					norminfoindex;	// normal bone info
	int					normindex;		// normal Vector 

	int					numgroups;		// deformation groups
	int					groupindex;
} mstudiomodel_t;


// Vector 	boundingbox[model][bone][2];	// complex intersection info


// meshes
typedef struct 
{
	int					numtris;
	int					triindex;
	int					skinref;
	int					numnorms;		// per mesh normals
	int					normindex;		// normal Vector 
} mstudiomesh_t;

// triangles
#if 0
typedef struct 
{
	short				vertindex;		// index into vertex array
	short				normindex;		// index into normal array
	short				s,t;			// s,t position on skin
} mstudiotrivert_t;
#endif

// model flags
enum {
	EF_ROCKET	= 1,
	EF_GRENADE	= 2,
	EF_GIB		= 4,
	EF_ROTATE	= 8,
	EF_TRACER	= 16,
	EF_ZOMGIB	= 32,
	EF_TRACER2	= 64,
	EF_TRACER3	= 128
};
// lighting options

enum {
	STUDIO_NF_FLATSHADE		= 0x0001,
	STUDIO_NF_CHROME		= 0x0002,
	STUDIO_NF_FULLBRIGHT	= 0x0004,
	STUDIO_NF_NOMIPS        = 0x0008,
	STUDIO_NF_ALPHA         = 0x0010,
	STUDIO_NF_ADDITIVE      = 0x0020,
	STUDIO_NF_MASKED        = 0x0040
};
// motion flags
enum {
	STUDIO_X = 0x0001,
	STUDIO_Y = 0x0002,
	STUDIO_Z = 0x0004,
	STUDIO_XR = 0x0008,
	STUDIO_YR = 0x0010,
	STUDIO_ZR = 0x0020,
	STUDIO_LX = 0x0040,
	STUDIO_LY = 0x0080,
	STUDIO_LZ = 0x0100,
	STUDIO_AX = 0x0200,
	STUDIO_AY = 0x0400,
	STUDIO_AZ = 0x0800,
	STUDIO_AXR = 0x1000,
	STUDIO_AYR = 0x2000,
	STUDIO_AZR = 0x4000,
	STUDIO_TYPES = 0x7FFF,
	STUDIO_RLOOP = 0x8000,		// controller that wraps shortest distance
	// sequence flags
	STUDIO_LOOPING = 0x0001,
};

enum {
// bone flags
	STUDIO_HAS_NORMALS	= 0x0001,
	STUDIO_HAS_VERTICES = 0x0002,
	STUDIO_HAS_BBOX		= 0x0004,
	STUDIO_HAS_CHROME	= 0x0008		// if any of the textures have chrome on them
};

constexpr float RAD_TO_STUDIO		= (32768.0/PI);
constexpr float STUDIO_TO_RAD		= (PI / 32768.0);
#endif
