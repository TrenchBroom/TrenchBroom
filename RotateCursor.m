//
//  RotateCursor.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "RotateCursor.h"


@implementation RotateCursor

- (void)setPlaneNormal:(EAxis)thePlaneNormal {
    planeNormal = thePlaneNormal;
}

- (void)renderCursor {
    int steps = 20;
    float radius = 13;
    float thickness = 4;
    float ratio = 0.8f;

    TVector2f hand[(steps + 1) * 2];
    
    float innerRadius = radius - thickness / 2;
    float outerRadius = radius + thickness / 2;
    float length = 2 * M_PI * ratio;
    float da = length / steps;
    float a = 0;

    float s,c;
    for (int i = 0; i <= steps; i++) {
        s = sin(a);
        c = cos(a);
        
        hand[2 * i].x = s * innerRadius;
        hand[2 * i].y = c * innerRadius;
        hand[2 * i + 1].x = s * outerRadius;
        hand[2 * i + 1].y = c * outerRadius;
        
        a += da;
    }

    TVector2f tip[3];
    tip[0].x = s * (innerRadius - 2);
    tip[0].y = c * (innerRadius - 2);
    tip[1].x = s * (outerRadius + 2);
    tip[1].y = c * (outerRadius + 2);

    // l2 = 2r2 - 2r2 cos gamma
    // l2 = 2r2 * (1 - cos gamma)
    // (l2 / 2r2) = 1 - cos gamma
    // 1 - (l2 / 2r2) = cos gamma
    
    // c2 = a2 + b2 - 2ab cos gamma
    // c2 - a2 - b2 = -2ab cos gamma
    // a2 + b2 - c2 = 2ab cos gamma
    // cos gamma = (a2 + b2 - c2) / 2ab
    // cos gamma = (2 * r2 - l2) / 2r2
    
    a = a - da + acos(1 - (25 / (2 * radius * radius)));
    c = cos(a);
    s = sin(a);
    
    tip[2].x = s * radius;
    tip[2].y = c * radius;
    
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= steps; i++) {
        glVertex3f(hand[2 * i].x, hand[2 * i].y, 0);
        glVertex3f(hand[2 * i + 1].x, hand[2 * i + 1].y, 0);
    }
    glEnd();
    
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 3; i++)
        glVertex3f(tip[i].x, tip[i].y, 0);
    glEnd();
}

- (void)render {
    glDisable(GL_TEXTURE_2D);
    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glMatrixMode(GL_MODELVIEW);
    
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    
    switch (planeNormal) {
        case A_X:
            glRotatef(90, 0, 1, 0);
            [self renderCursor];
            break;
        case A_Y:
            glRotatef(90, 1, 0, 0);
            [self renderCursor];
            break;
        case A_Z:
            [self renderCursor];
            break;
    }
    
    glPopMatrix();
    
    glDepthFunc(GL_LESS);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
}

- (void)update:(TVector3f *)thePosition {
    position = *thePosition;
}

@end
