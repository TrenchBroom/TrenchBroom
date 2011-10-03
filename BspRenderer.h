/*
Copyright (C) 2010-2011 Kristian Duske

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

#import <Foundation/Foundation.h>
#import "EntityRenderer.h"

@class Bsp;
@class VBOBuffer;
@class VBOMemBlock;

@interface BspRenderer : NSObject <EntityRenderer> {
@private
    Bsp* bsp;
    NSData* palette;
    VBOBuffer* vbo;
    VBOMemBlock* block;
    NSMutableDictionary* textures;
    NSMutableDictionary* indices;
    NSMutableDictionary* counts;
}

- (id)initWithBsp:(Bsp *)theBsp vbo:(VBOBuffer *)theVbo palette:(NSData *)thePalette;

@end
