//
//  VBOArrayEntry.m
//  TrenchBroom
//
//  Created by Kristian Duske on 27.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "VBOArrayEntry.h"


@implementation VBOArrayEntry
- (id)initWithIndex:(int)theIndex count:(int)theCount {
    if (self = [self init]) {
        index = theIndex;
        count = theCount;
    }
    
    return self;
}

- (int)index {
    return index;
}

- (int)count {
    return count;
}

@end
