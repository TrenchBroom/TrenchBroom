//
//  VBOManagerTest.m
//  TrenchBroom
//
//  Created by Kristian Duske on 19.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "VBOManagerTestCase.h"
#import "VBOManager.h"
#import "VBOMemBlock.h"

@implementation VBOManagerTestCase

- (void)testAlloc {
    VBOManager* manager = [[VBOManager alloc] initWithTotalCapacity:2000];
    
    VBOMemBlock* block = [manager allocMemBlock:3000];
    STAssertNil(block, @"allocating a block with too little memory should return nil");
    
    block = [manager allocMemBlock:800];
    STAssertNotNil(block, @"allocating a block with sufficient memory should never return nil");
    STAssertEquals(800, [block capacity], @"allocated block should have exactly the requested size");
    STAssertFalse([block free], @"allocated block should not be free");
    STAssertEquals(1200, [manager freeCapacity], @"manager should have correct free capacity");
    
    VBOMemBlock* block2 = [manager allocMemBlock:1201];
    STAssertNil(block2, @"allocating a block with too little memory should return nil");

    block2 = [manager allocMemBlock:1200];
    STAssertNotNil(block2, @"allocating a block with sufficient memory should never return nil");
    STAssertEquals(1200, [block2 capacity], @"allocated block should have exactly the requested size, but has %i", [block2 capacity]);
    STAssertFalse([block2 free], @"allocated block should not be free");
    STAssertEquals(0, [manager freeCapacity], @"manager should have correct free capacity");
    
    [manager release];
}

- (void)testFreeAndMergeWithPrevious {
    VBOManager* manager = [[VBOManager alloc] initWithTotalCapacity:2000];
    
    VBOMemBlock* previous = [manager allocMemBlock:400];
    VBOMemBlock* block = [manager allocMemBlock:600];
    STAssertEquals(1000, [manager freeCapacity], @"allocating memory");
    
    [manager freeMemBlock:previous];
    STAssertEquals(1400, [manager freeCapacity], @"returning a memory block should free memory");
    
    [manager freeMemBlock:block];
    STAssertEquals(2000, [manager freeCapacity], @"returning a memory block should free memory");
    
    block = [manager allocMemBlock:2000];
    STAssertNotNil(block, @"after returning blocks in any order, free space should be merged");
    
    [manager release];
}

- (void)testFreeAndMergeWithNext {
    VBOManager* manager = [[VBOManager alloc] initWithTotalCapacity:2000];
    
    VBOMemBlock* block = [manager allocMemBlock:600];
    VBOMemBlock* next = [manager allocMemBlock:200];
    STAssertEquals(1200, [manager freeCapacity], @"allocating memory");
    
    [manager freeMemBlock:next];
    STAssertEquals(1400, [manager freeCapacity], @"returning a memory block should free memory");
    
    [manager freeMemBlock:block];
    STAssertEquals(2000, [manager freeCapacity], @"returning a memory block should free memory");
    
    block = [manager allocMemBlock:2000];
    STAssertNotNil(block, @"after returning blocks in any order, free space should be merged");
    
    [manager release];
}

- (void)testFreeAndMergeWithNextAndPrevious {
    VBOManager* manager = [[VBOManager alloc] initWithTotalCapacity:2000];
    
    VBOMemBlock* previous = [manager allocMemBlock:400];
    VBOMemBlock* block = [manager allocMemBlock:600];
    VBOMemBlock* next = [manager allocMemBlock:200];
    STAssertEquals(800, [manager freeCapacity], @"allocating memory");
    
    [manager freeMemBlock:previous];
    STAssertEquals(1200, [manager freeCapacity], @"returning a memory block should free memory");
    
    [manager freeMemBlock:next];
    STAssertEquals(1400, [manager freeCapacity], @"returning a memory block should free memory");

    [manager freeMemBlock:block];
    STAssertEquals(2000, [manager freeCapacity], @"returning a memory block should free memory");
    
    block = [manager allocMemBlock:2000];
    STAssertNotNil(block, @"after returning blocks in any order, free space should be merged");
    
    [manager release];
}

@end
