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
    TVector3f* shaftNormals;
    int shaftVertexCount;

    TVector3f* headVertices;
    TVector3f* headNormals;
    int headVertexCount;
    
    TVector4f fillColor;
    TVector4f outlineColor;
    
    const TVector3f* cameraPos;
}

- (void)setCameraPosition:(const TVector3f *)theCameraPosition;

@end
