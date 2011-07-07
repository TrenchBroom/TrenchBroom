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
#import "AliasSkin.h"
#import "Math.h"

@interface Texture (private)

- (NSData *)convertTexture:(NSData *)theTextureData width:(int)theWidth height:(int)theHeight palette:(NSData *)thePalette;

@end

@implementation Texture (private)

- (NSData *)convertTexture:(NSData *)theTextureData width:(int)theWidth height:(int)theHeight palette:(NSData *)thePalette {
    uint8_t pixelBuffer[32];
    uint8_t colorBuffer[3];
    
    int size = theWidth * theHeight;
    NSMutableData* texture = [[NSMutableData alloc] initWithCapacity:size * 3];
    
    for (int y = 0; y < theHeight; y++) {
        for (int x = 0; x < theWidth; x++) {
            int pixelIndex = y * theWidth + x;
            int pixelBufferIndex = pixelIndex % 32;
            if (pixelBufferIndex == 0) {
                int length = mini(32, size - pixelIndex);
                [theTextureData getBytes:pixelBuffer range:NSMakeRange(pixelIndex, length)];
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
    return [self initWithName:[theEntry name] image:[theEntry mip0] width:[theEntry width] height:[theEntry height] palette:thePalette];
}

- (id)initWithName:(NSString *)theName skin:(AliasSkin *)theSkin index:(int)theIndex palette:(NSData *)thePalette {
    NSAssert(theSkin != nil, @"skin must not be nil");
    return [self initWithName:theName image:[theSkin pictureAtIndex:theIndex] width:[theSkin width] height:[theSkin height] palette:thePalette];
}

- (id)initWithName:(NSString *)theName image:(NSData *)theImage width:(int)theWidth height:(int)theHeight palette:(NSData *)thePalette {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theImage != nil, @"image must not be nil");
    NSAssert(theWidth > 0, @"width must be positive");
    NSAssert(theHeight > 0, @"height must be positive");
    NSAssert(thePalette != nil, @"palette must no be nil");
    
    if ((self = [self init])) {
        uniqueId = [[[IdGenerator sharedGenerator] getId] retain];
        
        name = [[NSString alloc] initWithString:theName];
        width = theWidth;
        height = theHeight;
        data = [[self convertTexture:theImage width:width height:height palette:thePalette] retain];
        
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

- (void)incUsageCount {
    usageCount++;
}

- (void)decUsageCount {
    usageCount--;
}

- (void)setUsageCount:(int)theUsageCount {
    usageCount = theUsageCount;
}

- (int)usageCount {
    return usageCount;
}

- (void)activate {
    if (textureId == 0) {
        if (data != nil) {
            glGenTextures(1, &textureId);
            
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, [data bytes]);
            [data release];
            data = nil;
        } else {
            NSLog(@"Warning: cannot recreate texture '%@'", self);
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, textureId);
}

- (void)deactivate {
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (NSComparisonResult)compareByName:(Texture *)texture {
    return [name compare:[texture name]];
}

- (NSComparisonResult)compareByUsageCount:(Texture *)texture {
    if (usageCount > [texture usageCount])
        return NSOrderedAscending;
    if (usageCount < [texture usageCount])
        return NSOrderedDescending;
    return [self compareByName:texture];
}

- (void)dealloc {
    if (textureId != 0)
        glDeleteTextures(1, &textureId);
    [uniqueId release];
    [name release];
    [data release];
    [super dealloc];
}
@end
