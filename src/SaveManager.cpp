#include "SaveManager.h"
#include <tinyxml2.h>
#include <iostream>
#include "StaticVars.h"
#include "Consts.h"

namespace SaveSystem
{
    void Save()
    {
        tinyxml2::XMLDocument doc;

        tinyxml2::XMLElement* root = doc.NewElement("SaveData");
        doc.InsertFirstChild(root);

        tinyxml2::XMLElement* ce = doc.NewElement("bgcolor");
        ce->SetAttribute("r", bgcolor.r);
        ce->SetAttribute("g", bgcolor.g);
        ce->SetAttribute("b", bgcolor.b);
        ce->SetAttribute("a", bgcolor.a);
        root->InsertEndChild(ce);
        tinyxml2::XMLElement* c1e = doc.NewElement("color");
        c1e->SetAttribute("r", textcolor.r);
        c1e->SetAttribute("g", textcolor.g);
        c1e->SetAttribute("b", textcolor.b);
        c1e->SetAttribute("a", textcolor.a);
        root->InsertEndChild(c1e);


        tinyxml2::XMLElement* c2e = doc.NewElement("debug");
        c2e->SetAttribute("value", debug);
        root->InsertEndChild(c2e);

        tinyxml2::XMLElement* c3e = doc.NewElement("fullscreen");
        c3e->SetAttribute("value", fullscreen);
        root->InsertEndChild(c3e);

        tinyxml2::XMLElement* c4e = doc.NewElement("windowsize");
        c4e->SetAttribute("X", WindowSize.x);
        c4e->SetAttribute("Y", WindowSize.y);
        root->InsertEndChild(c4e);

        tinyxml2::XMLElement* c5e = doc.NewElement("progress");
        c5e->SetAttribute("p", webprogress);
        root->InsertEndChild(c5e);

        if (doc.SaveFile(savefilename.c_str()) != tinyxml2::XML_SUCCESS)
        {
            std::cerr << "err saving" << std::endl;
        }
        return;
    }

    void Load()
    {
        tinyxml2::XMLDocument doc;

        if (doc.LoadFile(savefilename.c_str()) != tinyxml2::XML_SUCCESS)
        {
            std::cerr << "Error loading" << std::endl;
            Save();
            return;
        }

        tinyxml2::XMLElement* root = doc.FirstChildElement("SaveData");
        if (root)
        {
            tinyxml2::XMLElement* bgcolorElement = root->FirstChildElement("bgcolor");
            if (bgcolorElement)
            {
                bgcolor.r = bgcolorElement->IntAttribute("r");
                bgcolor.g = bgcolorElement->IntAttribute("g");
                bgcolor.b = bgcolorElement->IntAttribute("b");
                bgcolor.a = bgcolorElement->IntAttribute("a");
            }
            else
            {
                std::cerr << "err" << std::endl;
            }

            tinyxml2::XMLElement* colorElement = root->FirstChildElement("color");
            if (colorElement)
            {
                textcolor.r = colorElement->IntAttribute("r");
                textcolor.g = colorElement->IntAttribute("g");
                textcolor.b = colorElement->IntAttribute("b");
                textcolor.a = colorElement->IntAttribute("a");
            }
            else
            {
                std::cerr << "err" << std::endl;
            }

            tinyxml2::XMLElement* _debug = root->FirstChildElement("debug");
            if (_debug)
            {
                debug = _debug->IntAttribute("value");
            }
            else
            {
                std::cerr << "err" << std::endl;
            }

            tinyxml2::XMLElement* _fullscreen = root->FirstChildElement("fullscreen");
            if (_fullscreen)
            {
                fullscreen = _fullscreen->IntAttribute("value");
            }
            else
            {
                std::cerr << "err" << std::endl;
            }

            tinyxml2::XMLElement* _windowsize = root->FirstChildElement("windowsize");
            if (_windowsize)
            {
                WindowSize.x = _windowsize->IntAttribute("X");
                WindowSize.y = _windowsize->IntAttribute("Y");
            }
            else
            {
                std::cerr << "err" << std::endl;
            }

            tinyxml2::XMLElement* _progress = root->FirstChildElement("progress");
            if (_progress)
            {
                webprogress = _progress->IntAttribute("p");
            }
            else
            {
                std::cerr << "err" << std::endl;
            }
        }
        else
        {
            return;
        }
    }
}
