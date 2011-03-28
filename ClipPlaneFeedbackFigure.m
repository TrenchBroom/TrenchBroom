//
//  ClipPlaneFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ClipPlaneFeedbackFigure.h"
#import "Vector3i.h"

@implementation ClipPlaneFeedbackFigure

- (id)initWithPoint1:(Vector3i *)thePoint1 point2:(Vector3i *)thePoint2 point3:(Vector3i *)thePoint3 {
    NSAssert(thePoint1 != nil, @"point 1 must not be nil");
    NSAssert(thePoint2 != nil, @"point 2 must not be nil");
    NSAssert(thePoint3 != nil, @"point 3 must not be nil");
    
    if (self = [self init]) {
        point1 = [thePoint1 retain];
        point2 = [thePoint2 retain];
        point3 = [thePoint3 retain];
    }
    
    return self;
}

- (void)render {
    glColor4f(0, 1, 0, 0.5f);
    glPolygonMode(GL_FRONT, GL_FILL);
    glBegin(GL_TRIANGLES);
    glVertex3f([point1 x], [point1 y], [point1 z]);
    glVertex3f([point2 x], [point2 y], [point2 z]);
    glVertex3f([point3 x], [point3 y], [point3 z]);
    glVertex3f([point3 x], [point3 y], [point3 z]);
    glVertex3f([point2 x], [point2 y], [point2 z]);
    glVertex3f([point1 x], [point1 y], [point1 z]);
    glEnd();
}

- (void)dealloc {
    [point1 release];
    [point2 release];
    [point3 release];
    [super dealloc];
}

@end
