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

#import "BspFace.h"


@implementation BspFace

- (id)initWithTextureInfo:(TTextureInfo *)theTextureInfo vertices:(TVector3f *)theVertices vertexCount:(int)theVertexCount {
    NSAssert(theTextureInfo != NULL, @"texture info must not be NULL");
    NSAssert(theVertices != NULL, @"vertex array must not be NULL");
    NSAssert(theVertexCount >= 3, @"vertex count must be at least 3");
    
    if ((self = [self init])) {
        textureInfo = *theTextureInfo;
        vertexCount = theVertexCount;
        vertices = malloc(vertexCount * sizeof(TVector3f));
        memcpy(vertices, theVertices, vertexCount * sizeof(TVector3f));
        
        bounds.min = vertices[0];
        bounds.max = vertices[0];
        
        for (int i = 1; i < vertexCount; i++)
            mergeBoundsWithPoint(&bounds, &vertices[i], &bounds);
    }
    
    return self;
}

- (TBoundingBox *)bounds {
    return &bounds;
}

- (TTextureInfo *)textureInfo {
    return &textureInfo;
}

- (TVector3f *)vertices {
    return vertices;
}

- (int)vertexCount {
    return vertexCount;
}

- (void)texCoords:(TVector2f *)theTexCoords forVertex:(TVector3f *)theVertex {
    NSAssert(theTexCoords != NULL, @"texture coordinate vector must not be NULL");
    NSAssert(theVertex != NULL, @"vertex must not be nil");
    
    theTexCoords->x = (dotV3f(theVertex, &textureInfo.sAxis) + textureInfo.sOffset) / [textureInfo.texture width];
    theTexCoords->y = (dotV3f(theVertex, &textureInfo.tAxis) + textureInfo.tOffset) / [textureInfo.texture height];
}

- (void)dealloc {
    free(vertices);
    [super dealloc];
}

@end
