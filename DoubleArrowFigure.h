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

#import <Foundation/Foundation.h>
#import "Figure.h"
#import <OpenGL/gl.h>
#import "Math.h"

@interface DoubleArrowFigure : NSObject <Figure> {
    TVector3f* shaftVertices;
    TVector3f* shaftVertexNormals;
    TVector3f* shaftSurfaceNormals;
    TVector3f* shaftSurfacePositions;
    int shaftVertexCount;
    int shaftSurfaceCount;
    
    TVector3f* topHeadVertices;
    TVector3f* topHeadVertexNormals;
    TVector3f* topHeadSurfaceNormals;
    TVector3f* topHeadSurfacePositions;
    TVector3f topHeadCapNormal;
    TVector3f topHeadCapPosition;
    
    TVector3f* bottomHeadVertices;
    TVector3f* bottomHeadVertexNormals;
    TVector3f* bottomHeadSurfaceNormals;
    TVector3f* bottomHeadSurfacePositions;
    TVector3f bottomHeadCapNormal;
    TVector3f bottomHeadCapPosition;

    int headVertexCount;
    int headSurfaceCount;
    
    TVector4f fillColor;
    TVector4f outlineColor;
    
    TVector3f cameraPosition;
    TVector3f position;
    float scale;
}

- (id)initWithDirection:(EAxis)theDirection shaftRadius:(float)theShaftRadius shaftLength:(float)theShaftLength headRadius:(float)theHeadRadius headLength:(float)theHeadLength;

- (void)setPosition:(const TVector3f *)thePosition;
- (void)setCameraPosition:(const TVector3f *)theCameraPosition;
- (void)setScale:(float)theScale;
- (void)setFillColor:(const TVector4f *)theFillColor;
- (void)setOutlineColor:(const TVector4f *)theOutlineColor;

@end
