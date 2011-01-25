//
//  Edge.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Edge.h"


@implementation Edge
- (id)initWithStartIndex:(int)theStartIndex endIndex:(int)theEndIndex {
    if (self = [self init]) {
        [self setStartIndex:theStartIndex];
        [self setEndIndex:theEndIndex];
    }
    
    return self;
}

- (int)startIndex {
    return startIndex;
}

- (int)endIndex {
    return endIndex;
}


- (Vector3f *)startVertex:(NSArray *)vertices {
    return [vertices objectAtIndex:startIndex];
}

- (Vector3f *)endVertex:(NSArray *)vertices {
    return [vertices objectAtIndex:endIndex];
}

- (void)setStartIndex:(int)theStartIndex {
    startIndex = theStartIndex;
}

- (void)setEndIndex:(int)theEndIndex {
    endIndex = theEndIndex;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"start: %i, end: %i", startIndex, endIndex];
}

@end
