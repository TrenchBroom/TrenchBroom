//
//  WadEntry.m
//  TrenchBroom
//
//  Created by Kristian Duske on 20.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "WadEntry.h"


@implementation WadEntry
- (id)initWithName:(NSString *)theName {
    if (theName == nil)
        [NSException raise:NSInvalidArgumentException format:@"name must not be nil"];
    
    if (self = [self init]) {
        name = [theName retain];
    }
    
    return self;
}

- (NSString *)name {
    return name;
}

- (void)dealloc {
    [name release];
    [super dealloc];
}

@end
