//
//  EdgeFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 22.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EdgeFigure.h"
#import "Edge.h"
#import "Vertex.h"
#import "Vector3f.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"

@implementation EdgeFigure

- (id)initWithEdge:(Edge *)theEdge {
    NSAssert(theEdge != nil, @"edge must not be nil");

    if (self = [self init]) {
        edge = [theEdge retain];
    }
    
    return self;
}

- (void)dealloc {
    [block free];
    [block release];
    block = nil;
    [edge release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Figure

- (void)updateVBO:(VBOBuffer *)theVbo {
    [block free];
    [block release];
    block = [theVbo allocMemBlock:6 * siizeof(float)];

    int offset = 0;
    offset = [block writeVector3f:[[edge startVertex] vector] offset:offset];
    offset = [block writeVector3f:[[edge endVertex] vector] offset:offset];
    [block setState:BS_USED_VALID];
}

@end
