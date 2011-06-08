//
//  PakDirectory.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "PakDirectory.h"
#import "PakDirectoryEntry.h"
#import "IO.h"

@implementation PakDirectory

- (id)initWithName:(NSString *)theName data:(NSData *)theData {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theData != nil, @"data must not be nil");
    
    if (self = [self init]) {
        name = [theName retain];
        entries = [[NSMutableDictionary alloc] init];

        NSDate* startDate = [NSDate date];
        if (!readPakDirectory(theData, self)) {
            NSLog(@"Unable to read directory of pak file %@", name);
            [self release];
            return nil;
        }

        NSTimeInterval duration = [startDate timeIntervalSinceNow];
        NSLog(@"Read directory of pak file %@ in %f seconds", name, -duration);
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    [entries release];
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (void)addEntry:(PakDirectoryEntry *)theEntry {
    NSAssert(theEntry != nil, @"entry must not be nil");
    [entries setObject:theEntry forKey:[theEntry name]];
}

- (PakDirectoryEntry *)entryForName:(NSString *)theName {
    return [entries objectForKey:theName];
}

- (NSComparisonResult)compareByName:(PakDirectory *)other {
    return -1 * [name caseInsensitiveCompare:[other name]];
}

@end
