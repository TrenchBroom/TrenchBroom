//
//  FaceOffsetTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FaceTool.h"
#import "Tool.h"

@class Face;
@class Ray3D;
@class Vector3f;
@class Options;

@interface FaceOffsetTool : FaceTool {
    @private
    int moveLeftKey;
    int moveRightKey;
    int moveUpKey;
    int moveDownKey;
}

@end
