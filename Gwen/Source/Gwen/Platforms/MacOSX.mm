/*
 GWEN
 Copyright (c) 2011 Facepunch Studios
 See license in Gwen.h
 */

#include "Gwen/Macros.h"
#include "Gwen/Platform.h"
#include "Gwen/Font.h"
#include "Gwen/Utility.h"
#include <fstream>

#ifdef __APPLE__

using namespace Gwen;
using namespace Gwen::Platform;

static NSCursor* iCursorConvertion[] = 
{
	[NSCursor arrowCursor], 
	[NSCursor IBeamCursor],
	[NSCursor resizeUpDownCursor],
	[NSCursor resizeLeftRightCursor],
	[NSCursor arrowCursor], // TODO: make proper cursor for resizing diagonal nwse directions
	[NSCursor arrowCursor], // TODO: make proper cursor for resizing diagonal nesw directions
	[NSCursor crosshairCursor], // TODO: make proper cursor for resizing in all directions
	[NSCursor arrowCursor], // TODO: make proper cursor for 'forbidden'
	[NSCursor arrowCursor], // TODO: make proper cursor for 'wait'
	[NSCursor pointingHandCursor]
};

static Gwen::UnicodeString gs_ClipboardEmulator;

void Gwen::Platform::SetCursor( unsigned char iCursor )
{
    [NSCursor pop];
	[iCursorConvertion[iCursor] push];
}

Gwen::UnicodeString Gwen::Platform::GetClipboardText()
{
	return gs_ClipboardEmulator;
}

bool Gwen::Platform::SetClipboardText( const Gwen::UnicodeString& str )
{
	gs_ClipboardEmulator = str;
	return true;
}

float Gwen::Platform::GetTimeInSeconds()
{
    NSTimeInterval oldTime = [[NSDate date] timeIntervalSince1970];
    return static_cast<float>(oldTime);
}

bool Gwen::Platform::FileOpen( const String& Name, const String& StartPath, const String& Extension, Gwen::Event::Handler* pHandler, Event::Handler::FunctionStr fnCallback )
{
	// No platform independent way to do this.
	// Ideally you would open a system dialog here
    
	return false;
}

bool Gwen::Platform::FileSave( const String& Name, const String& StartPath, const String& Extension, Gwen::Event::Handler* pHandler, Gwen::Event::Handler::FunctionStr fnCallback )
{
	// No platform independent way to do this.
	// Ideally you would open a system dialog here
    
	return false;
}

String Gwen::Platform::ResolveFontPath(Gwen::Font* pFont) {
	String fontDirectoryPaths[2] = {"/System/Library/Fonts/", "/Library/Fonts/"};
	String extensions[2] = {".ttf", ".ttc"};
    String facename = Gwen::Utility::UnicodeToString(pFont->facename);
    
	for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            String fontPath = fontDirectoryPaths[i] + facename + extensions[j];
            std::fstream fs(fontPath.c_str(), std::ios::binary | std::ios::in);
            if (fs.is_open())
                return fontPath;
        }
	}

    return "/System/Library/Fonts/LucidaGrande.ttc";
}

UnicodeString Gwen::Platform::GetDefaultFontFace() {
    return L"LucidaGrande";
}

float Gwen::Platform::GetDefaultFontSize() {
    return 13.0f;
}

#endif
