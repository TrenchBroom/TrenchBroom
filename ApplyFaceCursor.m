//
//  ApplyFaceCursor.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ApplyFaceCursor.h"
#import "Matrix4f.h"
#import "Face.h"

@implementation ApplyFaceCursor

- (void)render {
    glDisable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT, GL_LINE);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    glTranslatef(position.x - center.x, position.y - center.y, position.z - center.z);
    glMultMatrixf([matrix columnMajor]);

    glLineWidth(2);
    glColor4f(1, 1, 0, 1);

    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(-8, -8, 0);
    glVertex3f(-8, +8, 0);
    glVertex3f(+8, +8, 0);
    glVertex3f(+8, -8, 0);
    glEnd();
    
    if (applyFlags) {
        glLineWidth(3);
        glBegin(GL_LINES);
        glVertex3f(-6, 6, 0);
        glVertex3f(+2, 6, 0);
        glVertex3f(-6, 4, 0);
        glVertex3f(+5, 4, 0);
        glVertex3f(-6, 2, 0);
        glVertex3f(+5, 2, 0);
        glVertex3f(-6, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(-6, -2, 0);
        glVertex3f(3, -2, 0);
        glVertex3f(-6, -4, 0);
        glVertex3f(5, -4, 0);
        glVertex3f(-6, -6, 0);
        glVertex3f(1, -6, 0);
        glEnd();
    }
    
    glLineWidth(1);
    glPopMatrix();
    
}

- (void)setFace:(id <Face>)theFace {
    [matrix release];
    matrix = [[theFace surfaceToWorldMatrix] retain];
    center = *[theFace center];
}

- (void)setApplyFlags:(BOOL)doApplyFlags {
    applyFlags = doApplyFlags;
}

- (void)update:(const TVector3f *)thePosition {
    position = *thePosition;
}

- (void)dealloc {
    [matrix release];
    [super dealloc];
}


@end
