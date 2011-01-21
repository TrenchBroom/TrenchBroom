//
//  TextureManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "TextureManager.h"
#import "Wad.h"
#import "WadPaletteEntry.h"
#import "WadTextureEntry.h"

@implementation TextureManager

- (id)init {
    if (self = [super init]) {
        textures = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)loadTexturesFrom:(Wad *)wad {
    if (wad == nil)
        [NSException raise:NSInvalidArgumentException format:@"wad must not be nil"];
    
    glEnable(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    WadPaletteEntry* paletteEntry = [[wad paletteEntries] objectAtIndex:0];
    NSArray* textureEntries = [wad textureEntries];
    
    GLuint textureIds[[textureEntries count]];
    
    glGenTextures([textureEntries count], textureIds);
    
    for (int i = 0; i < [textureEntries count]; i++) {
        int texId = textureIds[i];
        NSLog(@"loading texture %i with ID %i", i, texId);
        
        WadTextureEntry* textureEntry = [textureEntries objectAtIndex:i];
        
        glBindTexture(GL_TEXTURE_2D, texId);
        glColorTable(GL_TEXTURE_2D, GL_RGB8, 256, GL_RGB, GL_UNSIGNED_BYTE, [[paletteEntry data] bytes]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_COLOR_INDEX, [textureEntry width], [textureEntry height], 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, [[textureEntry mip0] bytes]);
        
        [textures setObject:[NSNumber numberWithInt:texId] forKey:[textureEntry name]];
    }
}

- (void)dealloc {
    [textures release];
    [super dealloc];
}

@end
