#include "CutscenePlayer.h"
#include <raylib.h>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace CutscenePlayer
{
    Texture2D currentvar;
    std::vector<Texture2D> frames;
    int i = 0;
    bool ended = false;
    Sound gunshot;

    void init()
    {
        const std::string folderPath = "./assets/endingcutscene/";
        std::vector<std::filesystem::directory_entry> files;

        gunshot = LoadSound("./assets/sfx/gunshot.mp3");

        for (const auto& entry : std::filesystem::directory_iterator(folderPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".png")
            {
                files.push_back(entry);
            }
        }

        std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
            return a.path().filename() < b.path().filename();
            });

        for (const auto& entry : files)
        {
            std::string path = entry.path().string();
            Texture2D tex = LoadTexture(path.c_str());
            frames.push_back(tex);
        }

        if (!frames.empty())
        {
            currentvar = frames[i];
        }
        else
        {
            ended = true;
        }
    }

    void playcutscene()
    {
        if (!ended && i + 1 < frames.size())
        {
            i++;
            currentvar = frames[i];
        }
        else
        {
            ended = true;
        }
    }
}
