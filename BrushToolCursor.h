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
#import "Vector3f.h"

@interface BrushToolCursor : NSObject <Cursor> {
    Vector3f* position;
    GLUquadric* arms;
    GLUquadric* disks;
    BOOL initialized;
    EVectorComponent planeNormal;
}

- (void)setPlaneNormal:(EVectorComponent)thePlaneNormal;

@end
