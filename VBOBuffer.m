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

- (void)checkChain {
    VBOMemBlock* block = firstBlock;
    if ([block previous] != nil)
        [NSException raise:@"VBOBufferChainException" format:@"first block has previous block"];
    
    while ([block next] != nil) {
        block = [block next];
        VBOMemBlock* previous = [block previous];
        VBOMemBlock* next = [block next];
        if ([previous next] != block)
            [NSException raise:@"VBOBufferChainException" format:@"chain is invalid"];
        if (next != nil && [next previous] != block)
            [NSException raise:@"VBOBufferChainException" format:@"chain is invalid"];
    }
    
    if (block != lastBlock)
        [NSException raise:@"VBOBufferChainException" format:@"last block is not last in chain"];
}

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
        lastBlock = firstBlock;
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
    active = YES;
}

- (void)deactivate {
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    active = NO;
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

- (void)writeBuffer:(const void*)theBuffer address:(int)theAddress count:(int)theCount {
    if (buffer == NULL)
        [NSException raise:BufferNotMappedException format:@"cannot write to unmapped buffer"];
    
    memcpy(buffer + theAddress, theBuffer, theCount);
}

- (void)writeFloat:(float)f address:(int)theAddress {
    if (buffer == NULL)
        [NSException raise:BufferNotMappedException format:@"cannot write to unmapped buffer"];
    
    for (int i = 0; i < 4; i++)
        buffer[theAddress + i] = ((char *)&f)[i];
}

- (void)writeVector3f:(Vector3f *)theVector address:(int)theAddress {
    [self writeFloat:[theVector x] address:theAddress];
    [self writeFloat:[theVector y] address:theAddress + sizeof(float)];
    [self writeFloat:[theVector z] address:theAddress + 2 * sizeof(float)];
}

- (void)writeVector2f:(Vector2f *)theVector address:(int)theAddress {
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
    BOOL wasActive = active;
    BOOL wasMapped = buffer != NULL;

    uint8_t* temp = NULL;
    if (vboId != 0 && freeCapacity < totalCapacity) {
        if (!wasActive)
            [self activate];
        if (!wasMapped)
            [self mapBuffer];
        
        temp = malloc(totalCapacity);
        memcpy(temp, buffer, totalCapacity);
    }
    
    int addedCapacity = newCapacity - totalCapacity;
    freeCapacity = newCapacity - (totalCapacity - freeCapacity);
    totalCapacity = newCapacity;
    
    if ([lastBlock state] == BS_FREE) {
        [self resizeMemBlock:lastBlock toCapacity:[lastBlock capacity] + addedCapacity];
    } else {
        VBOMemBlock* block = [[VBOMemBlock alloc] initBlockIn:self at:[lastBlock address] + [lastBlock capacity] capacity:addedCapacity];
        [block insertBetweenPrevious:lastBlock next:nil];
        [self insertFreeMemBlock:block];
        lastBlock = block;
    }

    if (vboId != 0) {
        if (buffer != NULL)
            [self unmapBuffer];
        if (active)
            [self deactivate];
        glDeleteBuffers(1, &vboId);
        vboId = 0;
    }
    
    if (temp != NULL) {
        if (!active)
            [self activate];
        if (buffer == NULL)
            [self mapBuffer];
        memcpy(buffer, temp, totalCapacity - addedCapacity);
        free(temp);
        if (!wasMapped)
            [self unmapBuffer];
        if (!wasActive)
            [self deactivate];
    } else {
        if (wasActive && !active)
            [self activate];
        if (wasMapped && buffer == NULL)
            [self mapBuffer];
    }
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

        [remainder insertBetweenPrevious:memBlock next:[memBlock next]];
        [self insertFreeMemBlock:remainder];
        
        if (lastBlock == memBlock)
            lastBlock = remainder;
    }
    
    [memBlock setState:BS_USED_INVALID];
    freeCapacity -= [memBlock capacity];

    return memBlock;
}

- (VBOMemBlock *)freeMemBlock:(VBOMemBlock *)memBlock {
    VBOMemBlock* previous = [memBlock previous];
    VBOMemBlock* next = [memBlock next];

    freeCapacity += [memBlock capacity];
    [memBlock setState:BS_FREE];
    
    if (previous != nil && [previous state] == BS_FREE &&  
        next != nil && [next state] == BS_FREE) {
        [self resizeMemBlock:previous toCapacity:[previous capacity] + [memBlock capacity] + [next capacity]];

        if (lastBlock == next)
            lastBlock = previous;

        [self removeFreeMemBlock:next];
        [next remove];
        [next release];
        
        [memBlock remove];
        [memBlock release];
        
        return previous;
    }
    
    if (previous != nil && [previous state] == BS_FREE) {
        if (lastBlock == memBlock)
            lastBlock = previous;

        [self resizeMemBlock:previous toCapacity:[previous capacity] + [memBlock capacity]];
        [memBlock remove];
        [memBlock release];
        
        return previous;
    }
    
    if (next != nil && [next state] == BS_FREE) {
        if (lastBlock == next)
            lastBlock = memBlock;

        [memBlock setCapacity:[memBlock capacity] + [next capacity]];
        [memBlock setState:BS_FREE];

        [self insertFreeMemBlock:memBlock];
        [self removeFreeMemBlock:next];
        
        [next remove];
        [next release];
        
        return memBlock;
    }
    
    [memBlock setState:BS_FREE];
    [self insertFreeMemBlock:memBlock];
    
    return memBlock;
}

- (VBOMemBlock *)packMemBlock:(VBOMemBlock *)freeBlock {
    VBOMemBlock* first = [freeBlock next];
    if (first == nil)
        return nil;
    
    VBOMemBlock* previous = [freeBlock previous];
    VBOMemBlock* last = first;
    int size = 0;
    int address = [first address];
    do {
        [last setAddress:[previous address] + [previous capacity]];
        size += [last capacity];
        previous = last;
        last = [last next];
    } while (last != nil && [last state] != BS_FREE);
    
    if (size <= [freeBlock capacity]) {
        memcpy(buffer + [freeBlock address], buffer + address, size);
    } else {
        uint8_t* temp = malloc(size);
        memcpy(temp, buffer + address, size);
        memcpy(buffer + [freeBlock address], temp, size);
        free(temp);
    }
    
    // remove the free block
    if (firstBlock == freeBlock)
        firstBlock = [freeBlock next];
    
    [self removeFreeMemBlock:freeBlock];
    [freeBlock remove];
    [freeBlock release];
    
    return last;
}

- (void)pack {
    if (buffer == NULL)
        [NSException raise:@"InvalidStateException" format:@"buffer must be mapped before it can be packed"];
    
    if (totalCapacity == freeCapacity || ([lastBlock state] == BS_FREE && [lastBlock capacity] == freeCapacity))
        return;
    
    VBOMemBlock* freeBlock = firstBlock; // find first free block
    while (freeBlock != nil && [freeBlock state] != BS_FREE)
        freeBlock = [freeBlock next];
    
    int capacity = 0; // total capacity of the blocks that we killed so far
    while (freeBlock != nil && [freeBlock next] != nil) {
        int t = [freeBlock capacity];
        freeBlock = [self packMemBlock:freeBlock];
        capacity += t;
    }

    if (capacity > 0) {
        if ([lastBlock state] == BS_FREE) {
            [self resizeMemBlock:lastBlock toCapacity:[freeBlock capacity] + capacity];
        } else {
            VBOMemBlock* newBlock = [[VBOMemBlock alloc] initBlockIn:self at:[lastBlock address] + [lastBlock capacity] capacity:capacity];
            [self insertFreeMemBlock:newBlock];
            [newBlock insertBetweenPrevious:lastBlock next:nil];
            lastBlock = newBlock;
        }
    }
}

- (void)freeAllBlocks {
    if (freeCapacity == totalCapacity)
        return;
    
    VBOMemBlock* block = firstBlock;
    do {
        if ([block state] != BS_FREE)
            block = [self freeMemBlock:block];
        block = [block next];
    } while (block != nil);
}

- (void)dealloc {
    [self unmapBuffer];
    [self deactivate];
    if (vboId != 0)
        glDeleteBuffers(1, &vboId);

    [freeBlocksByCapacity release];
    
    VBOMemBlock* block = lastBlock;
    while (block != nil) {
        block = [block previous];
        if (block != nil)
            [[block next] release];
    }
    
    [super dealloc];
}

@end
