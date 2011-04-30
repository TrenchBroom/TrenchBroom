//
//  ClipPlaneFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ClipPlaneFeedbackFigure.h"

@implementation ClipPlaneFeedbackFigure

- (id)initWithPoint1:(TVector3i *)thePoint1 point2:(TVector3i *)thePoint2 point3:(TVector3i *)thePoint3 {
    if (self = [self init]) {
        point1 = *thePoint1;
        point2 = *thePoint2;
        point3 = *thePoint3;
    }
    
    return self;
}

- (void)render {
    glColor4f(0, 1, 0, 0.1f);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    glVertex3f(point1.x, point1.y, point1.z);
    glVertex3f(point2.x, point2.y, point2.z);
    glVertex3f(point3.x, point3.y, point3.z);
    glVertex3f(point3.x, point3.y, point3.z);
    glVertex3f(point2.x, point2.y, point2.z);
    glVertex3f(point1.x, point1.y, point1.z);
    glEnd();
    glEnable(GL_CULL_FACE);
}

@end
