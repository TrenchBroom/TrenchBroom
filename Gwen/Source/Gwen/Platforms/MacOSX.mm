/*
 GWEN
 Copyright (c) 2011 Facepunch Studios
 See license in Gwen.h
 */

#include "Gwen/Macros.h"
#include "Gwen/Platform.h"
#include "Gwen/Font.h"
#include "Gwen/InputHandler.h"
#include "Gwen/Utility.h"
#include <fstream>

#ifdef __APPLE__

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

namespace Gwen {
    namespace Platform {
        void SetCursor( unsigned char iCursor )
        {
            [NSCursor pop];
            [iCursorConvertion[iCursor] push];
        }
        
        UnicodeString GetClipboardText()
        {
            return gs_ClipboardEmulator;
        }
        
        bool SetClipboardText( const UnicodeString& str )
        {
            gs_ClipboardEmulator = str;
            return true;
        }
        
        float GetTimeInSeconds()
        {
            NSTimeInterval oldTime = [[NSDate date] timeIntervalSince1970];
            return static_cast<float>(oldTime);
        }
        
        bool FileOpen( const String& Name, const String& StartPath, const String& Extension, Event::Handler* pHandler, Event::Handler::FunctionStr fnCallback )
        {
            NSOpenPanel* openPanel = [NSOpenPanel openPanel];
            [openPanel setCanChooseFiles:YES];
            [openPanel setCanChooseDirectories:YES];
            [openPanel setAllowsMultipleSelection:NO];
            [openPanel setAllowedFileTypes:[NSArray arrayWithObject:[NSString stringWithCString:Extension.c_str() encoding:NSASCIIStringEncoding]]];
            [openPanel setAllowsOtherFileTypes:NO];
            [openPanel setTitle:[NSString stringWithCString:Name.c_str() encoding:NSASCIIStringEncoding]];
            [openPanel setNameFieldLabel:@"File"];
            [openPanel setCanCreateDirectories:NO];
            [openPanel setDirectory:[NSString stringWithCString:StartPath.c_str() encoding:NSASCIIStringEncoding]];
            
            if ([openPanel runModal] == NSFileHandlingPanelOKButton) {
                for (NSURL* url in [openPanel URLs]) {
                    NSString* path = [url path];
                    if (path == nil)
                        return false;
                    
                    String cppPath([path cStringUsingEncoding:NSASCIIStringEncoding]);
                    if (pHandler && fnCallback)
                        (pHandler->*fnCallback)(cppPath);
                    
                }
            } else {
                if (pHandler && fnCallback)
                    (pHandler->*fnCallback)("");
            }
            
            return false;
        }
        
        bool FileSave( const String& Name, const String& StartPath, const String& Extension, Event::Handler* pHandler, Event::Handler::FunctionStr fnCallback )
        {
            // No platform independent way to do this.
            // Ideally you would open a system dialog here
            
            return false;
        }
        
        String ResolveFontPath(Font* pFont) {
            String fontDirectoryPaths[2] = {"/System/Library/Fonts/", "/Library/Fonts/"};
            String extensions[2] = {".ttf", ".ttc"};
            String facename = Utility::UnicodeToString(pFont->facename);
            
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
        
        UnicodeString GetDefaultFontFace() {
            return L"LucidaGrande";
        }
        
        float GetDefaultFontSize() {
            return 13.0f;
        }

		bool IsModifierKeyDown(int key) {
            switch (key) {
                case Gwen::Key::Shift:
                    return ([NSEvent modifierFlags] & NSShiftKeyMask) != 0;
                case Gwen::Key::Control:
                    return ([NSEvent modifierFlags] & NSControlKeyMask) != 0;
                case Gwen::Key::Alt:
                    return ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0;
                case Gwen::Key::Command:
                    return ([NSEvent modifierFlags] & NSCommandKeyMask) != 0;
                default:
                    return false;
            }
        }
    }
}

#endif
