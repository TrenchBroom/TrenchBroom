//
//  RotationFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "RotationFeedbackFigure.h"

@implementation RotationFeedbackFigure

- (void)dealloc {
    if (initialized)
        gluDeleteQuadric(sphere);
    [super dealloc];
}

- (void)render {
    if (!initialized) {
        sphere = gluNewQuadric();
        gluQuadricDrawStyle(sphere, GLU_FILL);
        gluQuadricNormals(sphere, GLU_SMOOTH);
        initialized = YES;
    }
    
    int segments = radius * 15;
    TVector3f points[segments];
    makeCircle(radius, segments, points);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);

    // XY plane
    glColor4f(0, 0, 1, 1);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++)
        glVertex3f(points[i].x, points[i].y, 0);
    glEnd();
    
    // XZ plane
    glColor4f(0, 1, 0, 1);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++)
        glVertex3f(points[i].x, 0, points[i].y);
    glEnd();

    // YZ plane
    glColor4f(1, 0, 0, 1);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++)
        glVertex3f(0, points[i].x, points[i].y);
    glEnd();
    
    glPolygonMode(GL_FRONT, GL_FILL);
    glColor4f(0, 0, 1, 0.2f);
    gluSphere(sphere, radius, radius * 1.2f, radius * 1.2f);
    
    if (drag) {
        glColor4f(1, 1, 0, 1);
        glScalef(radius, radius, radius);
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(initialDragVector.x, initialDragVector.y, initialDragVector.z);
        glVertex3f(0, 0, 0);
        glVertex3f(currentDragVector.x, currentDragVector.y, currentDragVector.z);
        glEnd();
    }
    
    glPopMatrix();
}

- (void)updateCenter:(TVector3f *)theCenter radius:(float)theRadius {
    center = *theCenter;
    radius = theRadius;
}

- (void)setDragging:(BOOL)isDragging {
    drag = isDragging;
}

- (void)updateInitialDragVector:(TVector3f *)theVector {
    initialDragVector = *theVector;
}

- (void)updateCurrentDragVector:(TVector3f *)theVector {
    currentDragVector = *theVector;
}

@end
