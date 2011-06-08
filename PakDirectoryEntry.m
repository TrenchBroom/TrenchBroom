//
//  PakDirectoryEntry.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "PakDirectoryEntry.h"


@implementation PakDirectoryEntry

- (id)initWithName:(NSString *)theName address:(int)theAddress size:(int)theSize {
    NSAssert(theName != nil, @"name must not be nil");
    
    if (self = [self init]) {
        name = [theName retain];
        address = theAddress;
        size = theSize;
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

- (NSData *)entryDataFromFileData:(NSData *)theFileData {
    return [theFileData subdataWithRange:NSMakeRange(address, size)];
}

@end
