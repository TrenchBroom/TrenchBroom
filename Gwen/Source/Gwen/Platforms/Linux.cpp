/*
	GWEN
	Copyright (c) 2011 Facepunch Studios
	See license in Gwen.h
*/

#include "Gwen/Macros.h"
#include "Gwen/Platform.h"
#include "Gwen/Font.h"
#include "Gwen/Utility.h"

#ifdef __GNUC__

#include <gtkmm.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

#include <fstream>

static Gdk::CursorType iCursorConvertion[] =
{
	Gdk::ARROW,
	Gdk::XTERM,
	Gdk::SB_V_DOUBLE_ARROW,
	Gdk::SB_H_DOUBLE_ARROW,
	Gdk::SIZING,
	Gdk::SIZING,
	Gdk::SIZING,
	Gdk::ARROW, // STOP sign or NO
	Gdk::WATCH,
	Gdk::HAND1
};

namespace Gwen
{
    namespace Platform
    {

        void SetCursor( unsigned char iCursor )
        {
            Glib::RefPtr<Gdk::Cursor> cursor = Gdk::Cursor::create(iCursorConvertion[iCursor]);

            Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();
            Glib::RefPtr<Gdk::Window> window = screen->get_active_window();
            window->set_cursor(cursor);
        }

        Gwen::UnicodeString GetClipboardText()
        {
            return L"";
        }

        bool SetClipboardText( const Gwen::UnicodeString& str )
        {
            return true;
        }

        float GetTimeInSeconds()
        {
            struct timeval t;
            gettimeofday(&t, NULL);
            return static_cast<float>(t.tv_sec) + static_cast<float>(t.tv_usec) / (1000.0f * 1000.0f);
        }



        bool FileOpen( const String& Name, const String& StartPath, const String& Extension, Gwen::Event::Handler* pHandler, Event::Handler::FunctionStr fnCallback )
        {
            // TODO
            return false;
        }

        bool FileSave( const String& Name, const String& StartPath, const String& Extension, Gwen::Event::Handler* pHandler, Gwen::Event::Handler::FunctionStr fnCallback )
        {
            // TODO
            return false;
        }

        String ResolveFontPath(Gwen::Font* pFont) {
            /*
            String extensions[2] = {".ttf", ".ttc"};
            String facename = Gwen::Utility::UnicodeToString(pFont->facename);

            TCHAR windowsPathC[MAX_PATH];
            GetWindowsDirectory(windowsPathC, MAX_PATH);
            String windowsPath(windowsPathC);
            if (windowsPath.back() != '\\')
                windowsPath.push_back('\\');
            String fontDirectoryPath = windowsPath + "Fonts\\";
            String fontBasePath = fontDirectoryPath + facename;

            for (int i = 0; i < 2; i++) {
                String fontPath = fontBasePath + extensions[i];
                std::fstream fs(fontPath.c_str(), std::ios::binary | std::ios::in);
                if (fs.is_open())
                    return fontPath;
            }
            return fontDirectoryPath + "Arial.ttf";
            */

            return "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-R.ttf";
        }

        UnicodeString GetDefaultFontFace() {
            return L"Ubuntu-R";
        }

        float GetDefaultFontSize() {
            return 13.0f;
        }


    }
}

#endif // WIN32
