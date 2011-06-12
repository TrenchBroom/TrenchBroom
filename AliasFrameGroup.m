//
//  EntityModelFrameGroup.m
//  TrenchBroom
//
//  Created by Kristian Duske on 11.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "AliasFrameGroup.h"
#import "AliasFrame.h"

@implementation AliasFrameGroup

- (id)initWithFrames:(NSArray *)theFrames times:(float *)theTimes {
    NSAssert(theFrames != nil, @"frame array must not be nil");
    NSAssert(theTimes != NULL, @"time array must not be nil");
    NSAssert([theFrames count] > 0, @"frame array must contain at least one frame");
    
    if (self = [self init]) {
        frames = [[NSArray alloc] initWithArray:theFrames];
        times = malloc([frames count] * sizeof(float));
        memcpy(times, theTimes, [frames count] * sizeof(float));
        
        NSEnumerator* frameEn = [frames objectEnumerator];
        AliasFrame* frame = [frameEn nextObject];
        bounds = *[frame bounds];

        while ((frame = [frameEn nextObject]))
            mergeBoundsWithBounds(&bounds, [frame bounds], &bounds);
    }
    
    return self;
}

- (void)dealloc {
    [frames release];
    free(times);
    [super dealloc];
}

- (int)frameCount {
    return [frames count];
}

- (float)timeAtIndex:(int)theIndex {
    NSAssert(theIndex >= 0 && theIndex < [self frameCount], @"frame index out of bounds");
    return times[theIndex];
}

- (AliasFrame *)frameAtIndex:(int)theIndex {
    NSAssert(theIndex >= 0 && theIndex < [self frameCount], @"frame index out of bounds");
    return [frames objectAtIndex:theIndex];
}

- (const TBoundingBox *)bounds {
    return &bounds;
}

@end
