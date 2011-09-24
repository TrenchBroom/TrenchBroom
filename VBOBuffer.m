//
//  VBOManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "VBOBuffer.h"
#import "VBOMemBlock.h"

NSString* const BufferNotMappedException = @"BufferNotMappedException";

int writeBuffer(const uint8_t* buffer, uint8_t* vbo, int address, int count) {
    memcpy(vbo + address, buffer, count);
    return address + count;
}

int writeByte(unsigned char b, uint8_t* vbo, int address){
    vbo[address] = b;
    return address + 1;
}

int writeFloat(float f, uint8_t* vbo, int address) {
    for (int i = 0; i < 4; i++)
        vbo[address + i] = ((char *)&f)[i];
    return address + sizeof(float);
}

int writeColor4fAsBytes(const TVector4f* color, uint8_t* vbo, int address) {
    int a = address;
    a = writeByte(color->x * 0xFF, vbo, a);
    a = writeByte(color->y * 0xFF, vbo, a);
    a = writeByte(color->z * 0xFF, vbo, a);
    a = writeByte(color->w * 0xFF, vbo, a);
    return a;
}

int writeVector4f(const TVector4f* vector, uint8_t* vbo, int address) {
    int a = address;
    a = writeFloat(vector->x, vbo, a);
    a = writeFloat(vector->y, vbo, a);
    a = writeFloat(vector->z, vbo, a);
    a = writeFloat(vector->w, vbo, a);
    return a;
}

int writeVector3f(const TVector3f* vector, uint8_t* vbo, int address) {
    int a = address;
    a = writeFloat(vector->x, vbo, a);
    a = writeFloat(vector->y, vbo, a);
    a = writeFloat(vector->z, vbo, a);
    return a;
}

int writeVector2f(const TVector2f* vector, uint8_t* vbo, int address) {
    int a = address;
    a = writeFloat(vector->x, vbo, a);
    a = writeFloat(vector->y, vbo, a);
    return a;
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
    if ((self = [super init])) {
        freeBlocksByCapacity = [[NSMutableArray alloc] initWithCapacity:10];
    }
         
    return self;
}

- (id)initWithTotalCapacity:(int)capacity {
    if ((self = [self init])) {
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

- (unsigned int)findMemBlockWithMinimalCapacity:(int)capacity inRange:(NSRange)range {
    if (range.length == 1) {
        VBOMemBlock* block = [freeBlocksByCapacity objectAtIndex:range.location];
        if ([block capacity] >= capacity)
            return range.location;
        else
            return [freeBlocksByCapacity count];
    } else {
        int s = range.length / 2;
        int l = [self findMemBlockWithMinimalCapacity:capacity 
                                              inRange:NSMakeRange(range.location, s)];
        int r = [self findMemBlockWithMinimalCapacity:capacity 
                                              inRange:NSMakeRange(range.location + s, range.length - s)];
        if (l < [freeBlocksByCapacity count])
            return l;
        return r;
    }
}

- (unsigned int)findMemBlockWithMinimalCapacity:(int)capacity {
    if ([freeBlocksByCapacity count] == 0)
        return 0;
    return [self findMemBlockWithMinimalCapacity:capacity inRange:NSMakeRange(0, [freeBlocksByCapacity count])];
}

- (void)insertFreeMemBlock:(VBOMemBlock *)memBlock {
    if ([memBlock state] != BS_FREE)
        return;
    
    unsigned index = [self findMemBlockWithMinimalCapacity:[memBlock capacity]];
    if (index >= [freeBlocksByCapacity count])
        [freeBlocksByCapacity addObject:memBlock];
    else
        [freeBlocksByCapacity insertObject:memBlock atIndex:index];
}

- (void)removeFreeMemBlock:(VBOMemBlock *)memBlock {
    if ([memBlock state] != BS_FREE)
        return;
    
    unsigned index = [self findMemBlockWithMinimalCapacity:[memBlock capacity]];
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

    NSAssert(glGetError() == GL_NO_ERROR, @"buffer could not be activated: %i", glGetError());
    active = YES;
}

- (void)deactivate {
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    active = NO;
}

- (BOOL)active {
    return active;
}

- (void)mapBuffer {
    buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    NSAssert(glGetError() == GL_NO_ERROR && buffer != NULL, @"buffer could not be mapped: %i", glGetError());
}

- (void)unmapBuffer {
    if (buffer != NULL) {
        glUnmapBuffer(GL_ARRAY_BUFFER);
        buffer = NULL;
    }
}

- (BOOL)mapped {
    return buffer != NULL;
}

- (uint8_t *)buffer {
    return buffer;
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
    
    unsigned index = [self findMemBlockWithMinimalCapacity:capacity];
    
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
    
    /*
    if (last != nil) {
        [last setAddress:[last address] - [freeBlock capacity]];
        [self resizeMemBlock:last toCapacity:[last capacity] + [freeBlock capacity]];
    } else {
        
    }
     */
    
    if (last != nil) {
        [last setAddress:[last address] - [freeBlock capacity]];
        [self resizeMemBlock:last toCapacity:[last capacity] + [freeBlock capacity]];
    } else {
        VBOMemBlock* newBlock = [[VBOMemBlock alloc] initBlockIn:self at:[previous address] + [previous capacity] capacity:[freeBlock capacity]];
        [self insertFreeMemBlock:newBlock];
        [newBlock insertBetweenPrevious:previous next:nil];
        lastBlock = newBlock;
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
    
    while (freeBlock != nil && [freeBlock next] != nil)
        freeBlock = [self packMemBlock:freeBlock];

    /*
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
     */
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

    VBOMemBlock* block = firstBlock;
    while (block != nil) {
        VBOMemBlock* next = [block next];
        [block release];
        block = next;
    }
    
    [super dealloc];
}

@end
