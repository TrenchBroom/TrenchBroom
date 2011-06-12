//
//  EntityModelFrameGroup.h
//  TrenchBroom
//
//  Created by Kristian Duske on 11.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@class AliasFrame;

@interface AliasFrameGroup : NSObject {
    float* times;
    NSArray* frames;
    TBoundingBox bounds;
}

- (id)initWithFrames:(NSArray *)theFrames times:(float *)theTimes;

- (int)frameCount;
- (float)timeAtIndex:(int)theIndex;
- (AliasFrame *)frameAtIndex:(int)theIndex;
- (const TBoundingBox *)bounds;

@end
