//
//  FeedbackLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "FeedbackLayer.h"
#import "EdgeRenderer.h"

@implementation FeedbackLayer

- (void)renderEdges {
    glColor4f(0, 0, 1, 1);
    glDisable(GL_DEPTH_TEST);
    [edgeRenderer render];
    glEnable(GL_DEPTH_TEST);
}

@end
