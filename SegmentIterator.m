//
//  SegmentIterator.m
//  TrenchBroom
//
//  Created by Kristian Duske on 12.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SegmentIterator.h"
#import "Vector3f.h"
#import "Math.h"

@implementation SegmentIterator

+ (SegmentIterator *)iteratorWithVertices:(NSArray *)theVertices vertical:(BOOL)isVertical clockwise:(BOOL)isClockwise {
    return [[[SegmentIterator alloc] initWithVertices:theVertices vertical:isVertical clockwise:isClockwise] autorelease];
}

- (id)initWithVertices:(NSArray *)theVertices vertical:(BOOL)isVertical clockwise:(BOOL)isClockwise {
    if (theVertices == nil)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must not be nil"];
    if ([theVertices count] < 3)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must contain at least three elements"];
    
    if (self = [self init]) {
        vertices = [theVertices retain];
        vertical = isVertical;
        clockwise = isClockwise;
        
        if (vertical) {
            leftIndex = smallestYVertex2D(vertices);
            rightIndex = leftIndex;
        } else {
            leftIndex = smallestXVertex2D(vertices);
            rightIndex = leftIndex;
        }
    }
    
    return self;
}

- (int)leftSucc:(int)index {
    if (clockwise)
        return (index + 1) % [vertices count];
    return (index - 1 + [vertices count]) % [vertices count];
}

- (int)leftPred:(int)index {
    if (clockwise)
        return (index - 1 + [vertices count]) % [vertices count];
    return (index + 1) % [vertices count];
}

- (int)rightSucc:(int)index {
    if (clockwise)
        return (index - 1 + [vertices count]) % [vertices count];
    return (index + 1) % [vertices count];
}

- (int)rightPred:(int)index {
    if (clockwise)
        return (index + 1) % [vertices count];
    return (index - 1 + [vertices count]) % [vertices count];
}

- (Vector3f *)nextLeft {
    Vector3f* left = [vertices objectAtIndex:leftIndex];

    int nextIndex = [self leftSucc:leftIndex];
    Vector3f* next = [vertices objectAtIndex:nextIndex];
    
    if (vertical) {
        if (fgt([next x], [left x]) && flt([next y], [left y]))
            return nil;
    } else {
        if (flt([next y], [left y]) && flte([next x], [left x]))
            return nil;
    }
    
    leftIndex = nextIndex;
    return next;
}

- (Vector3f *)nextRight {
    Vector3f* right = [vertices objectAtIndex:rightIndex];
    
    int nextIndex = [self rightSucc:rightIndex];
    Vector3f* next = [vertices objectAtIndex:nextIndex];

    if (vertical) {
        if (flt([next x], [right x]) && flte([next y], [right y]))
            return nil;
    } else {
        if (fgt([next y], [right y]) && flt([next x], [right x]))
            return nil;
    }
    
    rightIndex = nextIndex;
    return next;
}

- (Vector3f *)forwardLeftTo:(float)a {
    Vector3f* left = [vertices objectAtIndex:leftIndex];
    if (vertical) {
        while (left != nil && flt([left y], a))
            left = [self nextLeft];
    } else {
        while (left != nil && flt([left x], a))
            left = [self nextLeft];
    }

    if (left == nil)
        return nil;
    
    leftIndex = [self leftPred:leftIndex];
    return [vertices objectAtIndex:leftIndex];
}

- (Vector3f *)forwardRightTo:(float)a {
    Vector3f* right = [vertices objectAtIndex:rightIndex];
    if (vertical) {
        while (right != nil && flt([right y], a))
            right = [self nextRight];
    } else {
        while (right != nil &&  flt([right x], a))
            right = [self nextRight];
    }
    
    if (right == nil)
        return nil;
    
    rightIndex = [self rightPred:rightIndex];
    return [vertices objectAtIndex:rightIndex];
}

- (void)dealloc {
    [vertices release];
    [super dealloc];
}
@end
