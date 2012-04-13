/*
 Copyright (C) 2010-2012 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#import "Document.h"
#import "Editor.h"
#import "FontManager.h"
#import "MacStringFactory.h"
#import "MacProgressIndicator.h"
#import "DocumentWindowController.h"
#import <string>

using namespace TrenchBroom;
using namespace TrenchBroom::Controller;
using namespace TrenchBroom::Renderer;

@implementation Document

- (id)init {
    self = [super init];
    if (self) {
        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* definitionPath = [mainBundle pathForResource:@"quake" ofType:@"def"];
        const char* definitionPathC = [definitionPath cStringUsingEncoding:NSASCIIStringEncoding];
        NSString* palettePath = [mainBundle pathForResource:@"QuakePalette" ofType:@"lmp"];
        const char* palettePathC = [palettePath cStringUsingEncoding:NSASCIIStringEncoding];
        
        editor = new Editor(definitionPathC, palettePathC);
    }
    return self;
}

- (NSString *)windowNibName {
    return @"Document";
}

- (void)makeWindowControllers {
	DocumentWindowController* controller = [[DocumentWindowController alloc] initWithWindowNibName:@"DocumentWindow"];
	[self addWindowController:controller];
    [controller release];
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
    return nil;
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError {
    NSString* path = [absoluteURL path];
    const char* pathC = [path cStringUsingEncoding:NSASCIIStringEncoding];

    MacProgressIndicator* indicator = new MacProgressIndicator("Loading map file...");
    ((Editor*)editor)->loadMap(pathC, indicator);
    delete indicator;
    
    return YES;
}

+ (BOOL)autosavesInPlace {
    return YES;
}

- (void*)editor {
    return editor;
}

@end
