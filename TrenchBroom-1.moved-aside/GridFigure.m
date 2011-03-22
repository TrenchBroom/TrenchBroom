//
//  GridFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "GridFigure.h"
#import "Vector3f.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"

@implementation GridFigure

- (id)initWithPoint1:(Vector3f *)thePoint1 point2:(Vector3f *)thePoint2 {
    if (self = [self init]) {
        point1 = [thePoint1 retain];
        point2 = [thePoint2 retain];
    }
    
    return self;
}

- (void)prepareWithVbo:(VBOBuffer *)theVbo {
    if (block != nil && [block vbo] != theVbo)
        [self invalidate];
    
    if (block == nil)
        block = [[theVbo allocMemBlock:3 * sizeof(float) * 2] retain];
    
    if ([block state] == BS_USED_INVALID) {
        int offset = [block writeVector3f:point1 offset:0];
        [block writeVector3f:point2 offset:offset];
        [block setState:BS_USED_VALID];
    }
}

- (void)invalidate {
    [block free];
    [block release];
    block = nil;
}

- (void)dealloc {
    [self invalidate];
    [point1 release];
    [point2 release];
    [super dealloc];
}

@end
