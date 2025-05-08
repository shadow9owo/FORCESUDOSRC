#pragma once
#include <string>
#include <raylib.h>

namespace CutscenePlayer
{
	extern Texture2D currentvar;
	extern bool ended;
	extern Sound gunshot;
	
	extern void init();
	extern void playcutscene();
}