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

#import "FaceInfo.h"
#import "Face.h"
#import "MutableFace.h"
#import "TextureManager.h"
#import "Texture.h"

@implementation FaceInfo

+ (id)faceInfoFor:(id <Face>)theFace {
    return [[[FaceInfo alloc] initWithFace:theFace] autorelease];
}

- (id)initWithFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    
    if ((self = [self init])) {
        faceId = [[theFace faceId] retain];
        point1 = *[theFace point1];
        point2 = *[theFace point2];
        point3 = *[theFace point3];
        xOffset = [theFace xOffset];
        yOffset = [theFace yOffset];
        xScale = [theFace xScale];
        yScale = [theFace yScale];
        rotation = [theFace rotation];
        textureName = [[NSString alloc] initWithString:[[theFace texture] name]];
    }
    
    return self;
}

- (void)dealloc {
    [faceId release];
    [textureName release];
    [super dealloc];
}
                   
- (void)updateFace:(MutableFace *)theFace textureManager:(TextureManager *)theTextureManager {
    NSAssert([faceId isEqualToNumber:[theFace faceId]], @"face id must be equal");
    
    [theFace setPoint1:&point1 point2:&point2 point3:&point3];
    [theFace setXOffset:xOffset];
    [theFace setYOffset:yOffset];
    [theFace setXScale:xScale];
    [theFace setYScale:yScale];
    [theFace setRotation:rotation];
    
    Texture* texture = [theTextureManager textureForName:textureName];
    [theFace setTexture:texture];
}

@end
