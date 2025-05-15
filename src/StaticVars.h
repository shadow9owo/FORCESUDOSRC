#pragma once
#include <raylib.h>
#include <vector>
#include <string>
#include "PixelRect.h"

extern Camera2D cam;
extern std::vector<std::string> stringbuffer;
extern int fontsize;
extern int currentline;
extern std::vector<char> linebuffer;
extern Font font;
extern bool listen4input;
extern bool isanapprunning;

extern void reset();

extern bool quitting;

extern bool debug;

extern std::vector<PixelRect> framebuffer;
extern std::vector<PixelRect> overlayframebuffer;

extern Color textcolor;
extern Color bgcolor;

extern std::vector<std::string> ls;

extern bool infected;

extern void getls();

extern int webprogress;

extern bool ismusicplaying;

extern std::vector<Music> musiclist;

enum apps
{
	None,
	SimpleEditor,
	Browser
};

enum Scenes
{
	MainMenu,
	Game,
	Outro
};

extern Scenes currentscene;

extern std::vector<std::string> whitelistedmods;

extern apps apprunning;