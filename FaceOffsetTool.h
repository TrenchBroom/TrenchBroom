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
    Face* face;
    SelectionManager* selectionManager;
    Vector3f* lastSurfacePos;
}

/*!
    @function
    @abstract   Initializes the face offset tool with the specified face.
    @param      theFace The face which is edited through this tool.
    @param      theSelectionManager The selection manager.
    @result     The initialized face offset tool.
*/
- (id)initWithFace:(Face *)theFace selectionManager:(SelectionManager *)theSelectionManager;

/*!
    @function
    @abstract   Renders the visual handle for this tool
*/
- (void)render;

@end
