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

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>

@class WadTextureEntry;
@class AliasSkin;
@class BspTexture;

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
- (id)initWithName:(NSString *)theName skin:(AliasSkin *)theSkin index:(int)theIndex palette:(NSData *)thePalette;
- (id)initWithBspTexture:(BspTexture *)theBspTexture palette:(NSData *)thePalette;
- (id)initWithName:(NSString *)theName image:(NSData *)theImage width:(int)theWidth height:(int)theHeight palette:(NSData *)thePalette;
- (id)initDummyWithName:(NSString *)theName;

- (NSString *)name;
- (NSNumber *)uniqueId;
- (int)width;
- (int)height;
- (BOOL)dummy;

- (void)incUsageCount;
- (void)decUsageCount;
- (void)setUsageCount:(int)theUsageCount;
- (int)usageCount;

- (void)activate;
- (void)deactivate;

- (NSComparisonResult)compareByName:(Texture *)texture;
- (NSComparisonResult)compareByUsageCount:(Texture *)texture;
@end
