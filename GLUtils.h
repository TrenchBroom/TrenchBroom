//
//  GLUtils.h
//  TrenchBroom
//
//  Created by Kristian Duske on 10.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <OpenGL/gl.h>
#import "Math.h"

void glVertexV3f(const TVector3f* v);
void glSetEdgeOffset(float f);
void glResetEdgeOffset();