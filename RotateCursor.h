//
//  RotateCursor.h
//  TrenchBroom
//
//  Created by Kristian Duske on 04.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Cursor.h"

@interface RotateCursor : NSObject <Cursor> {
@private
    TVector3f center;
    EAxis vAxis;
    float hAngle;
    float vAngle;
    BOOL drag;
    float radius;
    float initialHAngle;
    float initialVAngle;
}

- (void)updateCenter:(TVector3f *)theCenter radius:(float)theRadius verticalAxis:(EAxis)theVerticalAxis initialHAngle:(float)theInitialHAngle initialVAngle:(float)theInitialVAngle;
- (void)setDragging:(BOOL)isDragging;
- (void)updateHorizontalAngle:(float)theHAngle verticalAngle:(float)theVAngle;

@end
