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
#import "Math.h"

@interface DragFaceCursor : NSObject <Cursor> {
    TVector3f position;
    float angle;
    TVector3f axis;
    GLUquadric* arm;
    GLUquadric* disks;
    BOOL initialized;
    
}

- (void)setDragDir:(const TVector3f *)theDragDir;

@end
