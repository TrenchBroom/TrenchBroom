//
//  ClipLineFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ClipLineFeedbackFigure.h"
#import "Vector3i.h"

@implementation ClipLineFeedbackFigure

- (id)initWithStartPoint:(Vector3i *)theStartPoint endPoint:(Vector3i *)theEndPoint {
    NSAssert(theStartPoint != nil, @"start point must not be nil");
    NSAssert(theEndPoint != nil, @"end point must not be nil");
    
    if (self = [self init]) {
        startPoint = [theStartPoint retain];
        endPoint = [theEndPoint retain];
    }
    
    return self;
}

- (void)render {
    glColor4f(0, 1, 0, 1);
    glBegin(GL_LINES);
    glVertex3f([startPoint x], [startPoint y], [startPoint z]);
    glVertex3f([endPoint x], [endPoint y], [endPoint z]);
    glEnd();
}

- (void)dealloc {
    [startPoint release];
    [endPoint release];
    [super dealloc];
}

@end
