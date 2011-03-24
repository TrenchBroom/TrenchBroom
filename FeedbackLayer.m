//
//  FeedbackLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "FeedbackLayer.h"
#import "ThinEdgeRenderer.h"

@implementation FeedbackLayer

- (void)renderEdges {
    glColor4f(1, 1, 0, 1);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2);
    [edgeRenderer render];
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1);
}

- (void)renderVertices {
    glColor4f(1, 1, 0, 1);
    glDisable(GL_DEPTH_TEST);
    [vertexRenderer render];
    glEnable(GL_DEPTH_TEST);
}

- (void)renderGrid {
    glColor4f(1, 1, 0, 0.5f);
    [gridRenderer render];
}

@end
