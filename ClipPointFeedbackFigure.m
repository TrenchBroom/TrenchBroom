//
//  ClipPointFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ClipPointFeedbackFigure.h"
#import "Camera.h"

@implementation ClipPointFeedbackFigure

- (id)initWithPoint:(const TVector3i *)thePoint camera:(Camera *)theCamera {
    if (self = [self init]) {
        setV3f(&point, thePoint);
        sphere = NULL;
        camera = theCamera;
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
    
    float dist = [camera distanceTo:&point];
    float radius = dist / 300 * 3;
    
    glPushMatrix();
    glTranslatef(point.x, point.y, point.z);
    glColor4f(0, 1, 0, 1);
    gluSphere(sphere, radius, 12, 12);
    glPopMatrix();
}

- (void)dealloc {
    if (sphere != NULL)
        gluDeleteQuadric(sphere);
    [super dealloc];
}
@end
