#pragma once

//msprite_t::type
#define SPR_VP_PARALLEL_UPRIGHT 0
#define SPR_FACING_UPRIGHT 1
#define SPR_VP_PARALLEL 2
#define SPR_ORIENTED 3
#define SPR_VP_PARALLEL_ORIENTED 4

//mspriteframedesc_t::type
#define SPR_SINGLE 0
#define SPR_GROUP 1

struct mspriteframe_t
{
	int width;
	int height;
	float up, down, left, right;
	int gl_texturenum;
};

struct mspritegroup_t
{
	int numframes;
	float *intervals;
	mspriteframe_t *frames[1];
};

struct mspriteframedesc_t
{
	int type;
	mspriteframe_t *frameptr;
};

struct msprite_t
{
	short type;
	short texFormat;
	int maxwidth;
	int maxheight;
	int numframes;
	int paloffset;
	float beamlength;
	void *cachespot;
	mspriteframedesc_t frames[1];
};
