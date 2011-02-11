//
//  FaceOffsetTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Tool.h"

@class Face;
@class SelectionManager;
@class Ray3D;
@class Vector3f;

@interface FaceOffsetTool : NSObject <Tool> {
    @private
    NSMutableArray* faces;
    NSMutableArray* figures;
    Vector3f* lastSurfacePos;
    Face* draggedFace;
    int moveLeftKey;
    int moveRightKey;
    int moveUpKey;
    int moveDownKey;
}

- (Face *)faceHitByRay:(Ray3D *)theRay;

@end
