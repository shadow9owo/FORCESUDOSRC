#include "MediaPlayer.h"
#include "StaticVars.h"
#include <filesystem>
#include <raylib.h>
#include <iostream>

namespace fs = std::filesystem;

namespace MediaPlayer
{
	int currentsongplaying = 0;
	void UpdateMusic()
	{
		if (!IsMusicStreamPlaying(musiclist[currentsongplaying])) {
			int newSong = GetRandomValue(0, musiclist.size() - 1);
			while (newSong == MediaPlayer::currentsongplaying) {
				newSong = GetRandomValue(0, musiclist.size() - 1);
			}
			MediaPlayer::currentsongplaying = newSong;
			PlayMusicStream(musiclist[currentsongplaying]);
		}
		else {
			UpdateMusicStream(musiclist[currentsongplaying]);
		}
		return;
	}

	void StopMusic()
	{
		StopMusicStream(musiclist[currentsongplaying]);
		return;
	}

	void RefreshList()
	{
		if (!musiclist.empty()) { musiclist.clear(); }
		try
		{
			std::string path = "./assets/music/";
			for (const auto& entry : fs::directory_iterator(path)) {
				if (entry.is_regular_file() && entry.path().extension() == ".mp3") {
					musiclist.push_back(LoadMusicStream(entry.path().string().c_str()));
				}
			}
		}
		catch (const std::exception&)
		{
			std::cout << "no music files weird error idk" << std::endl;
			musiclist = {};
		}
		return;
	}
}