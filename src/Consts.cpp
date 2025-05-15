#include "Consts.h"
#include "Vector2I.h"
#include <vector>
#include <string>
#include <filesystem>
#include "FSudo.h"

namespace fs = std::filesystem;

Vector2I WindowSize = { 800,600 };
bool fullscreen = false;

std::string savefilename = "save.xml";

std::vector<std::string> manual = {};
std::vector<std::string> apppaths = {};
void loadapppaths()
{
	if (!apppaths.empty()) { apppaths.clear(); }

	try
	{
		std::string path = "./apps/";
		for (const auto& entry : fs::directory_iterator(path)) {
			if (entry.is_regular_file() && entry.path().extension() == ".lua") {
				apppaths.push_back(entry.path().filename().string());
			}
		}
	}
	catch (const std::exception&)
	{

	}

	try
	{
		std::string path = "./apps/";
		for (const auto& entry : fs::directory_iterator(path)) {
			if (entry.is_regular_file() && Utils::tolowercasestring(entry.path().extension().string()) == ".shell") {
				apppaths.push_back(entry.path().filename().string());
			}
		}
	}
	catch (const std::exception&)
	{

	}

	return;
}

void loadmanual()
{
	manual.push_back("contents :\n 1 - 2 navigation\n 3 - 11 basic utility usage\n 12 - notes\n");
	manual.push_back("1 - naviagation\n\nnavigation in the freecze shell is quite simple.\nas the freecze shell is quite similar to unix\ndue to the fact that its just a really modified fork of it\nso if you have basic knowledge of bash then just use it\n");
	manual.push_back("2 - ls\n\nlists all the files in your home directory\n");
	manual.push_back("3 - del\n\ndeletes a choosen file in your home directory\n");
	manual.push_back("4 - ee\n\nbasic unix inspired file editor\n");
	manual.push_back("5 - mediaplayer\n\nmusic playing utility\n");
	manual.push_back("6 - browser\n\nweb viewing utility\n");
	manual.push_back("7 - cat\n\nread file command\n");
	manual.push_back("8 - color\n\nfont color utility\n");
	manual.push_back("9 - bcolor\n\nbackground color utility\n");
	manual.push_back("10 - clear\n\nscreen clean utility\n");
	manual.push_back("11 - debug\n\ndebug utility\n");
	manual.push_back("12 use lua scripts if you want async execution\nuse shell scripts if you dont care");
	return;
}