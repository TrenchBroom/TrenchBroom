//
//  VBOManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "VBOManager.h"

CFComparisonResult compareMemBlocks(const void *val1, const void *val2, void *context) {
    VBOMemBlock *block1 = (VBOMemBlock *)val1;
    VBOMemBlock *block2 = (VBOMemBlock *)val2;
    
    if ([block1 capacity] < [block2 capacity])
        return kCFCompareLessThan;
    
    if ([block1 capacity] > [block2 capacity])
        return kCFCompareGreaterThan;
    
    return kCFCompareEqualTo;
}

@implementation VBOManager

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
        firstBlock = [[VBOMemBlock alloc] initWithBlockCapacity:capacity];
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
    if (![memBlock free])
        return;
    
    unsigned index = [self findMemBlock:memBlock];
    if (index >= [freeBlocksByCapacity count])
        [freeBlocksByCapacity addObject:memBlock];
    else
        [freeBlocksByCapacity insertObject:memBlock atIndex:index];
}

- (void)removeFreeMemBlock:(VBOMemBlock *)memBlock {
    if (![memBlock free])
        return;
    
    unsigned index = [self findMemBlock:memBlock];
    VBOMemBlock* candidate = [freeBlocksByCapacity objectAtIndex:index];
    int i = index;
    
    while (i > 0 && candidate != memBlock && [candidate free] && [candidate capacity] == [memBlock capacity])
        candidate = [freeBlocksByCapacity objectAtIndex:--i];
    
    if (candidate != memBlock) {
        i = index + 1;
        candidate = [freeBlocksByCapacity objectAtIndex:i];
        while (i < [freeBlocksByCapacity count] - 1 && candidate != memBlock && [candidate free] && [candidate capacity] == [memBlock capacity])
            candidate = [freeBlocksByCapacity objectAtIndex:++i];
    }
    
    if (candidate != memBlock)
        [NSException raise:NSInvalidArgumentException format:@"the given memory block could not be found"];
    
    [freeBlocksByCapacity removeObjectAtIndex:i];
}

- (void)resizeMemBlock:(VBOMemBlock *)memBlock toCapacity:(int)capacity {
    if (capacity == [memBlock capacity])
        return;

    if (![memBlock free])
        return;
    
    [self removeFreeMemBlock:memBlock];
    [memBlock setCapacity:capacity];
    [self insertFreeMemBlock:memBlock];
}

- (VBOMemBlock *)allocMemBlock:(int)capacity {
    
    if (capacity > freeCapacity)
        return nil;
    
    VBOMemBlock* query = [[VBOMemBlock alloc]initWithBlockCapacity:capacity];
    unsigned index = [self findMemBlock:query];
    [query release];
    
    // todo try to rearrange memory?
    if (index >= [freeBlocksByCapacity count])
        return nil;
    
    VBOMemBlock* memBlock = [freeBlocksByCapacity objectAtIndex:index];
    [freeBlocksByCapacity removeObjectAtIndex:index];
    
    // split the memory block
    if (capacity < [memBlock capacity]) {
        VBOMemBlock *remainder = [[VBOMemBlock alloc] initWithBlockCapacity:[memBlock capacity] - capacity];
        [memBlock setCapacity:capacity];

        VBOMemBlock* next = [memBlock next];
        
        [remainder setNext:next];
        [remainder setPrevious:memBlock];
        
        [memBlock setNext:remainder];
        [next setPrevious:remainder];

        [self insertFreeMemBlock:remainder];
    }
    
    [memBlock setFree:NO];
    freeCapacity -= [memBlock capacity];
    
    return memBlock;
}

- (void)freeMemBlock:(VBOMemBlock *)memBlock {

    VBOMemBlock* previous = [memBlock previous];
    VBOMemBlock* next = [memBlock next];
    
    if ([previous free] && [next free]) {
        VBOMemBlock* nextNext = [next next];
        
        [previous setNext:nextNext];
        [nextNext setPrevious:previous];
        
        [memBlock setPrevious:nil];
        [memBlock setNext:nil];
        [next setPrevious:nil];
        [next setNext:nil];
        
        [self resizeMemBlock:previous toCapacity:[previous capacity] + [memBlock capacity] + [next capacity]];
        [self removeFreeMemBlock:next];
    } else if ([previous free]) {
        [previous setNext:next];
        [next setPrevious:previous];
        
        [memBlock setPrevious:nil];
        [memBlock setNext:nil];

        [self resizeMemBlock:previous toCapacity:[previous capacity] + [memBlock capacity]];
    } else if ([next free]) {
        VBOMemBlock* nextNext = [next next];
        
        [memBlock setNext:nextNext];
        [nextNext setPrevious:memBlock];
        
        [next setPrevious:nil];
        [next setNext:nil];
        
        [memBlock setCapacity:[memBlock capacity] + [next capacity]];
        [memBlock setFree:YES];
        [self insertFreeMemBlock:memBlock];
        [self removeFreeMemBlock:next];
    } else {
        [memBlock setFree:YES];
        [self insertFreeMemBlock:memBlock];
    }
    
    freeCapacity += [memBlock capacity];
}

- (void)dealloc {
    [freeBlocksByCapacity release];
    [firstBlock release];
    [super dealloc];
}

@end
