/*
 GWEN
 Copyright (c) 2011 Facepunch Studios
 See license in Gwen.h
 */

#include "Gwen/Macros.h"
#include "Gwen/Platform.h"

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

#endif
