/*
This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "VBOMemBlock.h"
#import "VBOBuffer.h"

@implementation VBOMemBlock

- (id)init {
    if (self = [super init]) {
        state = BS_FREE;
    }
    
    return self;
}

- (id)initBlockIn:(VBOBuffer *)theVboBuffer at:(int)theAddress capacity:(int)theCapacity {
    if (self = [self init]) {
        vboBuffer = theVboBuffer;
        address = theAddress;
        capacity = theCapacity;
    }
    
    return self;
}

- (int)address {
    return address;
}

- (void)setAddress:(int)theAddress {
    address = theAddress;
}

- (int)capacity {
    return capacity;
}

- (EVBOMemBlockState)state {
    return state;
}

- (VBOBuffer *)vbo {
    return vboBuffer;
}

- (void)setCapacity:(int)aSize {
    capacity = aSize;
}

- (void)setState:(EVBOMemBlockState)theState {
    state = theState;
}

- (void)activate {
    [vboBuffer activate];
}

- (void)deactivate {
    [vboBuffer deactivate];
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
    next = memBlock;
}

- (void)insertBetweenPrevious:(VBOMemBlock *)previousBlock next:(VBOMemBlock *)nextBlock {
    if ((previousBlock != nil && [previousBlock next] != nextBlock) || (nextBlock != nil && [nextBlock previous] != previousBlock))
        [NSException raise:NSInvalidArgumentException format:@"cannot insert between unchained blocks"];
    
    [previousBlock setNext:self];
    previous = previousBlock;
    [nextBlock setPrevious:self];
    next = nextBlock;
}

- (void)remove {
    [previous setNext:next];
    [next setPrevious:previous];
    
    previous = nil;
    next = nil;
}

- (void)free {
    [vboBuffer freeMemBlock:self];
}

- (id)retain {
    return [super retain];
}

- (oneway void)release {
    [super release];
}

@end
