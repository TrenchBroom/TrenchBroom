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

#import "TextureCollection.h"
#import <OpenGL/gl.h>
#import "Texture.h"
#import "Wad.h"
#import "WadEntry.h"
#import "WadTextureEntry.h"

@implementation TextureCollection

- (id)initName:(NSString *)theName palette:(NSData *)thePalette wad:(Wad *)theWad {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(thePalette != nil, @"palette must not be nil");
    NSAssert(theWad != nil, @"wad must not be nil");
    
    if ((self = [self init])) {
        name = [theName retain];

        NSArray* textureEntries = [theWad textureEntries];
        textures = [[NSMutableArray alloc] initWithCapacity:[textureEntries count]];
        
        for (int i = 0; i < [textureEntries count]; i++) {
            WadTextureEntry* textureEntry = [textureEntries objectAtIndex:i];
            Texture* texture = [[Texture alloc] initWithWadEntry:textureEntry palette:thePalette];
            [textures addObject:texture];
            [texture release];
        }
    }
    
    return self;
}

- (NSString *)name {
    return name;
}

- (NSArray *)textures {
    return textures;
}

- (void)dealloc {
    [name release];
    [textures release];
    [super dealloc];
}

@end
