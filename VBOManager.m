//
//  VBOManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "VBOManager.h"


@implementation VBOManager

- (id)init {
    if (self = [super init]) {
        freeMemByIndex = [[NSMutableArray alloc] initWithCapacity:10];
        freeMemBySize = [[NSMutableArray alloc] initWithCapacity:10];
    }
         
    return self;
}

- (id)initWithSize:(int)aSize {
    if (self = [self init]) {
        size = aSize;
        VBOMemBlock* memBlock = [[VBOMemBlock alloc] initWithIndex:0 size:aSize];
        [freeMemByIndex addObject:memBlock];
        [freeMemBySize addObject:memBlock];
        [freeMem release];
    }
    
    return self;
}

- (void)insertMemBlock:(VBOMemBlock *)aMemBlock {
    
    if ([freeMemByIndex count]) {
        [freeMemByIndex addObject:aMemBlock];
    } else {
        if ([aMemBlock index] > [[freeMemByIndex lastObject] index]) {
            if (![[freeMemByIndex lastObject] mergeWith:aMemBlock])
                [freeMemByIndex addObject:aMemBlock];
        } else {
            // TODO binary search (CFArrayBSearchValues)
            int blockIndex;
            for (VBOMemBlock* memBlock in freeMemByIndex) {
                if ([aMemBlock index] > [memBlock index]) {
                    VBOMemBlock* previous;
                    if (blockIndex > 0)
                        previous = [freeMemByIndex objectAtIndex:blockIndex];
                    
                    VBOMemBlock* next;
                    if (blockIndex + 1 < [freeMemByIndex count])
                        next = [freeMemByIndex objectAtIndex:(blockIndex + 1)];
                    
                    if ([previous mergeWith:aMemBlock]) {
                        if ([next mergeWith:previous]) {
                            [freeMemByIndex removeObjectAtIndex:blockIndex];
                            // change next's position in freeMemBySize appropriately
                        } else {
                            // change previous' position in freeMemBySize appropriately
                        }
                    } else if ([next mergeWith:aMemBlock]) {
                        // change next's position in freeMemBySize appropriately
                    } else {
                        [freeMemByIndex insertObject:aMemBlock atIndex:blockIndex];
                    }
                    break;
                }
                blockIndex++;
            }
            
            if (blockIndex > 0) {
                if ([previous mergeWith:aMemBlock]) {
                }
            }
        }
        
        // merge
        if (blockIndex > 0 {
            VBOMemBlock* previous = [freeMemByIndex objectAtIndex:(blockIndex - 1)];
            if ([aMemBlock index] == [previous index] + [previous size]) {
            }
        }
    }
    
    if ([freeMemBySize count] == 0 || [aMemBlock size] >= [[freeMemBySize lastObject] size]) {
        [freeMemBySize addObject:aMemBlock];
    } else {
        // TODO binary search (CFArrayBSearchValues)
        int blockIndex = 0;
        for (VBOMemBlock* memBlock in freeMemBySize) {
            if ([aMemBlock size] >= [memBlock size]) {
                [freeMemBySize insertObject:aMemBlock atIndex:blockIndex];
                break;
            }
            blockIndex++;
        }
    }
}

- (void)removeMemBlock:(VBOMemBlock *)memBlock {
    // TODO binary search (CFArrayBSearchValues)
    int blockIndex = 0;
    for (VBOMemBlock* memBlock in freeMemByIndex) {
        if ([aMemBlock index] > [memBlock index]) {
            [freeMemByIndex insertObject:aMemBlock atIndex:blockIndex];
            break;
        }
        blockIndex++;
    }
    
    blockIndex = 0;
    for (VBOMemBlock* memBlock in freeMemBySize) {
        if ([aMemBlock size] >= [memBlock size]) {
            [freeMemBySize insertObject:aMemBlock atIndex:blockIndex];
            break;
        }
        blockIndex++;
    }
}


- (VBOMemBlock*)getMemBlockSize:(int)aSize {
    // TODO look at CFArrayBSearchValues to perform binary search
    int blockIndex = 0;
    for (VBOMemBlock* memBlock in freeMem) {
        if ([memBlock size] >= aSize) {
            [freeMem autorelease]
            [freeMem removeObjectAtIndex:index];
            
            if ([freeMem size] > aSize) {
                VBOMemBlock* result = [[VBOMemBlock alloc] initWithIndex:[freeMem index] size:aSize];
            }
        }
        
        blockIndex++;
    }
    
    return nil;
}

- (void)dealloc {
    [freeMemByIndex release];
    [freeMemBySize release];

    [super dealloc];
}

@end
