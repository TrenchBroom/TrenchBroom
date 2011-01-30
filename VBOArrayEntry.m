//
//  VBOArrayEntry.m
//  TrenchBroom
//
//  Created by Kristian Duske on 27.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "VBOArrayEntry.h"


@implementation VBOArrayEntry
- (id)initWithObject:(id)theObject index:(int)theIndex count:(int)theCount {
    if (theObject == nil)
        [NSException raise:NSInvalidArgumentException format:@"object must not be nil"];
    
    if (self = [self init]) {
        object = [theObject retain];
        index = theIndex;
        count = theCount;
    }
    
    return self;
}

- (id)object {
    return object;
}

- (int)index {
    return index;
}

- (int)count {
    return count;
}

- (void)dealloc {
    [object release];
    [super dealloc];
}

@end
