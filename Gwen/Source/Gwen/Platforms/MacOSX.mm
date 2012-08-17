/*
 GWEN
 Copyright (c) 2011 Facepunch Studios
 See license in Gwen.h
 */

#include "NSString+StdStringAdditions.h"

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
            NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
            NSArray* objects = [pasteboard readObjectsForClasses:[NSArray arrayWithObject:[NSString class]] options:nil];
            NSString* nsStr = [objects objectAtIndex:0];
            return [nsStr stdWString];
        }
        
        bool SetClipboardText( const UnicodeString& str )
        {
            NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
            [pasteboard clearContents];
            
            NSString* nsStr = [NSString stringWithStdWString:str];
            
            NSArray* pasteboardObjects = [[NSArray alloc] initWithObjects:nsStr, nil];
            [pasteboard writeObjects:pasteboardObjects];
            [pasteboardObjects release];

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
            [openPanel setAllowedFileTypes:[NSArray arrayWithObject:[NSString stringWithStdString:Extension]]];
            [openPanel setAllowsOtherFileTypes:NO];
            [openPanel setTitle:[NSString stringWithStdString:Name]];
            [openPanel setNameFieldLabel:@"File"];
            [openPanel setCanCreateDirectories:NO];
            
            NSURL* startPathUrl = [NSURL fileURLWithPath:[NSString stringWithStdString:StartPath]];
            [openPanel setDirectoryURL:startPathUrl];
            
            if ([openPanel runModal] == NSFileHandlingPanelOKButton) {
                for (NSURL* url in [openPanel URLs]) {
                    NSString* path = [url path];
                    if (path == nil)
                        return false;
                    
                    if (pHandler && fnCallback)
                        (pHandler->*fnCallback)([path stdString]);
                    
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
