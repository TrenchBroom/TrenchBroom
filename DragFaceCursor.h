//
//  DragFaceCursor.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "Cursor.h"

@class Vector3f;

@interface DragFaceCursor : NSObject <Cursor> {
    Vector3f* position;
    float angle;
    Vector3f* axis;
    GLUquadric* arm;
    GLUquadric* disks;
    BOOL initialized;
    
}

- (void)setDragDir:(Vector3f *)theDragDir;

@end
