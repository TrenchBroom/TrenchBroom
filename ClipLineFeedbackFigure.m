//
//  ClipLineFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ClipLineFeedbackFigure.h"

@implementation ClipLineFeedbackFigure

- (id)initWithStartPoint:(TVector3i *)theStartPoint endPoint:(TVector3i *)theEndPoint {
    if (self = [self init]) {
        startPoint = *theStartPoint;
        endPoint = *theEndPoint;
    }
    
    return self;
}

- (void)render {
    glColor4f(0, 1, 0, 1);
    glBegin(GL_LINES);
    glVertex3f(startPoint.x, startPoint.y, startPoint.z);
    glVertex3f(endPoint.x, endPoint.y, endPoint.z);
    glEnd();
}

@end
