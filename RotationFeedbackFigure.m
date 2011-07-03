//
//  RotationFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "RotationFeedbackFigure.h"

@implementation RotationFeedbackFigure

- (void)render {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // glDisable(GL_CULL_FACE);
    // glEnable(GL_DEPTH_TEST);

    float innerRadius = radius;
    float outerRadius = 2;

    int innerSegments = radius / 2;
    if (innerSegments < 12)
        innerSegments = 12;
    int outerSegments = 12;

    TVector3f points[innerSegments * outerSegments];
    makeTorus(innerRadius, outerRadius, innerSegments, outerSegments, points);

    for (int i = 0; i < innerSegments; i++) {
        int p = (i + innerSegments - 1) % innerSegments;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j < outerSegments; j++) {
            int pa = p * outerSegments + j;
            int ia = i * outerSegments + j;
            glVertex3f(points[pa].x, points[pa].y, points[pa].z);
            glVertex3f(points[ia].x, points[ia].y, points[ia].z);
        }
        
        int pa = p * outerSegments;
        int ia = i * outerSegments;
        glVertex3f(points[pa].x, points[pa].y, points[pa].z);
        glVertex3f(points[ia].x, points[ia].y, points[ia].z);
        glEnd();
    }
    
    /*
    // XY plane
    glColor4f(0, 1, 0, 0.2f);
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < segments; i++)
        glVertex3f(points[i].x, points[i].y, 0);
    glEnd();

    glColor4f(0, 1, 0, 1);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++)
        glVertex3f(points[i].x, points[i].y, 0);
    glEnd();
    
    if (vAxis == A_Y) {
        // XZ plane
        glColor4f(0, 1, 0, 0.2f);
        glBegin(GL_TRIANGLE_FAN);
        for (int i = 0; i < segments; i++)
            glVertex3f(points[i].x, 0, points[i].y);
        glEnd();

        glColor4f(0, 1, 0, 1);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; i++)
            glVertex3f(points[i].x, 0, points[i].y);
        glEnd();
    } else {
        // YZ plane
        glColor4f(0, 1, 0, 0.2f);
        glBegin(GL_TRIANGLE_FAN);
        for (int i = 0; i < segments; i++)
            glVertex3f(0, points[i].x, points[i].y);
        glEnd();

        glColor4f(0, 1, 0, 1);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; i++)
            glVertex3f(0, points[i].x, points[i].y);
        glEnd();
    }
    
    if (drag) {
        glColor4f(0, 1, 0, 1);
        glScalef(radius, radius, radius);
        glBegin(GL_LINES);
        if (vAxis == A_Y) {
            glVertex3f(0, 0, 0);
            glVertex3f(1, 0, 0);
            glVertex3f(0, 0, 0);
            glVertex3f(cos(hAngle), sin(hAngle), 0);
            glVertex3f(0, 0, 0);
            glVertex3f(cos(vAngle), 0, sin(vAngle));
        } else {
            glVertex3f(0, 0, 0);
            glVertex3f(0, 1, 0);
            glVertex3f(0, 0, 0);
            glVertex3f(cos(hAngle), sin(hAngle), 0);
            glVertex3f(0, 0, 0);
            glVertex3f(0, cos(vAngle), sin(vAngle));
        }
        glEnd();
    }
    */
    glPopMatrix();
}

- (void)updateCenter:(TVector3f *)theCenter radius:(float)theRadius verticalAxis:(EAxis)theVerticalAxis {
    NSAssert(vAxis != A_Z, @"vertical axis must not be the Z axis");
    
    center = *theCenter;
    radius = theRadius;
    vAxis = theVerticalAxis;
}

- (void)setDragging:(BOOL)isDragging {
    drag = isDragging;
}

- (void)updateHorizontalAngle:(float)theHAngle verticalAngle:(float)theVAngle {
    hAngle = theHAngle;
    vAngle = theVAngle;
}

@end
