//
//  FaceOffsetTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Face;

@interface FaceOffsetTool : NSObject {
    @private
    Face* face;
}

/*!
    @function
    @abstract   Initializes the face offset tool with the specified face.
    @param      theFace The face which is edited through this tool.
    @result     The initialized face offset tool.
*/
- (id)initWithFace:(Face *)theFace;

/*!
    @function
    @abstract   Renders the visual handle for this tool
*/
- (void)render;

@end
