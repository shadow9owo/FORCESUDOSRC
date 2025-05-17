#include "SimpleEdit.h"
#include "StaticVars.h"
#include "Consts.h"
#include <raylib.h>
#include <fstream>
#include "FSudo.h"
namespace simpleedit
{
	std::vector<std::string> editorbuff;
	std::string filename;

	void stf(const std::string& filename)
	{
		std::ofstream file(filename);
		for (const auto& line : editorbuff)
			file << line << "\n";
		file << std::string(linebuffer.begin(), linebuffer.end());
		file.close();
	}

    void handleinput() //fucking arrow keys can go suck my dick
    {
        int charKey = GetCharPressed();
        while (charKey > 0)
        {
            if ((charKey >= 32) && (charKey <= 125))
            {
                linebuffer.push_back((char)charKey);
            }

            charKey = GetCharPressed();
        }

        int key = GetKeyPressed();
        while (key > 0)
        {
            if (key == KEY_BACKSPACE)
            {
                if (!linebuffer.empty())
                {
                    linebuffer.pop_back();
                }
                else if (!editorbuff.empty())
                {
                    std::string prev = editorbuff.back();
                    editorbuff.pop_back();
                    linebuffer = std::vector<char>(prev.begin(), prev.end());
                    currentline = std::max(0, currentline - 1);
                }
            }
            else if (key == KEY_ENTER)
            {
                std::string currentLine(linebuffer.begin(), linebuffer.end());
                editorbuff.push_back(currentLine);
                linebuffer.clear();
                currentline++;
            }
            else if (key == KEY_ESCAPE)
            {
                stf(filename);
                apprunning = None;
                reset();
                std::vector<std::string> a;
                a.push_back("saved " + filename + "\n");
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
            if (currentline < editorbuff.size())
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
        for (std::string a : editorbuff)
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
