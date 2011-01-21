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

NSString* const UnknownTextureNameException = @"UnknownTextureNameException";
NSString* const MissingPaletteException = @"MissingPaletteException";

@implementation TextureManager

- (id)init {
    if (self = [super init]) {
        textures = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithPalette:(NSData *)thePalette {
    if (thePalette == nil)
        [NSException raise:NSInvalidArgumentException format:@"palette must not be nil"];
    
    if (self = [self init]) {
        palette = [thePalette retain];
    }
    
    return self;
}

- (NSData *)convertTexture:(WadTextureEntry *)textureEntry {
    uint8_t pixelBuffer[32];
    uint8_t colorBuffer[3];
    
    int width = [textureEntry width];
    int height = [textureEntry height];
    
    NSData* textureData = [textureEntry mip0];
    
    int size = width * height;
    NSMutableData* texture = [[NSMutableData alloc] initWithCapacity:size * 3];
    
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            int pixelIndex = y * width + x;
            int pixelBufferIndex = pixelIndex % 32;
            if (pixelBufferIndex == 0) {
                int length = fmin(32, size - (pixelBufferIndex * 32));
                [textureData getBytes:pixelBuffer range:NSMakeRange(pixelIndex, length)];
            }
            
            int paletteIndex = pixelBuffer[pixelBufferIndex];
            [palette getBytes:colorBuffer range:NSMakeRange(paletteIndex * 3, 3)];
            [texture appendBytes:colorBuffer length:3];
        }
    }
    
    return [texture autorelease];
}

- (void)loadTexturesFrom:(Wad *)wad {
    if (wad == nil)
        [NSException raise:NSInvalidArgumentException format:@"wad must not be nil"];
    
    glEnable(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    NSArray* textureEntries = [wad textureEntries];
    
    GLsizei numTextures = [textureEntries count];
    GLuint textureIds[numTextures];
    
    glGenTextures(numTextures, textureIds);
    
    NSLog(@"loading %i textures from %@", [textureEntries count], [wad name]);
    for (int i = 0; i < numTextures; i++) {
        WadTextureEntry* textureEntry = [textureEntries objectAtIndex:i];
        int texId = textureIds[i];

        NSData* textureData = [self convertTexture:textureEntry];
        
        int width = [textureEntry width];
        int height = [textureEntry height];
        const void* buffer = [textureData bytes];
        
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
        
        [textures setObject:[NSNumber numberWithInt:texId] forKey:[textureEntry name]];
    }
}

- (void)activateTexture:(NSString *)name {
    if (name == nil)
        [NSException raise:NSInvalidArgumentException format:@"name must not be nil"];
    
    NSNumber* texId = [textures objectForKey:name];
    if (texId == nil)
        [NSException raise:UnknownTextureNameException format:@"unknown texture name: %@", name];
    
    glBindTexture(GL_TEXTURE_2D, [texId intValue]);
}

- (void)deactivateTexture {
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (void)disposeTextures {
    GLuint textureIds[[textures count]];
    
    NSEnumerator* texIdEn = [textures keyEnumerator];
    NSNumber* texId;
    
    int i = 0;
    while ((texId = [texIdEn nextObject]))
        textureIds[i++] = [texId intValue];
    
    glDeleteTextures([textures count], textureIds);
    [textures removeAllObjects];
}

- (void)dealloc {
    [palette release];
    [textures release];
    [super dealloc];
}

@end
