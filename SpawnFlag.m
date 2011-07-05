//
//  SpawnFlag.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "SpawnFlag.h"


@implementation SpawnFlag

- (id)initWithName:(NSString *)theName flag:(int)theFlag {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theFlag > 0, @"flag must be a positive integer");
    
    if ((self = [self init])) {
        name = [theName retain];
        flag = theFlag;
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (int)flag {
    return flag;
}

- (NSComparisonResult)compareByFlag:(SpawnFlag *)otherFlag {
    if (flag > [otherFlag flag])
        return NSOrderedAscending;
    if (flag < [otherFlag flag])
        return NSOrderedDescending;
    return NSOrderedSame;
}
@end
