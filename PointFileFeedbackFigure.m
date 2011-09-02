//
//  PointFileFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.09.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "PointFileFeedbackFigure.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "GLUtils.h"

@implementation PointFileFeedbackFigure

- (id)initWithPoints:(TVector3f *)thePoints pointCount:(int)thePointCount {
    NSAssert(thePoints != NULL, @"point list must not be NULL");
    NSAssert(thePointCount > 0, @"point count must be greater than 0");
    
    if ((self = [self init])) {
        pointCount = thePointCount;
        points = malloc(pointCount * sizeof(TVector3f));
        memcpy(points, thePoints, pointCount * sizeof(TVector3f));
    }
    
    return self;
}

- (void)dealloc {
    if (points != NULL)
        free(points);
    [super dealloc];
}

- (void)render {
    glDisable(GL_DEPTH_TEST);
    glColor4f(1, 1, 0, 0.3f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < pointCount; i++)
        glVertexV3f(&points[i]);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    
    glColor4f(1, 1, 0, 1);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < pointCount; i++)
        glVertexV3f(&points[i]);
    glEnd();
}

@end
