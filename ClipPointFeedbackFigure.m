//
//  ClipPointFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ClipPointFeedbackFigure.h"
#import "Vector3i.h"
#import "Vector3f.h"

@implementation ClipPointFeedbackFigure

- (id)initWithPoint:(Vector3i *)thePoint {
    NSAssert(thePoint != nil, @"point must not be nil");
    if (self = [self init]) {
        point = [thePoint retain];
        sphere = gluNewQuadric();
        gluQuadricDrawStyle(sphere, GLU_FILL);
    }
    
    return self;
}

- (void)render {
    glFrontFace(GL_CCW);
    glPushMatrix();
    glTranslatef([point x], [point y], [point z]);
    glColor4f(0, 1, 0, 1);
    gluSphere(sphere, 3, 12, 12);
    glPopMatrix();
}

- (void)dealloc {
    [point release];
    gluDeleteQuadric(sphere);
    [super dealloc];
}
@end
