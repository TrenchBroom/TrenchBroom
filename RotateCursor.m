//
//  RotateCursor.m
//  TrenchBroom
//
//  Created by Kristian Duske on 04.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "RotateCursor.h"

@implementation RotateCursor

- (void)render {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor4f(1, 1, 0, 1);
    // glDisable(GL_CULL_FACE);
    // glEnable(GL_DEPTH_TEST);
    
    glRotatef(hAngle * 360 / (2 * M_PI) + 270, 0, 0, 1);
    glRotatef(vAngle * 360 / (2 * M_PI), 0, 1, 0);
    
    float length = 40 / radius;
    
    float innerRadius = radius;
    float outerRadius = 2;
    
    int innerSegments = radius / 16;
    if (innerSegments < 12)
        innerSegments = 12;
    int outerSegments = 12;
    
    TVector3f torus[(innerSegments + 1) * outerSegments];
    makeTorusPart(innerRadius, outerRadius, innerSegments, outerSegments, 0, length, torus);
    
    int circleSegments = radius / 2;
    if (circleSegments < 12)
        circleSegments = 12;
    TVector3f circle[circleSegments];
    makeCircle(radius, circleSegments, circle);
    
    int coneSegments = 12;
    TVector3f cone[coneSegments + 2];
    makeCone(4, 10, coneSegments, cone);
    
    int capSegments = coneSegments;
    TVector3f cap[capSegments];
    makeCircle(4, capSegments, cap);
    
    glPushMatrix();
    glRotatef(-length * 90 / M_PI + 90, 0, 0, 1);
    glTranslatef(0, -radius, 0);
    glRotatef(-90, 0, 1, 0);
    
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < coneSegments + 2; i++)
        glVertex3f(cone[i].x, cone[i].y, cone[i].z);
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    for (int i = capSegments - 1; i >= 0; i--)
        glVertex3f(cap[i].x, cap[i].y, cap[i].z);
    glEnd();
    glPopMatrix();
    
    glPushMatrix();
    glRotatef(length * 90 / M_PI + 90, 0, 0, 1);
    glTranslatef(0, -radius, 0);
    glRotatef(90, 0, 1, 0);
    
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < coneSegments + 2; i++)
        glVertex3f(cone[i].x, cone[i].y, cone[i].z);
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    for (int i = capSegments - 1; i >= 0; i--)
        glVertex3f(cap[i].x, cap[i].y, cap[i].z);
    glEnd();
    glPopMatrix();
    
    
    glPushMatrix();
    glRotatef(90, 0, 0, 1);
    glRotatef(90, 0, 1, 0);
    glRotatef(-length * 90 / M_PI, 0, 0, 1);
    glTranslatef(0, -radius, 0);
    glRotatef(-90, 0, 1, 0);
    
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < coneSegments + 2; i++)
        glVertex3f(cone[i].x, cone[i].y, cone[i].z);
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    for (int i = capSegments - 1; i >= 0; i--)
        glVertex3f(cap[i].x, cap[i].y, cap[i].z);
    glEnd();
    glPopMatrix();
    
    glPushMatrix();
    glRotatef(90, 0, 0, 1);
    glRotatef(90, 0, 1, 0);
    glRotatef(length * 90 / M_PI, 0, 0, 1);
    glTranslatef(0, -radius, 0);
    glRotatef(90, 0, 1, 0);
    
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < coneSegments + 2; i++)
        glVertex3f(cone[i].x, cone[i].y, cone[i].z);
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    for (int i = capSegments - 1; i >= 0; i--)
        glVertex3f(cap[i].x, cap[i].y, cap[i].z);
    glEnd();
    glPopMatrix();
    
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(radius, 0, 0);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < circleSegments; i++)
        glVertex3f(circle[i].x, circle[i].y, circle[i].z);
    glEnd();
    
    for (int i = 0; i < innerSegments; i++) {
        int n = i + 1;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j < outerSegments; j++) {
            int ia = i * outerSegments + j;
            int na = n * outerSegments + j;
            glVertex3f(torus[ia].x, torus[ia].y, torus[ia].z);
            glVertex3f(torus[na].x, torus[na].y, torus[na].z);
        }
        
        int ia = i * outerSegments;
        int na = n * outerSegments;
        glVertex3f(torus[ia].x, torus[ia].y, torus[ia].z);
        glVertex3f(torus[na].x, torus[na].y, torus[na].z);
        glEnd();
    }
    
    glRotatef(90, 1, 0, 0);
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < circleSegments; i++)
        glVertex3f(circle[i].x, circle[i].y, circle[i].z);
    glEnd();
    
    for (int i = 0; i < innerSegments; i++) {
        int n = i + 1;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j < outerSegments; j++) {
            int ia = i * outerSegments + j;
            int na = n * outerSegments + j;
            glVertex3f(torus[ia].x, torus[ia].y, torus[ia].z);
            glVertex3f(torus[na].x, torus[na].y, torus[na].z);
        }
        
        int ia = i * outerSegments;
        int na = n * outerSegments;
        glVertex3f(torus[ia].x, torus[ia].y, torus[ia].z);
        glVertex3f(torus[na].x, torus[na].y, torus[na].z);
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
    radius = fmax(30, theRadius);
    vAxis = theVerticalAxis;
}

- (void)setDragging:(BOOL)isDragging {
    drag = isDragging;
}

- (void)updateHorizontalAngle:(float)theHAngle verticalAngle:(float)theVAngle {
    hAngle = theHAngle;
    vAngle = theVAngle;
}
- (void)update:(TVector3f *)thePosition {
}

@end
