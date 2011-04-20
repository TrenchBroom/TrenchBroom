//
//  Texture.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>

@class WadTextureEntry;

@interface Texture : NSObject {
    NSString* name;
    NSNumber* uniqueId;
    GLuint textureId;
    int width;
    int height;
    int usageCount;
    NSData* data;
}

- (id)initWithWadEntry:(WadTextureEntry *)theEntry palette:(NSData *)thePalette;

- (NSString *)name;
- (NSNumber *)uniqueId;
- (int)width;
- (int)height;

- (void)incUsageCount;
- (void)decUsageCount;
- (void)setUsageCount:(int)theUsageCount;
- (int)usageCount;

- (void)activate;
- (void)deactivate;

- (NSComparisonResult)compareByName:(Texture *)texture;
- (NSComparisonResult)compareByUsageCount:(Texture *)texture;
@end
