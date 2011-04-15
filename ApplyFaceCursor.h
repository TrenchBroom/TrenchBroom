//
//  ApplyFaceCursor.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Cursor.h"

@class Vector3f;
@class Matrix4f;
@protocol Face;

@interface ApplyFaceCursor : NSObject <Cursor> {
    Vector3f* position;
    Vector3f* center;
    Matrix4f* matrix;
    BOOL applyFlags;
}

- (void)setFace:(id <Face>)theFace;
- (void)setApplyFlags:(BOOL)doApplyFlags;

@end
