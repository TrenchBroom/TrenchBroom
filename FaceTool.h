//
//  FaceTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 18.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Tool.h"

@class Vector3f;
@class Ray3D;
@class Face;
@class Options;

@interface FaceTool : NSObject <Tool> {
    NSMutableArray* faces;
    NSMutableArray* figures;
    Vector3f* lastSurfacePos;
    Face* draggedFace;
    Options* options;
}

- (id)initWithOptions:(Options *)theOptions;

- (Face *)faceHitByRay:(Ray3D *)theRay;

@end
