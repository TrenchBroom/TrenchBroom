//
//  VBOMemBlock.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "VBOMemBlock.h"


@implementation VBOMemBlock

- (id)init {
    if (self = [super init]) {
        [self setFree:YES];
    }
    
    return self;
}

- (id)initWithBlockCapacity:(int)aSize {
    if (self = [self init]) {
        [self setCapacity:aSize];
    }
    
    return self;
}

- (int)capacity {
    return capacity;
}

- (BOOL)free {
    return free;
}

- (void)setFree:(BOOL)value {
    free = value;
}

- (void)setCapacity:(int)aSize {
    capacity = aSize;
}

- (VBOMemBlock *)previous {
    return previous;
}

- (VBOMemBlock *)next {
    return next;
}

- (void)setPrevious:(VBOMemBlock *)memBlock {
    previous = memBlock;
}

- (void)setNext:(VBOMemBlock *)memBlock {
    [next release];
    next = [memBlock retain];
}

- (void)dealloc {
    [next release];
    [super dealloc];
}

@end
