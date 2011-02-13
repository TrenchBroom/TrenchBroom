//
//  SegmentIterator.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;

@interface SegmentIterator : NSObject {
    @private
    NSArray* vertices;
    BOOL vertical;
    BOOL clockwise;
    int leftIndex;
    int rightIndex;
}

+ (SegmentIterator *)iteratorWithVertices:(NSArray *)theVertices vertical:(BOOL)isVertical clockwise:(BOOL)isClockwise;

- (id)initWithVertices:(NSArray *)theVertices vertical:(BOOL)isVertical clockwise:(BOOL)isClockwise;

- (Vector3f *)nextLeft;
- (Vector3f *)nextRight;

- (Vector3f *)forwardLeftTo:(float)a;
- (Vector3f *)forwardRightTo:(float)a;

@end
