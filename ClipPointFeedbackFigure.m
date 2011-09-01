//
//  ClipPointFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ClipPointFeedbackFigure.h"

@implementation ClipPointFeedbackFigure

- (id)initWithPoint:(TVector3i *)thePoint {
    if (self = [self init]) {
        point = *thePoint;
        sphere = NULL;
    }
    
    return self;
}

- (void)render {
    if (sphere == NULL) {
        sphere = gluNewQuadric();
        gluQuadricDrawStyle(sphere, GLU_FILL);
    }
    
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT, GL_FILL);
    
    glPushMatrix();
    glTranslatef(point.x, point.y, point.z);
    glColor4f(0, 1, 0, 1);
    gluSphere(sphere, 3, 12, 12);
    glPopMatrix();
}

- (void)dealloc {
    if (sphere != NULL)
        gluDeleteQuadric(sphere);
    [super dealloc];
}
@end
