//
//  ChoiceArgument.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ChoiceArgument.h"


@implementation ChoiceArgument

- (id)initWithKey:(int)theKey value:(NSString *)theValue {
    NSAssert(theValue != nil, @"value must not be nil");
    if (self = [self init]) {
        key = theKey;
        value = [theValue retain];
    }
    
    return self;
}

- (int)key {
    return key;
}

- (NSString *)value {
    return value;
}

- (void)dealloc {
    [value release];
    [super dealloc];
}

@end
