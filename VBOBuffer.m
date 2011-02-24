//
//  VBOManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "Vector2f.h"
#import "Vector3f.h"

NSString* const BufferNotMappedException = @"BufferNotMappedException";

CFComparisonResult compareMemBlocks(const void *val1, const void *val2, void *context) {
    VBOMemBlock *block1 = (VBOMemBlock *)val1;
    VBOMemBlock *block2 = (VBOMemBlock *)val2;
    
    if ([block1 capacity] < [block2 capacity])
        return kCFCompareLessThan;
    
    if ([block1 capacity] > [block2 capacity])
        return kCFCompareGreaterThan;
    
    return kCFCompareEqualTo;
}

@implementation VBOBuffer

- (id)init {
    if (self = [super init]) {
        freeBlocksByCapacity = [[NSMutableArray alloc] initWithCapacity:10];
    }
         
    return self;
}

- (id)initWithTotalCapacity:(int)capacity {
    if (self = [self init]) {
        totalCapacity = capacity;
        freeCapacity = capacity;
        firstBlock = [[VBOMemBlock alloc] initBlockIn:self at:0 capacity:capacity];
        [freeBlocksByCapacity addObject:firstBlock];
    }
    
    return self;
}

- (int)totalCapacity {
    return totalCapacity;
}

- (int)freeCapacity {
    return freeCapacity;
}

- (unsigned)findMemBlock:(VBOMemBlock *)query {
    return (unsigned)CFArrayBSearchValues((CFArrayRef)freeBlocksByCapacity, 
                                          CFRangeMake(0, CFArrayGetCount((CFArrayRef)freeBlocksByCapacity)),
                                          query,
                                          (CFComparatorFunction)compareMemBlocks,
                                          NULL);
}

- (void)insertFreeMemBlock:(VBOMemBlock *)memBlock {
    if ([memBlock state] != BS_FREE)
        return;
    
    unsigned index = [self findMemBlock:memBlock];
    if (index >= [freeBlocksByCapacity count])
        [freeBlocksByCapacity addObject:memBlock];
    else
        [freeBlocksByCapacity insertObject:memBlock atIndex:index];
}

- (void)removeFreeMemBlock:(VBOMemBlock *)memBlock {
    if ([memBlock state] != BS_FREE)
        return;
    
    unsigned index = [self findMemBlock:memBlock];
    VBOMemBlock* candidate = [freeBlocksByCapacity objectAtIndex:index];
    int i = index;
    
    while (i > 0 && candidate != memBlock 
           && [candidate state] == BS_FREE 
           && [candidate capacity] == [memBlock capacity])
        candidate = [freeBlocksByCapacity objectAtIndex:--i];
    
    if (candidate != memBlock) {
        i = index + 1;
        candidate = [freeBlocksByCapacity objectAtIndex:i];
        while (i < [freeBlocksByCapacity count] - 1 && candidate != memBlock 
               && [candidate state] == BS_FREE 
               && [candidate capacity] == [memBlock capacity])
            candidate = [freeBlocksByCapacity objectAtIndex:++i];
    }
    
    if (candidate != memBlock)
        [NSException raise:NSInvalidArgumentException format:@"the given memory block could not be found"];
    
    [freeBlocksByCapacity removeObjectAtIndex:i];
}

- (void)activate {
    if (vboId == 0) {
        glGenBuffers(1, &vboId);
        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glEnableClientState(GL_VERTEX_ARRAY);
        glBufferData(GL_ARRAY_BUFFER, totalCapacity, NULL, GL_DYNAMIC_DRAW);
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glEnableClientState(GL_VERTEX_ARRAY);
    }
}

- (void)deactivate {
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

- (void)mapBuffer {
    buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

- (void)unmapBuffer {
    if (buffer != NULL) {
        glUnmapBuffer(GL_ARRAY_BUFFER);
        buffer = NULL;
    }
}

- (void)writeFloat:(float)f address:(int)theAddress {
    if (buffer == NULL)
        [NSException raise:BufferNotMappedException format:@"cannot write to unmapped buffer"];
    
    for (int i = 0; i < 4; i++)
        buffer[theAddress + i] = ((char *)&f)[i];
}

- (void)writeVector3f:(Vector3f *)theVector address:(int)theAddress {
    if (theVector == nil)
        [NSException raise:NSInvalidArgumentException format:@"vector must not be nil"];
    
    [self writeFloat:[theVector x] address:theAddress];
    [self writeFloat:[theVector y] address:theAddress + sizeof(float)];
    [self writeFloat:[theVector z] address:theAddress + 2 * sizeof(float)];
}

- (void)writeVector2f:(Vector2f *)theVector address:(int)theAddress {
    if (theVector == nil)
        [NSException raise:NSInvalidArgumentException format:@"vector must not be nil"];
    
    [self writeFloat:[theVector x] address:theAddress];
    [self writeFloat:[theVector y] address:theAddress + sizeof(float)];
}


- (void)resizeMemBlock:(VBOMemBlock *)memBlock toCapacity:(int)capacity {
    if (capacity == [memBlock capacity])
        return;

    if ([memBlock state] != BS_FREE)
        return;
    
    [self removeFreeMemBlock:memBlock];
    [memBlock setCapacity:capacity];
    [self insertFreeMemBlock:memBlock];
}

- (void)resizeBufferTo:(int)newCapacity {
    int addedCapacity = newCapacity - totalCapacity;
    freeCapacity = newCapacity - (totalCapacity - freeCapacity);
    totalCapacity = newCapacity;
    
    VBOMemBlock* block = firstBlock;
    VBOMemBlock* lastBlock;
    while (block != nil) {
        if ([block state] == BS_USED_VALID)
            [block setState:BS_USED_INVALID];
        lastBlock = block;
        block = [block next];
    }
    
    if ([lastBlock state] == BS_FREE) {
        [self resizeMemBlock:lastBlock toCapacity:[lastBlock capacity] + addedCapacity];
    } else {
        block = [[VBOMemBlock alloc] initBlockIn:self at:[lastBlock address] + [lastBlock capacity] capacity:addedCapacity];
        [lastBlock setNext:block];
        [block setPrevious:lastBlock];
        [self insertFreeMemBlock:block];
        [block release];
    }

    [self dispose];
}

- (VBOMemBlock *)allocMemBlock:(int)capacity {
    if (capacity > freeCapacity) {
        [self resizeBufferTo:2 * totalCapacity];
        return [self allocMemBlock:capacity];
    }
    
    VBOMemBlock* query = [[VBOMemBlock alloc]initBlockIn:self at:0 capacity:capacity];
    unsigned index = [self findMemBlock:query];
    [query release];
    
    // todo try to rearrange memory?
    if (index >= [freeBlocksByCapacity count]) {
        [self resizeBufferTo:2 * totalCapacity];
        return [self allocMemBlock:capacity];
    }
    
    VBOMemBlock* memBlock = [freeBlocksByCapacity objectAtIndex:index];
    [freeBlocksByCapacity removeObjectAtIndex:index];
    
    // split the memory block
    if (capacity < [memBlock capacity]) {
        VBOMemBlock *remainder = [[VBOMemBlock alloc] initBlockIn:self at:[memBlock address] + capacity capacity:[memBlock capacity] - capacity];
        [memBlock setCapacity:capacity];

        VBOMemBlock* next = [memBlock next];
        
        [remainder setNext:next];
        [remainder setPrevious:memBlock];
        
        [memBlock setNext:remainder];
        [next setPrevious:remainder];

        [self insertFreeMemBlock:remainder];
        [remainder release];
    }
    
    [memBlock setState:BS_USED_INVALID];
    freeCapacity -= [memBlock capacity];
    
    return memBlock;
}

- (void)freeMemBlock:(VBOMemBlock *)memBlock {
    if (memBlock == nil)
        [NSException raise:NSInvalidArgumentException format:@"block must not be nil"];
    
    VBOMemBlock* previous = [memBlock previous];
    VBOMemBlock* next = [memBlock next];

    freeCapacity += [memBlock capacity];
    
    if (previous != nil && [previous state] == BS_FREE &&  
        next != nil && [next state] == BS_FREE) {
        VBOMemBlock* nextNext = [next next];
        
        [previous setNext:nextNext];
        [nextNext setPrevious:previous];
        
        [memBlock setPrevious:nil];
        [memBlock setNext:nil];
        [next setPrevious:nil];
        [next setNext:nil];
        
        [self resizeMemBlock:previous toCapacity:[previous capacity] + [memBlock capacity] + [next capacity]];
        [self removeFreeMemBlock:next];

    } else if (previous != nil && [previous state] == BS_FREE) {
        [previous setNext:next];
        [next setPrevious:previous];
        
        [memBlock setPrevious:nil];
        [memBlock setNext:nil];

        [self resizeMemBlock:previous toCapacity:[previous capacity] + [memBlock capacity]];
    } else if (next != nil && [next state] == BS_FREE) {
        VBOMemBlock* nextNext = [next next];
        
        [memBlock setNext:nextNext];
        [nextNext setPrevious:memBlock];
        
        [next setPrevious:nil];
        [next setNext:nil];
        
        [memBlock setCapacity:[memBlock capacity] + [next capacity]];
        [memBlock setState:BS_FREE];
        [self insertFreeMemBlock:memBlock];
        [self removeFreeMemBlock:next];
    } else {
        [memBlock setState:BS_FREE];
        [self insertFreeMemBlock:memBlock];
    }
}

- (void)dealloc {
    [self unmapBuffer];
    [self deactivate];
    if (vboId != 0)
        glDeleteBuffers(1, &vboId);

    [freeBlocksByCapacity release];
    [firstBlock release];
    [super dealloc];
}

@end
