//
//  ApplyFaceCursor.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"
#import "Cursor.h"

@protocol Face;

@interface ApplyFaceCursor : NSObject <Cursor> {
    TVector3f position;
    TVector3f center;
    TMatrix4f matrix;
    BOOL applyFlags;
}

- (void)setFace:(id <Face>)theFace;
- (void)setApplyFlags:(BOOL)doApplyFlags;

@end
