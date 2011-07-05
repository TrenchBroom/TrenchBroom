//
//  RotationFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "RotateFeedbackFigure.h"

@implementation RotateFeedbackFigure

- (void)render {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);
    glEnable(GL_DEPTH_TEST);
    
    glRotatef((initialHAngle + hAngle) * 360 / (2 * M_PI), 0, 0, 1);
    glRotatef((initialVAngle + vAngle) * 360 / (2 * M_PI), 0, 1, 0);
    
    int circleSegments = 2 * radius;
    if (circleSegments < 12)
        circleSegments = 12;
    TVector3f circle[circleSegments];
    makeCircle(radius, circleSegments, circle);
    
    glDisable(GL_DEPTH_TEST);
    glColor4f(1, 1, 0, 0.3f);
    
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(radius, 0, 0);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < circleSegments; i++)
        glVertex3f(circle[i].x, circle[i].y, circle[i].z);
    glEnd();
    
    glRotatef(90, 1, 0, 0);
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < circleSegments; i++)
        glVertex3f(circle[i].x, circle[i].y, circle[i].z);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glColor4f(1, 1, 0, 1);

    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(radius, 0, 0);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < circleSegments; i++)
        glVertex3f(circle[i].x, circle[i].y, circle[i].z);
    glEnd();
    
    glRotatef(90, 1, 0, 0);
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < circleSegments; i++)
        glVertex3f(circle[i].x, circle[i].y, circle[i].z);
    glEnd();

    
    glPopMatrix();
}

- (void)updateCenter:(TVector3f *)theCenter radius:(float)theRadius verticalAxis:(EAxis)theVerticalAxis initialHAngle:(float)theInitialHAngle initialVAngle:(float)theInitialVAngle {
    NSAssert(vAxis != A_Z, @"vertical axis must not be the Z axis");
    
    center = *theCenter;
    radius = fmax(30, theRadius);
    vAxis = theVerticalAxis;
    initialHAngle = theInitialHAngle;
    initialVAngle = theInitialVAngle;
}

- (void)setDragging:(BOOL)isDragging {
    drag = isDragging;
}

- (void)updateHorizontalAngle:(float)theHAngle verticalAngle:(float)theVAngle {
    hAngle = theHAngle;
    vAngle = theVAngle;
}

@end
