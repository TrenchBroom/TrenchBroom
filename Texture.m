//
//  Texture.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Texture.h"
#import "IdGenerator.h"
#import "WadTextureEntry.h"

@interface Texture (private)

- (NSData *)convertTexture:(WadTextureEntry *)textureEntry palette:(NSData *)thePalette;

@end

@implementation Texture (private)

- (NSData *)convertTexture:(WadTextureEntry *)textureEntry palette:(NSData *)thePalette {
    uint8_t pixelBuffer[32];
    uint8_t colorBuffer[3];
    
    int w = [textureEntry width];
    int h = [textureEntry height];
    int size = w * h;
    
    NSData* textureData = [textureEntry mip0];
    NSMutableData* texture = [[NSMutableData alloc] initWithCapacity:size * 3];
    
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int pixelIndex = y * w + x;
            int pixelBufferIndex = pixelIndex % 32;
            if (pixelBufferIndex == 0) {
                int length = fmin(32, size - (pixelBufferIndex * 32));
                [textureData getBytes:pixelBuffer range:NSMakeRange(pixelIndex, length)];
            }
            
            int paletteIndex = pixelBuffer[pixelBufferIndex];
            [thePalette getBytes:colorBuffer range:NSMakeRange(paletteIndex * 3, 3)];
            [texture appendBytes:colorBuffer length:3];
        }
    }
    
    return [texture autorelease];
}

@end


@implementation Texture

- (id)initWithWadEntry:(WadTextureEntry *)theEntry palette:(NSData *)thePalette {
    NSAssert(theEntry != nil, @"texture entry must not be nil");
    NSAssert(thePalette != nil, @"palette must not be nil");
    
    if (self = [self init]) {
        uniqueId = [[[IdGenerator sharedGenerator] getId] retain];
        
        name = [[theEntry name] retain];
        data = [[self convertTexture:theEntry palette:thePalette] retain];
        width = [theEntry width];
        height = [theEntry height];
        textureId = 0;
    }
    
    return self;
}

- (NSString *)name {
    return name;
}

- (NSNumber *)uniqueId {
    return uniqueId;
}

- (int)width {
    return width;
}

- (int)height {
    return height;
}

- (void)activate {
    if (textureId == 0) {
        glGenTextures(1, &textureId);

        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, [data bytes]);
        [data release];
        data = nil;
    }
    
    glBindTexture(GL_TEXTURE_2D, textureId);
}

- (void)deactivate {
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (NSComparisonResult)compareByName:(Texture *)texture {
    return [name compare:[texture name]];
}

- (void)dealloc {
    if (textureId != 0)
        glDeleteTextures(1, &textureId);
    [uniqueId release];
    [name release];
    [super dealloc];
}
@end
