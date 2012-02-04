/*
Copyright (C) 2010-2012 Kristian Duske

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
