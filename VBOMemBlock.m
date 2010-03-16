//
//  VBOMemBlock.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "VBOMemBlock.h"


@implementation VBOMemBlock

- (id)initWithIndex:(int)anIndex size:(int)aSize {
    if (self = [self init]) {
        index = anIndex;
        size = aSize;
    }
    
    return self;
}

- (int)index {
    return index;
}

- (int)size {
    return size;
}

- (BOOL)mergeWith:(VBOMemBlock *)aMemBlock {
    if ([aMemBlock index] == index + size) {
        size += [aMemBlock size];
        return YES;
    }
    
    if (index == [aMemBlock index] + [aMemBlock size]) {
        index = [aMemBlock index];
        size += [aMemBlock size];
        return YES;
    }
    
    return NO;
}

@end
