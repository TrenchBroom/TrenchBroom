//
//  SelectionLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "SelectionLayer.h"

@implementation SelectionLayer

- (void)preRenderEdges {
    glColor4f(1, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
}

- (void)postRenderEdges {
    glEnable(GL_DEPTH_TEST);
}

@end
