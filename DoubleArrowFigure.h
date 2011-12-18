//
//  DoubleArrowFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 17.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

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
}

- (id)initWithDirection:(EAxis)theDirection;

- (void)setPosition:(const TVector3f *)thePosition;
- (void)setCameraPosition:(const TVector3f *)theCameraPosition;
- (void)setFillColor:(const TVector4f *)theFillColor;
- (void)setOutlineColor:(const TVector4f *)theOutlineColor;

@end
