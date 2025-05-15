#include <raylib.h>
#include "Consts.h"
#include "StaticVars.h"
#include "PixelRect.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "FSudo.h"
#include <cctype>
#include "Lua542/include/lua.hpp"
#include "SimpleEdit.h"
#include "SaveManager.h"
#include "Security.h"
#include "Browser.h"
#include "MediaPlayer.h"
#include "CutscenePlayer.h"
#ifdef _WIN32
#pragma comment(lib, "lib/lua54.lib")
#endif // _WIN32
#include <thread>

namespace fs = std::filesystem;

bool cutscenefinished = false;

lua_State* luahandle;

namespace Utils
{
    void system(std::string a)
    {
        Input::execcmd(a);
    }

    bool iskeypressed(int keycode)
    {
        return IsKeyPressed(keycode);
    }

    int getkeypressed()
    {
        return GetCharPressed();
    }

    //stolen from stack overflow xddd
    std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
        std::vector<std::string> tokens;
        size_t pos = 0;
        size_t delimiter_length = delimiter.length();
        size_t start = 0;

        while ((pos = s.find(delimiter, start)) != std::string::npos) {
            tokens.push_back(s.substr(start, pos - start));
            start = pos + delimiter_length;
        }

        tokens.push_back(s.substr(start));

        return tokens;
    }

    void InjectaRectToConsoleFrameBuffer(int x,int y,int w, int h,Color color)
    {
        framebuffer.push_back({x,y,w,h,color});
        return;
    }

    void InjectaRectToOverlayFrameBuffer(int x, int y, int w, int h, Color color)
    {
        overlayframebuffer.push_back({ x,y,w,h,color });
        return;
    }

    void PrintScreen(std::vector<std::string> input)
    {
        if (input.empty()) { return; }

        for (const std::string& str : input)
        {
            for (char var : str)
            {
                if (var == '\n')
                {
                    if (!linebuffer.empty()) {
                        std::string tmpstring(linebuffer.begin(), linebuffer.end());
                        stringbuffer.push_back(tmpstring);
                        linebuffer.clear();
                        currentline = currentline + 1;
                    }
                }
                else
                {
                    linebuffer.push_back(var);
                }
            }
        }

        input.clear();
        return;
    }

    std::string tolowercasestring(std::string input)
    {
        for (char& ch : input)
        {
            ch = std::tolower(static_cast<unsigned char>(ch));
        }
        return input;
    }
    bool containsproxy(const std::string& a)
    {
        std::ifstream file(fs::path(GetWorkingDirectory()) / "apps" / "homedir" / "proxies.host");

        if (!file.is_open()) {
            printf("Failed to open proxies.host\n");
            return false;
        }

        std::string str;
        while (std::getline(file, str))
        {
                if (Utils::tolowercasestring(str) == Utils::tolowercasestring(a))
                {
                    return true;
                }
        }
        return false;
    }
}

namespace Renderer
{
    void renderer()
    {
        if (apprunning == SimpleEditor)
        {
            stringbuffer = simpleedit::editorbuff;
        }else if (apprunning == Browser)
        {
            stringbuffer = browser::browserbuff;
        }
        int totalLines = currentline; // or however many lines are in your buffer
        int visibleLines = (WindowSize.y / 1.5f) / fontsize;

        if (totalLines > visibleLines) {
            cam.offset.y = -(totalLines - visibleLines) * fontsize;
        }
        else {
            cam.offset.y = 0;
        }

        int i = 0;
        for (std::string a : stringbuffer)
        {
            DrawTextPro(font, a.c_str(), { 0, (float)24 * i }, { 0,0 }, 0, 24, 1, textcolor);
            i = i + 1;
        }
        DrawTextPro(font, std::string(linebuffer.begin(), linebuffer.end()).c_str(), { 0, (float)24 * i }, { 0,0 }, 0, 24, 1, textcolor);
        if (((int)(GetTime() * 2)) % 2 == 0) {
            float x = MeasureTextEx(font, std::string(linebuffer.begin(), linebuffer.end()).c_str(), 24, 1).x;
            DrawLine(x, 24 * i + 22, x + 10, 24 * i + 22, textcolor);
        }
        for (PixelRect a : framebuffer)
        {
            DrawRectangle(a.x, a.y, a.w, a.h, a.color);
        }
        return;
    }
    void overlay()
    {
        if (debug)
        {
            DrawText(std::to_string(GetFPS()).c_str(), 0, 0, fontsize, GREEN);
            DrawText(std::to_string(currentline).c_str(), 0, 24, fontsize, GREEN);
            DrawText(std::to_string(linebuffer.size()).c_str(), 0, 48, fontsize, GREEN);
            DrawText(std::to_string(cam.offset.y).c_str(), 0, 72, fontsize, GREEN);
        }
        for (PixelRect a : overlayframebuffer)
        {
            DrawRectangle(a.x, a.y, a.w, a.h, a.color);
        }
        return;
    }
}

namespace Input
{
    bool execcmd(std::string& b)
    {
        if (b._Starts_with("--")) { 
            isanapprunning = false;
            return true;
        }
        else if (Utils::tolowercasestring(b) == "ls")
        {
            try
            {
                getls();
                Utils::PrintScreen(ls);
                isanapprunning = false;
                return true;
            }
            catch (const std::exception& e)
            {
                printf("%s", e.what());
            }
        }
        else if (Utils::tolowercasestring(b) == "clear")
        {
            isanapprunning = false;
            reset();
            return true;
        }
        else if (Utils::tolowercasestring(b) == "exit")
        {
            SaveSystem::Save();
            quitting = true;
            return true;
        }
        else if (Utils::tolowercasestring(b) == "clist")
        {
            std::vector<std::string> a;
            a.push_back("exit -- exits\n");
            a.push_back("clear -- clears screen\n");
            a.push_back("clist -- lists commands\n");
            a.push_back("alist -- lists applications [mods]\n");
            a.push_back(" arguments: refresh -- refreshes the installed app list\n");
            a.push_back("man -- shows the manual%\n");
            a.push_back(" arguments: %page% -- can be used to view a specific page\n");
            a.push_back("debug -- toggles the debug switch\n");
            a.push_back("bcolor -- lets you change background color 1-15\n");
            a.push_back("color -- lets you change text color 1-15\n");
            a.push_back("ls -- lists file in home directory\n");
            a.push_back("cat -- reads file\n");
            a.push_back("del -- deletes file\n");
            a.push_back("ee -- file editor\n");
            a.push_back("browser -- view an url\n");
            a.push_back("mediaplayer -- mediaplayer\n");
            a.push_back(" arguments: play -- plays song\n");
            a.push_back(" arguments: stop -- stops playing a song\n");
            a.push_back(" arguments: refresh -- refreshes music db\n");
            Utils::PrintScreen(a);
            isanapprunning = false;
            return true;
        }
        else if (Utils::tolowercasestring(b) == "debug")
        {
            debug = !debug;
            SaveSystem::Save();
            isanapprunning = false;
            return true;
        }
        else
        {
            if (Utils::tolowercasestring(Utils::split(b, " ").at(0)) == "man")
            {
                try
                {
                    int index = 0;
                    auto parts = Utils::split(b, " ");
                    if (parts.size() > 1) {
                        try {
                            index = std::stoi(parts.at(1));
                        }
                        catch (const std::exception&) {
                            index = 0;
                        }
                    }
                    Utils::PrintScreen({ manual.at(index) });
                    isanapprunning = false;
                    return true;
                }
                catch (const std::exception&)
                {
                    isanapprunning = false;
                    return false;
                }
            }
            else if (Utils::tolowercasestring(Utils::split(b, " ").at(0)) == "alist")
            {
                if (Utils::split(b, " ").size() < 2)
                {
                    for(std::string var : apppaths)
                    {
                        std::vector<std::string> a = {};
                        a.push_back(var + "\n");
                        Utils::PrintScreen(a);
                    }
                    isanapprunning = false;
                    return true;
                }
                else {
                    if (Utils::tolowercasestring(Utils::split(b, " ").at(1)) == "refresh")
                    {
                        loadapppaths();
                        std::vector<std::string> a = {};
                        a.push_back("reloaded..\n");
                        Utils::PrintScreen(a);
                        isanapprunning = false;
                        return true;
                    }
                }
            }
            else if (Utils::tolowercasestring(Utils::split(b, " ").at(0)) == "ee")
            {
                if (Utils::split(b, " ").size() < 2)
                {
                    std::vector<std::string> a = {};
                    a.push_back("specify file to edit / create\n");
                    Utils::PrintScreen(a);
                    isanapprunning = false;
                    return true;
                }
                else {
                    if (Utils::split(b, " ").at(1) == "boot.ini") {
                        std::vector<std::string> a = {};
                        a.push_back("access denied.\n");
                        Utils::PrintScreen(a);
                        isanapprunning = false;
                        return true;
                    }
                    reset();
                    apprunning = SimpleEditor;
                    simpleedit::editorbuff = {};
                    currentline = 0;
                    if (std::filesystem::exists(fs::path(GetWorkingDirectory()) / "apps" / "homedir" / Utils::split(b, " ").at(1)))
                    {
                        std::ifstream file(fs::path(GetWorkingDirectory()) / "apps" / "homedir" / Utils::split(b, " ").at(1));
                        std::string str;
                        while (std::getline(file, str))
                        {
                            simpleedit::editorbuff.push_back(str);
                        }
                        stringbuffer = simpleedit::editorbuff;
                    }
                    simpleedit::filename = (fs::path(GetWorkingDirectory()) / "apps" / "homedir" / Utils::split(b, " ").at(1)).string();
                    isanapprunning = false;
                    return true;
                }
            }
            else if (Utils::tolowercasestring(Utils::split(b, " ").at(0)) == "browser")
            {
                if (Utils::split(b, " ").size() < 2)
                {
                    std::vector<std::string> a = {};
                    a.push_back("specify url to view\n");
                    Utils::PrintScreen(a);
                    isanapprunning = false;
                    return true;
                }
                else {
                    if (Utils::tolowercasestring(Utils::split(b, " ").at(1)) == "biometric-vault.tech")
                    {
                        printf("%s", "1");
                        if (Utils::containsproxy("wowb"))
                        {
                            if (webprogress < 2)
                            {
                                printf("%s", "1");
                                if (std::filesystem::exists(fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1)))
                                {
                                    webprogress =2;
                                    apprunning = Browser;
                                    browser::browserbuff = {};
                                    currentline = 0;
                                    std::ifstream file(fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1));
                                    std::string str;
                                    while (std::getline(file, str))
                                    {
                                        browser::browserbuff.push_back(str + "\n");
                                    }
                                }
                                printf("%s", "1");
                                browser::filename = (fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1)).string();
                                isanapprunning = false;
                                return true;
                            }
                        }
                        else {
                            apprunning = None;
                            std::vector<std::string> a;
                            a.push_back("404\n");
                            Utils::PrintScreen(a);
                            isanapprunning = false;
                            return true;
                        }
                    }
                    else if (Utils::tolowercasestring(Utils::split(b, " ").at(1)) == "fleshcache.dev")
                    {
                        if (Utils::containsproxy("proxy27398shdwdwa"))
                        {
                            if (webprogress < 3)
                            {
                                if (std::filesystem::exists(fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1)))
                                {
                                    webprogress = 3;
                                    apprunning = Browser;
                                    browser::browserbuff = {};
                                    currentline = 0;
                                    std::ifstream file(fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1));
                                    std::string str;
                                    while (std::getline(file, str))
                                    {
                                        browser::browserbuff.push_back(str + "\n");
                                    }
                                }
                                browser::filename = (fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1)).string();
                                isanapprunning = false;
                                return true;
                            }
                        }
                        else {
                            apprunning = None;
                            std::vector<std::string> a;
                            a.push_back("404\n");
                            Utils::PrintScreen(a);
                            isanapprunning = false;
                            return true;
                        }
                    }
                    else if (Utils::tolowercasestring(Utils::split(b, " ").at(1)) == "backdoorplay.xxx")
                    {
                        if (Utils::containsproxy("iwhjaidj"))
                        {
                            if (webprogress < 4)
                            {
                                if (std::filesystem::exists(fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1)))
                                {
                                    webprogress = 4;
                                    apprunning = Browser;
                                    browser::browserbuff = {};
                                    currentline = 0;
                                    std::ifstream file(fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1));
                                    std::string str;
                                    while (std::getline(file, str))
                                    {
                                        browser::browserbuff.push_back(str + "\n");
                                    }
                                }
                                browser::filename = (fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1)).string();
                                isanapprunning = false;
                                return true;
                            }
                        }
                        else {
                            apprunning = None;
                            std::vector<std::string> a;
                            a.push_back("404\n");
                            Utils::PrintScreen(a);
                            isanapprunning = false;
                            return true;
                        }
                    }
                    else if (Utils::tolowercasestring(Utils::split(b, " ").at(1)) == "brainlease.net")
                    {

                        if (std::filesystem::exists(fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1)))
                        {
                            if (webprogress < 0)
                            {
                                webprogress = 0;
                            }
                            apprunning = Browser;
                            browser::browserbuff = {};
                            currentline = 0;
                            std::ifstream file(fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1));
                            std::string str;
                            while (std::getline(file, str))
                            {
                                browser::browserbuff.push_back(str + "\n");
                            }
                        }
                        else {
                            apprunning = None;
                        }
                        browser::filename = (fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1)).string();
                        isanapprunning = false;
                        return true;
                    }
                    else if (Utils::tolowercasestring(Utils::split(b, " ").at(1)) == "fleshforhire.store")
                    {
                        if (webprogress < 1)
                        {
                            if (std::filesystem::exists(fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1)))
                            {
                                webprogress = 1;
                                apprunning = Browser;
                                browser::browserbuff = {};
                                currentline = 0;
                                std::ifstream file(fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1));
                                std::string str;
                                while (std::getline(file, str))
                                {
                                    browser::browserbuff.push_back(str + "\n");
                                }
                            }
                            browser::filename = (fs::path(GetWorkingDirectory()) / "apps" / "websites" / Utils::split(b, " ").at(1)).string();
                            isanapprunning = false;
                            return true;
                        }
                        else {
                            apprunning = None;
                            std::vector<std::string> a;
                            a.push_back("404\n");
                            Utils::PrintScreen(a);
                            isanapprunning = false;
                            return true;
                        }
                    }
                    else if (Utils::tolowercasestring(Utils::split(b, " ").at(1)) == "redroom.xxx")
                    {
                        if (Utils::containsproxy("uiaehwsdiujawhuio"))
                        {
                            if (webprogress < 4)
                            {
                                apprunning = None;
                                std::vector<std::string> a;
                                a.push_back("kill yourself cheating bitch\n");
                                Utils::PrintScreen(a);
                                isanapprunning = false;
                                return true;
                            }
                            else {
                                currentscene = Outro;
                                isanapprunning = false;
                                return true;
                            }
                        }
                        else {
                            apprunning = None;
                            std::vector<std::string> a;
                            a.push_back("404\n");
                            Utils::PrintScreen(a);
                            isanapprunning = false;
                            return true;
                        }
                    }
                    else {
                        apprunning = None;
                        std::vector<std::string> a;
                        a.push_back("404\n");
                        Utils::PrintScreen(a);
                        isanapprunning = false;
                        return true;
                    }
                    reset();
                    apprunning = None;
                    isanapprunning = false;
                    return true;
                }
            }
            else if (Utils::tolowercasestring(Utils::split(b, " ").at(0)) == "color")
            {
                try
                {
                    switch (std::stoi(Utils::split(b, " ").at(1)))
                    {
                        case 0:
                            textcolor = BLACK;
                            break;
                        case 1:
                            textcolor = RED;
                            break;
                        case 2:
                            textcolor = GREEN;
                            break;
                        case 3:
                            textcolor = BLUE;
                            break;
                        case 4:
                            textcolor = YELLOW;
                            break;
                        case 5:
                            textcolor = MAGENTA;
                            break;
                        case 6:
                            textcolor = { 0, 255, 255 ,255 };
                            break;
                        case 7:
                            textcolor = WHITE;
                            break;
                        case 8:
                            textcolor = DARKGRAY;
                            break;
                        case 9:
                            textcolor = { 255, 127, 127,255 };
                            break;
                        case 10:
                            textcolor = {144,238, 144,255 };
                            break;
                        case 11:
                            textcolor = { 173, 216, 230,255 };
                            break;
                        case 12:
                            textcolor = { 255, 255, 237,255 };
                            break;
                        case 13:
                            textcolor = {255,128,255,255 };
                            break;
                        case 14:
                            textcolor = { 224, 255, 255,255 };
                            break;
                        case 15:
                            textcolor = { 255, 255, 247,255 };
                            break;
                    default:
                        throw std::runtime_error("invalid color code bruh");
                        break;
                    }
                    SaveSystem::Save();
                    isanapprunning = false;
                    return true;
                }
                catch (const std::exception&)
                {
                    isanapprunning = false;
                    return false;
                }
            }
            else if (Utils::tolowercasestring(Utils::split(b, " ").at(0)) == "bcolor")
            {
                try
                {
                    switch (std::stoi(Utils::split(b, " ").at(1)))
                    {
                        case 0:
                            bgcolor = BLACK;
                            break;
                        case 1:
                            bgcolor = RED;
                            break;
                        case 2:
                            bgcolor = GREEN;
                            break;
                        case 3:
                            bgcolor = BLUE;
                            break;
                        case 4:
                            bgcolor = YELLOW;
                            break;
                        case 5:
                            bgcolor = MAGENTA;
                            break;
                        case 6:
                            bgcolor = { 0, 255, 255,255 };
                            break;
                        case 7:
                            bgcolor = WHITE;
                            break;
                        case 8:
                            bgcolor = DARKGRAY;
                            break;
                        case 9:
                            bgcolor = { 255, 127, 127,255 };
                            break;
                        case 10:
                            bgcolor = { 144,238, 144 ,255 };
                            break;
                        case 11:
                            bgcolor = { 173, 216, 230,255 };
                            break;
                        case 12:
                            bgcolor = { 255, 255, 237,255 };
                            break;
                        case 13:
                            bgcolor = { 255,128,255,255 };
                            break;
                        case 14:
                            bgcolor = { 224, 255, 255,255 };
                            break;
                        case 15:
                            bgcolor = { 255, 255, 247,255 };
                            break;
                    default:
                        throw std::runtime_error("invalid color code bruh");
                        break;
                    }
                    SaveSystem::Save();
                    isanapprunning = false;
                    return true;
                }
                catch (const std::exception&)
                {
                    isanapprunning = false;
                    return false;
                }
            }
            else if (Utils::tolowercasestring(Utils::split(b, " ").at(0)) == "del")
            {
                try
                {
                    getls();
                    for(std::string var : ls)
                    {
                        std::cout << (fs::path(GetWorkingDirectory()) / "apps" / "homedir" / Utils::split(b, " ").at(1)).string().c_str() << std::endl;
                        if (std::filesystem::exists(fs::path(GetWorkingDirectory()) / "apps" / "homedir" / Utils::split(b, " ").at(1)))
                        {
                            try
                            {
                                std::filesystem::remove(fs::path(GetWorkingDirectory()) / "apps" / "homedir" / Utils::split(b, " ").at(1));
                                isanapprunning = false;
                                return true;
                            }
                            catch (const std::exception&)
                            {

                            }
                        }
                    }
                    isanapprunning = false;
                    return false;
                }
                catch (const std::exception& e)
                {
                    printf("%s", e.what());
                }
                isanapprunning = false;
                return false;
            }
            else if (Utils::tolowercasestring(Utils::split(b, " ").at(0)) == "cat")
            {
                try
                {
                    auto parts = Utils::split(b, " ");
                    if (parts.size() <= 1)
                    {
                        Utils::PrintScreen({ "nulinput\n" });
                        return false;
                    }
                    if (!(std::filesystem::exists(fs::path(GetWorkingDirectory()) / "apps" / "homedir" / Utils::split(b, " ").at(1)))) { return false; isanapprunning = false; }

                    std::ifstream file(fs::path(GetWorkingDirectory()) / "apps" / "homedir" / parts.at(1));
                    if (!file)
                    {
                        Utils::PrintScreen({ "couldntopenfile\n" });
                        return false;
                    }

                    std::string line;
                    std::vector<std::string> lines;
                    while (std::getline(file, line))
                    {
                        lines.push_back(line + "\n");
                    }

                    Utils::PrintScreen(lines);
                    isanapprunning = false;
                    return true;
                }
                catch (const std::exception& e)
                {
                    Utils::PrintScreen({ std::string("exception ") + e.what() + "\n" });
                    isanapprunning = false;
                    return false;
                }
            }
            else if (Utils::tolowercasestring(Utils::split(b, " ").at(0)) == "mediaplayer")
            {
                try
                {
                    if (Utils::tolowercasestring(Utils::split(b, " ").at(1)) == "play")
                    {
                        int newSong = GetRandomValue(0, musiclist.size() - 1);
                        while (newSong == MediaPlayer::currentsongplaying) {
                            newSong = GetRandomValue(0, musiclist.size() - 1);
                        }
                        MediaPlayer::currentsongplaying = newSong;
                        ismusicplaying = true;
                        MediaPlayer::UpdateMusic();
                        Utils::PrintScreen({ "playing\n" });
                        isanapprunning = false;
                        return true;
                    }
                    else if (Utils::tolowercasestring(Utils::split(b, " ").at(1)) == "stop")
                    {
                        MediaPlayer::StopMusic();
                        ismusicplaying = false;
                        Utils::PrintScreen({ "stopped successfully\n" });
                        isanapprunning = false;
                        return true;
                    }
                    else if (Utils::tolowercasestring(Utils::split(b, " ").at(1)) == "refresh")
                    {
                        ismusicplaying = false;
                        MediaPlayer::RefreshList();
                        Utils::PrintScreen({ "refreshed successfully " + std::to_string((musiclist.size() - 1)) + " files detected\n"});
                        isanapprunning = false;
                        return true;
                    }
                    isanapprunning = false;
                    return true;
                }
                catch (const std::exception& e)
                {
                    Utils::PrintScreen({ std::string("exception ") + e.what() + "\n" });
                    isanapprunning = false;
                    return false;
                }
            }
            for (const std::string& a : apppaths)
            {
                if (b == a)
                {
                    if (Utils::tolowercasestring(a).find("shell") != std::string::npos) {
                        std::ifstream ifs(a);

                        std::string line;

                        while (std::getline(ifs, line))
                        {
                            Input::execcmd(line);
                        }
                    }
                    else {
                        std::thread([=]() {
                            std::string scriptPath = "./apps/" + a;

                            lua_State* L = luahandle;
                            int result = luaL_dofile(L, scriptPath.c_str());
                            if (result != LUA_OK) {
                                const char* err = lua_tostring(L, -1);
                                lua_pop(L, 1);

                                std::vector<std::string> msg = { std::string("Lua error: ") + err + "\n" };
                                Utils::PrintScreen(msg);
                            }
                            }).detach();
                    }
                    return true;
                }
            

            return false;
        }
        isanapprunning = false;
        return false;
    }
    }
    void inputhandling()
    {
        if (infected) { return; }
        if (!listen4input) { return; }
        if (IsKeyPressed(KEY_ENTER))
        {
            if (!linebuffer.empty())
            {
                std::string tmpstring(linebuffer.begin(), linebuffer.end());
                linebuffer.clear();
                stringbuffer.push_back(tmpstring);
                if (!execcmd(tmpstring))
                {
                    std::vector<std::string> a;
                    a.push_back("Unknown command use clist to list commands\n");
                    a.push_back("or\n");
                    a.push_back("use alist to list installed apps\n");
                    a.push_back("if your app should be installed but it isnt then run \'alist refresh\'\n");
                    a.push_back("or an error accured\n");
                    Utils::PrintScreen(a);
                    isanapprunning = false;
                    currentline=currentline + 4; //dirty quick ugly hack
                    linebuffer.clear();
                }
                else {
                    linebuffer.clear();
                }
            }
            else {
                stringbuffer.push_back("");
                currentline++;
            }
        }
        else if (IsKeyPressed(KEY_BACKSPACE))
        {
            if (!linebuffer.empty())
            {
                linebuffer.pop_back();
            }
        }
        else if (IsKeyPressed(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C))
        {
            if (isanapprunning)
            {
                std::vector<std::string> a;
                a.push_back("App Closed..\n");
                Utils::PrintScreen(a);
                isanapprunning = false;
            }
        }
        else
        {
            int key = GetCharPressed();
            if (key > 0)
            {
                linebuffer.push_back((char)key);
            }
        }
    }
}

void defineluavariables(lua_State* L)
{
    lua_pushboolean(L, debug);
    lua_setglobal(L, "debug");

    lua_pushboolean(L, quitting);
    lua_setglobal(L, "quitting");

    lua_pushboolean(L, fullscreen);
    lua_setglobal(L, "fullscreen");

    lua_pushboolean(L, listen4input);
    lua_setglobal(L, "listen4input");

    lua_pushboolean(L, isanapprunning);
    lua_setglobal(L, "isanapprunning");
    return;
}

namespace lua
{
    int lua_utils_system(lua_State* L) {
        const char* cmd = luaL_checkstring(L, 1);
        Utils::system(cmd);
        return 0;
    }

    int lua_utils_iskeypressed(lua_State* L) {
        int key = luaL_checkinteger(L, 1);
        lua_pushboolean(L, Utils::iskeypressed(key));
        return 1;
    }

    int lua_utils_printscreen(lua_State* L) {
        if (!lua_istable(L, 1)) {
            return luaL_error(L, "expected table of strings");
        }

        std::vector<std::string> lines;

        lua_pushnil(L);
        while (lua_next(L, 1) != 0) {
            if (lua_isstring(L, -1)) {
                lines.push_back(lua_tostring(L, -1));
            }
            lua_pop(L, 1);
        }

        Utils::PrintScreen(lines);
        return 0;
    }

    int lua_utils_injectrectangle(lua_State* L) {
        try
        {
            float x = luaL_checknumber(L, 1);
            float y = luaL_checknumber(L, 2);
            float w = luaL_checknumber(L, 3);
            float h = luaL_checknumber(L, 4);
            int r = luaL_checkinteger(L, 5);
            int g = luaL_checkinteger(L, 6);
            int b = luaL_checkinteger(L, 7);
            int a = luaL_optinteger(L, 8, 255);

            Color color = { (unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a };

            Utils::InjectaRectToConsoleFrameBuffer(x, y, w, h, color);

            return 0;
        }
        catch (const std::exception&)
        {
            return 1;
        }

    }

    int lua_utils_injectrectangleoverlay(lua_State* L) {
        try
        {
            float x = luaL_checknumber(L, 1);
            float y = luaL_checknumber(L, 2);
            float w = luaL_checknumber(L, 3);
            float h = luaL_checknumber(L, 4);
            int r = luaL_checkinteger(L, 5);
            int g = luaL_checkinteger(L, 6);
            int b = luaL_checkinteger(L, 7);
            int a = luaL_optinteger(L, 8, 255);

            Color color = { (unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a };

            Utils::InjectaRectToOverlayFrameBuffer(x, y, w, h, color);

        }
        catch (const std::exception&)
        {
            return 1;
        }
        return 0;
    }

    int lua_utils_getkeypressed(lua_State* L)
    {
        int key = Utils::getkeypressed();
        lua_pushinteger(L, key);
        return 1;
    }

    int lua_utils_gethomedir(lua_State* L)
    {
        std::string path = std::string(GetWorkingDirectory()) + "/apps/homedir/";
        lua_pushstring(L, path.c_str());
        return 1;
    }

    int lua_utils_getappdir(lua_State * L)
    {
        std::string path = std::string(GetWorkingDirectory()) + "/apps/";
        lua_pushstring(L, path.c_str());
        return 1;
    }
}

void register_utils(lua_State* L) {
     lua_register(L, "system", lua::lua_utils_system);
     lua_register(L, "printscreen", lua::lua_utils_printscreen);
}

int main(void)
{
    SaveSystem::Load();

    InitWindow(WindowSize.x, WindowSize.y, "FSudo");

    InitAudioDevice();

    try
    {
        CutscenePlayer::init();
    }
    catch (const std::exception&)
    {

    }

    SetConfigFlags(FLAG_MSAA_4X_HINT);

    if (fullscreen && !IsWindowFullscreen())
    {
        ToggleFullscreen();
    }

    SetExitKey(0);

    SetTargetFPS(60);

    luahandle = luaL_newstate();
    luaL_openlibs(luahandle);
    register_utils(luahandle);
    defineluavariables(luahandle);

    loadmanual();
    loadapppaths();
    //CutscenePlayer::currentcutscenepath = "./assets/endingcutscene.mp4";
    
    font = LoadFont("./assets/Arial.ttf");

    cam.offset = { 0, 0 };
    cam.target = { 0, 0 };
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;

    bool booted = false;
    Rectangle mouse = { GetMouseX(),GetMouseY(),4,4 };
    Rectangle Story, Multiplayer, WipeSave;

    while (!WindowShouldClose() && !quitting)
    {
        switch (currentscene)
        {
        case MainMenu:
            BeginDrawing();
            mouse = { (float)GetMouseX(), (float)GetMouseY(), 4.0f, 4.0f };
            DrawText("FORCE SUDO\nDEMO",
                (WindowSize.x / 2) - (MeasureText("FORCE SUDO", 72) / 2),
                (WindowSize.y / 8),
                72, WHITE);

            // Story mode
            if (CheckCollisionRecs(mouse, Rectangle{
                (float)(WindowSize.x / 2.0f - MeasureText("STORY", 72) / 2.0f),
                (float)(WindowSize.y / 2.5f),
                (float)MeasureText("STORY", 72),
                72.0f }))
            {
                DrawText("STORY",
                    (WindowSize.x / 2) - (MeasureText("STORY", 72) / 2),
                    (WindowSize.y / 2.5f),
                    72, WHITE);

                if (IsMouseButtonPressed(0))
                {
                    currentscene = Game;
                }
            }
            else {
                DrawText("STORY",
                    (WindowSize.x / 2) - (MeasureText("STORY", 72) / 2),
                    (WindowSize.y / 2.5f),
                    72, GRAY);
            }

            // Multiplayer mode
            DrawText("MULTIPLAYER\n[UNAVAILIBLE IN THE DEMO]",
                (WindowSize.x / 2) - (MeasureText("MULTIPLAYER\n[UNAVAILIBLE IN THE DEMO]", 48) / 2),
                (WindowSize.y / 2),
                48, DARKGRAY);

            // Wipe save file
            if (CheckCollisionRecs(mouse, Rectangle{
                (float)(WindowSize.x / 2.0f - MeasureText("WIPE SAVE FILE", 72) / 2.0f),
                (float)(WindowSize.y / 1.5f),
                (float)MeasureText("WIPE SAVE FILE", 72),
                72.0f }))
            {
                DrawText("WIPE SAVE FILE",
                    (WindowSize.x / 2) - (MeasureText("WIPE SAVE FILE", 72) / 2),
                    (WindowSize.y / 1.5f),
                    72, WHITE);

                if (IsMouseButtonPressed(0))
                {
                    std::remove("save.xml");
                    std::remove("./apps/homedir/proxies.host");
                    
                    std::ofstream MyFile("./apps/homedir/proxies.host");

                    MyFile << "--this is an proxies file some websites require an proxy to access"<< std::endl;
                    MyFile << "put proxies here--" << std::endl;

                    MyFile.close();

                    exit(0);
                }
            }
            else {
                DrawText("WIPE SAVE FILE",
                    (WindowSize.x / 2) - (MeasureText("WIPE SAVE FILE", 72) / 2),
                    (WindowSize.y / 1.5f),
                    72, GRAY);
            }

            ClearBackground(BLACK);
            EndDrawing();
            break;
        case Game:
            if (!booted)
            {
                HideCursor();
                std::vector<std::string> a;
                try
                {
                    std::string path = std::string(GetWorkingDirectory()) + "/apps/homedir/";
                    std::cout << sha256_file(path + "boot.ini") << std::endl;
                    if (sha256_file(path + "boot.ini") != "5c7e41fbe268ae6d1865a60427ee73ca3ccdaca74370216cfe0507e04c439b90")
                    {
                        a.push_back("mbr not found.\n");
                        Utils::PrintScreen(a);
                        infected = true;
                    }
                    else
                    {
                        a.push_back("booting...\n");
                        a.push_back("Welcome to Freecze OS\n");
                        a.push_back("The first open source operating system of the 21st century\n");
                        a.push_back("#################################\n");
                        a.push_back("this OS comes with a manual\n");
                        a.push_back("to use the manual type man %page%\n");
                        a.push_back("use clist to list commands\n");
                        a.push_back("#################################\n");
                        Utils::PrintScreen(a);
                    }
                }
                catch (const std::exception&)
                {
                    a.push_back("mbr not found.\n");
                    Utils::PrintScreen(a);
                    infected = true;
                }

                if (!infected)
                {
                    MediaPlayer::RefreshList();

                    //autorun
                    std::ifstream ifs("./apps/homedir/AUTORUN.shell");

                    std::string line;

                    while (std::getline(ifs, line))
                    {
                        Input::execcmd(line);
                    }
                }
                booted = true;
            }
            if (ismusicplaying)
            {
                MediaPlayer::UpdateMusic();
            }
            if (IsKeyPressed(KEY_F11))
            {
                ToggleFullscreen();
            }
            BeginDrawing();
            ClearBackground(bgcolor);
            BeginMode2D(cam);
            if (apprunning == None)
            {
                Input::inputhandling();
            }
            else if (apprunning == SimpleEditor)
            {
                simpleedit::handleinput();
            }
            else if (apprunning == Browser)
            {
                browser::handleinput();
            }
            Renderer::renderer();
            EndMode2D();
            Renderer::overlay();
            EndDrawing();
            break;
        case Outro:
            BeginDrawing();
            if (!cutscenefinished)
            {
                if (!CutscenePlayer::ended)
                {
                    SetTargetFPS(12);
                    ClearBackground(BLACK);
                    CutscenePlayer::playcutscene();
                    DrawTexturePro(CutscenePlayer::currentvar, { 0,0,(float)CutscenePlayer::currentvar.width,(float)CutscenePlayer::currentvar.height }, { 0,0,(float)WindowSize.x,(float)WindowSize.y }, { 0,0 }, 0, WHITE);
                }
                else {
                    cutscenefinished = true;
                    PlaySound(CutscenePlayer::gunshot);
                    SetTargetFPS(60);
                }
            }
            else {
                ClearBackground(BLACK);
                SetExitKey((int)KEY_ESCAPE);
                DrawText("DEVELOPED By SHADOWDEV\nART By SHADOWDEV\nSTORY By SHADOWDEV\nDESIGN BY SHADOWDEV\n\nTHANKS FOR PLAYING\nTHE DEMO :D (press esc to exit)", 0, 0, 48, WHITE);
            }
            EndDrawing();
            break;
        default:
            break;
        }
    }

    CloseAudioDevice();

    CloseWindow();
    lua_close(luahandle);

	return 0;
}
