#ifndef MUSIC_H
#define MUSIC_H

constexpr const char* MUSIC_PATH = "music/";
constexpr unsigned int MAX_SONGS = 256;

struct song_t
{
	msstring Name;
};

typedef mslist<song_t> songplaylist;

#endif