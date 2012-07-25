/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#pragma once
#ifndef GWEN_PLATFORM_H
#define GWEN_PLATFORM_H

#include "Gwen/Structures.h"
#include "Gwen/Events.h"

namespace Gwen
{
	struct Font;

	namespace Platform
	{
		//
		// Set the system cursor to iCursor
		// Cursors are defined in Structures.h
		//
		void SetCursor( unsigned char iCursor );

		//
		// Used by copy/paste
		//
		UnicodeString GetClipboardText();
		bool SetClipboardText( const UnicodeString& str );

		//
		// Needed for things like double click
		//
		float GetTimeInSeconds();

		//
		// System Dialogs ( Can return false if unhandled )
		//
		bool FileOpen( const String& Name, const String& StartPath, const String& Extension, Gwen::Event::Handler* pHandler, Event::Handler::FunctionStr fnCallback );
		bool FileSave( const String& Name, const String& StartPath, const String& Extension, Gwen::Event::Handler* pHandler, Event::Handler::FunctionStr fnCallback );

        String ResolveFontPath(Gwen::Font* pFont);
        UnicodeString GetDefaultFontFace();
        float GetDefaultFontSize();

		bool IsModifierKeyDown(int key);
	}

}
#endif
