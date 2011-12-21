//
//  ArrowFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 17.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

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
}

- (id)initWithDirection:(const TVector3f *)theDirection;

- (void)setPosition:(const TVector3f *)thePosition;
- (void)setCameraPosition:(const TVector3f *)theCameraPosition;
- (void)setFillColor:(const TVector4f *)theFillColor;
- (void)setOutlineColor:(const TVector4f *)theOutlineColor;

@end
