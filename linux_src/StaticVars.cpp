#include "StaticVars.h"
#include <filesystem>

namespace fs = std::filesystem;

Camera2D cam = { 0 };
std::vector<std::string> stringbuffer = { };
int fontsize = 24;
int currentline = 0;
std::vector<char> linebuffer = { };
Font font = {};
bool listen4input = true;
bool isanapprunning = false;

bool quitting = false;

bool debug = false;

std::vector<PixelRect> overlayframebuffer = { };
std::vector<PixelRect> framebuffer = { };

std::vector<std::string> ls = { };

Color textcolor = WHITE;
Color bgcolor = BLACK;

apps apprunning = None;

bool infected = false;

int webprogress  = -1;

bool ismusicplaying = false;

std::vector<Music> musiclist = {};

Scenes currentscene = MainMenu;

std::vector<std::string> whitelistedmods{};

void getls() {
	if (!ls.empty()) {
		ls.clear();
	}

	std::string path = (fs::path(GetWorkingDirectory()) / "apps" / "homedir").string();

	if (fs::exists(path) && fs::is_directory(path)) {
		for (const auto& entry : fs::directory_iterator(path)) {
			if (entry.is_regular_file()) {
				ls.push_back(entry.path().filename().string() + "\n");
			}
		}
	}
	else {
		
	}
}


void reset()
{
	currentline = 0;
	if (!linebuffer.empty())
	{
		linebuffer.clear();
	}
	if (!stringbuffer.empty())
	{
		stringbuffer.clear();
	}
	if (!framebuffer.empty())
	{
		framebuffer.clear();
	}
	if (!overlayframebuffer.empty())
	{
		overlayframebuffer.clear();
	}
	if (!ls.empty())
	{
		ls.clear();
	}
	return;
}