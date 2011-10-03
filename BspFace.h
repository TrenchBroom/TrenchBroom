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

#import <Foundation/Foundation.h>
#import "Bsp.h"

@interface BspFace : NSObject {
@private
    TBoundingBox bounds;
    TTextureInfo textureInfo;
    TVector3f* vertices;
    int vertexCount;
}

- (id)initWithTextureInfo:(TTextureInfo *)theTextureInfo vertices:(TVector3f *)theVertices vertexCount:(int)theVertexCount;

- (TBoundingBox *)bounds;
- (TTextureInfo *)textureInfo;
- (TVector3f *)vertices;
- (int)vertexCount;
- (void)texCoords:(TVector2f *)theTexCoords forVertex:(TVector3f *)theVertex;

@end
