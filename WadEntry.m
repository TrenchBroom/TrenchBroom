//
//  WadEntry.m
//  TrenchBroom
//
//  Created by Kristian Duske on 20.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "WadEntry.h"


@implementation WadEntry
- (id)initWithType:(EWadEntryType)aType name:(NSString *)aName data:(NSData *)someData {
    if (aName == nil)
        [NSException raise:NSInvalidArgumentException format:@"name must not be nil"];
    if (someData == nil)
        [NSException raise:NSInvalidArgumentException format:@"data must not be nil"];
    
    if (self = [self init]) {
        type = aType;
        name = [aName retain];
        data = [someData retain];
    }
    
    return self;
}

- (EWadEntryType)type {
    return type;
}

- (NSString *)name {
    return name;
}

- (NSData *)data {
    return data;
}

- (void)dealloc {
    [name release];
    [data release];
    [super dealloc];
}

@end
