//
//  BoundsFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BoundsFeedbackFigure.h"
#import "GLUtils.h"

@interface BoundsFeedbackFigure (private)

- (void)doRender;

@end

@implementation BoundsFeedbackFigure (private)

- (void)doRender {
    glBegin(GL_LINES);
    glVertex3f(bounds.min.x, bounds.min.y, bounds.min.z);
    glVertex3f(bounds.min.x, bounds.min.y, bounds.max.z);
    glVertex3f(bounds.min.x, bounds.min.y, bounds.min.z);
    glVertex3f(bounds.min.x, bounds.max.y, bounds.min.z);
    glVertex3f(bounds.min.x, bounds.min.y, bounds.min.z);
    glVertex3f(bounds.max.x, bounds.min.y, bounds.min.z);
    
    glVertex3f(bounds.max.x, bounds.max.y, bounds.max.z);
    glVertex3f(bounds.max.x, bounds.max.y, bounds.min.z);
    glVertex3f(bounds.max.x, bounds.max.y, bounds.max.z);
    glVertex3f(bounds.max.x, bounds.min.y, bounds.max.z);
    glVertex3f(bounds.max.x, bounds.max.y, bounds.max.z);
    glVertex3f(bounds.min.x, bounds.max.y, bounds.max.z);
    
    glVertex3f(bounds.min.x, bounds.max.y, bounds.min.z);
    glVertex3f(bounds.min.x, bounds.max.y, bounds.max.z);
    glVertex3f(bounds.min.x, bounds.max.y, bounds.min.z);
    glVertex3f(bounds.max.x, bounds.max.y, bounds.min.z);
    
    glVertex3f(bounds.max.x, bounds.min.y, bounds.min.z);
    glVertex3f(bounds.max.x, bounds.max.y, bounds.min.z);
    glVertex3f(bounds.max.x, bounds.min.y, bounds.min.z);
    glVertex3f(bounds.max.x, bounds.min.y, bounds.max.z);
    
    glVertex3f(bounds.min.x, bounds.min.y, bounds.max.z);
    glVertex3f(bounds.max.x, bounds.min.y, bounds.max.z);
    glVertex3f(bounds.min.x, bounds.min.y, bounds.max.z);
    glVertex3f(bounds.min.x, bounds.max.y, bounds.max.z);
    glEnd();
}

@end

@implementation BoundsFeedbackFigure

- (void)setBounds:(TBoundingBox *)theBounds {
    bounds = *theBounds;
}

- (void)render {
    glDisable(GL_DEPTH_TEST);
    glColor4f(0, 1, 0, 0.5f);
    [self doRender];
    
    glEnable(GL_DEPTH_TEST);
    glColor4f(0, 1, 0, 1);
    [self doRender];
}

@end
