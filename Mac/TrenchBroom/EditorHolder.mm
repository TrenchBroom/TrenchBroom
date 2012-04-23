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


#import "EditorHolder.h"
#import "Editor.h"

using namespace TrenchBroom;
using namespace TrenchBroom::Controller;

@implementation EditorHolder

- (id)initWithDefinitionPath:(NSString *)definitionPath palettePath:(NSString *)palettePath {
    if ((self = [self init])) {
        const char* definitionPathC = [definitionPath cStringUsingEncoding:NSASCIIStringEncoding];
        const char* palettePathC = [palettePath cStringUsingEncoding:NSASCIIStringEncoding];
        
        editor = new Editor(definitionPathC, palettePathC);
    }
    
    return self;
}

- (void)dealloc {
    delete ((Editor *)editor);
    [super dealloc];
}

- (void*) editor {
    return editor;
}

@end
