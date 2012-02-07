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

@interface ArrowFigure : NSObject <Figure> {
    TVector3f* shaftVertices;
    TVector3f* shaftVertexNormals;
    TVector3f* shaftSurfaceNormals;
    TVector3f* shaftSurfacePositions;
    TVector3f shaftCapNormal;
    TVector3f shaftCapPosition;
    int shaftVertexCount;
    int shaftSurfaceCount;
    
    TVector3f* headVertices;
    TVector3f* headVertexNormals;
    TVector3f* headSurfaceNormals;
    TVector3f* headSurfacePositions;
    TVector3f headCapNormal;
    TVector3f headCapPosition;
    
    int headVertexCount;
    int headSurfaceCount;
    
    TVector4f fillColor;
    TVector4f outlineColor;
    
    TVector3f cameraPosition;
    TVector3f position;
    float scale;
}

- (id)initWithDirection:(const TVector3f *)theDirection shaftRadius:(float)theShaftRadius shaftLength:(float)theShaftLength headRadius:(float)theHeadRadius headLength:(float)theHeadLength;

- (void)setPosition:(const TVector3f *)thePosition;
- (void)setCameraPosition:(const TVector3f *)theCameraPosition;
- (void)setScale:(float)theScale;
- (void)setFillColor:(const TVector4f *)theFillColor;
- (void)setOutlineColor:(const TVector4f *)theOutlineColor;

@end
