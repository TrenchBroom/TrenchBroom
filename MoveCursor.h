//
//  BrushToolCursor.h
//  TrenchBroom
//
//  Created by Kristian Duske on 14.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "Cursor.h"
#import "Math.h"

@interface MoveCursor : NSObject <Cursor> {
    TVector3f position;
    GLUquadric* arms;
    GLUquadric* disks;
    BOOL initialized;
    EAxis planeNormal;
}

- (void)setPlaneNormal:(EAxis)thePlaneNormal;

@end
