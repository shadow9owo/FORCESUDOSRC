#include "Browser.h"
#include <raylib.h>
#include "StaticVars.h"
#include "Consts.h"
#include "FSudo.h"

namespace browser
{
    std::vector<std::string> browserbuff = {};
    std::string filename = {};
    void handleinput()
    {
        int charKey = GetCharPressed();
        while (charKey > 0)
        {
            if ((charKey >= 32) && (charKey <= 125))
            {

            }

            charKey = GetCharPressed();
        }

        int key = GetKeyPressed();
        while (key > 0)
        {
            if (key == KEY_ESCAPE)
            {
                apprunning = None;
                reset();
                std::vector<std::string> a;
                Utils::PrintScreen(a);
                currentline++;
                return;
            }

            key = GetKeyPressed();
        }

        if (IsKeyDown(KEY_UP))
        {
            if (currentline > 0)
                currentline--;
        }
        if (IsKeyDown(KEY_DOWN))
        {
            if (currentline < browserbuff.size())
                currentline++;
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
        for (std::string a : browserbuff)
        {
            DrawTextPro(font, a.c_str(), { 0, (float)24 * i }, { 0,0 }, 0, 24, 1, textcolor);
            i++;
        }

        for (PixelRect ab : framebuffer)
        {
            DrawRectangle(ab.x, ab.y, ab.w, ab.h, ab.color);
        }
    }
}