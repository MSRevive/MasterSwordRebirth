#ifndef MUSIC_H
#define MUSIC_H

#define MUSIC_PATH "music/"
#define MAX_SONGS 256

struct song_t
{
	msstring Name;
};

typedef mslist<song_t> songplaylist;

#endif