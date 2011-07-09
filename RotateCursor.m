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
    
    glDisable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT, GL_FILL);
    
    glRotatef((initialHAngle + hAngle) * 360 / (2 * M_PI), 0, 0, 1);
    glRotatef((initialVAngle + vAngle) * 360 / (2 * M_PI), 0, 1, 0);
    
    float length = 40 / radius;
    
    float innerRadius = radius;
    float outerRadius = 2;
    
    int innerSegments = radius / 16;
    if (innerSegments < 12)
        innerSegments = 12;
    int outerSegments = 12;
    
    TVector3f torus[(innerSegments + 1) * outerSegments];
    TVector3f torusNormals[(innerSegments + 1) * outerSegments];
    makeTorusPart(innerRadius, outerRadius, innerSegments, outerSegments, 0, length, torus, torusNormals);
    
    int coneSegments = 12;
    TVector3f cone[coneSegments + 2];
    TVector3f coneNormals[coneSegments + 2];
    makeCone(4, 10, coneSegments, cone, coneNormals);
    
    int capSegments = coneSegments;
    TVector3f cap[capSegments];
    makeCircle(4, capSegments, cap);
    
    glPushMatrix();
    glRotatef(-length * 90 / M_PI + 90, 0, 0, 1);
    glTranslatef(0, -radius, 0);
    glRotatef(-90, 0, 1, 0);
    
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < coneSegments + 2; i++) {
        glNormal3f(coneNormals[i].x, coneNormals[i].y, coneNormals[i].z);
        glVertex3f(cone[i].x, cone[i].y, cone[i].z);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(ZAxisNeg.x, ZAxisNeg.y, ZAxisNeg.z);
    for (int i = capSegments - 1; i >= 0; i--)
        glVertex3f(cap[i].x, cap[i].y, cap[i].z);
    glEnd();
    glPopMatrix();
    
    glPushMatrix();
    glRotatef(length * 90 / M_PI + 90, 0, 0, 1);
    glTranslatef(0, -radius, 0);
    glRotatef(90, 0, 1, 0);
    
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < coneSegments + 2; i++) {
        glNormal3f(coneNormals[i].x, coneNormals[i].y, coneNormals[i].z);
        glVertex3f(cone[i].x, cone[i].y, cone[i].z);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(ZAxisNeg.x, ZAxisNeg.y, ZAxisNeg.z);
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
    for (int i = 0; i < coneSegments + 2; i++) {
        glNormal3f(coneNormals[i].x, coneNormals[i].y, coneNormals[i].z);
        glVertex3f(cone[i].x, cone[i].y, cone[i].z);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(ZAxisNeg.x, ZAxisNeg.y, ZAxisNeg.z);
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
    for (int i = 0; i < coneSegments + 2; i++) {
        glNormal3f(coneNormals[i].x, coneNormals[i].y, coneNormals[i].z);
        glVertex3f(cone[i].x, cone[i].y, cone[i].z);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(ZAxisNeg.x, ZAxisNeg.y, ZAxisNeg.z);
    for (int i = capSegments - 1; i >= 0; i--)
        glVertex3f(cap[i].x, cap[i].y, cap[i].z);
    glEnd();
    glPopMatrix();
    
    for (int i = 0; i < innerSegments; i++) {
        int n = i + 1;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j < outerSegments; j++) {
            int ia = i * outerSegments + j;
            int na = n * outerSegments + j;

            glNormal3f(torusNormals[ia].x, torusNormals[ia].y, torusNormals[ia].z);
            glVertex3f(torus[ia].x, torus[ia].y, torus[ia].z);

            glNormal3f(torusNormals[na].x, torusNormals[na].y, torusNormals[na].z);
            glVertex3f(torus[na].x, torus[na].y, torus[na].z);
        }
        
        int ia = i * outerSegments;
        int na = n * outerSegments;

        glNormal3f(torusNormals[ia].x, torusNormals[ia].y, torusNormals[ia].z);
        glVertex3f(torus[ia].x, torus[ia].y, torus[ia].z);
        
        glNormal3f(torusNormals[na].x, torusNormals[na].y, torusNormals[na].z);
        glVertex3f(torus[na].x, torus[na].y, torus[na].z);
        
        glEnd();
    }
    
    glRotatef(90, 1, 0, 0);
    
    for (int i = 0; i < innerSegments; i++) {
        int n = i + 1;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j < outerSegments; j++) {
            int ia = i * outerSegments + j;
            int na = n * outerSegments + j;

            glNormal3f(torusNormals[ia].x, torusNormals[ia].y, torusNormals[ia].z);
            glVertex3f(torus[ia].x, torus[ia].y, torus[ia].z);
            
            glNormal3f(torusNormals[na].x, torusNormals[na].y, torusNormals[na].z);
            glVertex3f(torus[na].x, torus[na].y, torus[na].z);
        }
        
        int ia = i * outerSegments;
        int na = n * outerSegments;

        glNormal3f(torusNormals[ia].x, torusNormals[ia].y, torusNormals[ia].z);
        glVertex3f(torus[ia].x, torus[ia].y, torus[ia].z);
        
        glNormal3f(torusNormals[na].x, torusNormals[na].y, torusNormals[na].z);
        glVertex3f(torus[na].x, torus[na].y, torus[na].z);

        glEnd();
    }
    
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

- (void)update:(TVector3f *)thePosition {
}

@end
