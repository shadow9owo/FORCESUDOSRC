#pragma once
#include "Vector2I.h"
#include <vector>
#include <string>
extern Vector2I WindowSize;
extern bool fullscreen;

extern std::vector<std::string> manual;
extern std::vector<std::string> apppaths;

extern void loadmanual();
extern void loadapppaths();

extern std::string savefilename;