//
//  VBOMemBlock.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "VBOMemBlock.h"
#import "VBOBuffer.h"
#import "Vector3f.h"
#import "Vector2f.h"

@implementation VBOMemBlock

- (id)init {
    if (self = [super init]) {
        state = BS_FREE;
    }
    
    return self;
}

- (id)initBlockIn:(VBOBuffer *)theVboBuffer at:(int)theAddress capacity:(int)theCapacity {
    if (theVboBuffer == nil)
        [NSException raise:NSInvalidArgumentException format:@"VBO buffer must not be nil"];
    
    if (self = [self init]) {
        vboBuffer = [theVboBuffer retain];
        address = theAddress;
        capacity = theCapacity;
    }
    
    return self;
}

- (int)address {
    return address;
}

- (int)capacity {
    return capacity;
}

- (EVBOMemBlockState)state {
    return state;
}

- (void)setCapacity:(int)aSize {
    capacity = aSize;
}

- (void)setState:(EVBOMemBlockState)theState {
    state = theState;
}

- (int)writeVector3f:(Vector3f *)theVector offset:(int)theOffset {
    [vboBuffer writeVector3f:theVector address:address + theOffset];
    return theOffset + 3 * sizeof(float);
}

- (int)writeVector2f:(Vector2f *)theVector offset:(int)theOffset {
    [vboBuffer writeVector2f:theVector address:address + theOffset];
    return theOffset + 2 * sizeof(float);
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
    [vboBuffer release];
    [next release];
    [super dealloc];
}

@end
