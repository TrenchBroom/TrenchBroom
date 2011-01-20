//
//  Wad.m
//  TrenchBroom
//
//  Created by Kristian Duske on 20.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Wad.h"


@implementation Wad

- (id)init {
    if (self = [super init]) {
        entries = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithName:(NSString *)aName {
    if (aName == nil)
        [NSException raise:NSInvalidArgumentException format:@"name must not be nil"];
    
    if (self = [self init]) {
        name = [aName retain];
    }
    
    return self;
}

- (WadEntry *)createEntryWithType:(EWadEntryType)type name:(NSString *)aName data:(NSData *)data {
    WadEntry* entry = [[WadEntry alloc] initWithType:type name:aName data:data];
    [entries setObject:entry forKey:aName];
    
    return [entry autorelease];
}

- (WadEntry *)entryForName:(NSString *)aName {
    if (aName == nil)
        [NSException raise:NSInvalidArgumentException format:@"name must not be nil"];
    
    return [entries objectForKey:aName];
}

- (NSString *)name {
    return name;
}

- (void)dealloc {
    [entries release];
    [super dealloc];
}

@end
