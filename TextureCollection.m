//
//  TextureCollection.m
//  TrenchBroom
//
//  Created by Kristian Duske on 08.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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
