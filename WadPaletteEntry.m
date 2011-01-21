//
//  WadPaletteEntry.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "WadPaletteEntry.h"


@implementation WadPaletteEntry
- (id)initWithName:(NSString *)theName data:(NSData *)theData {
    if (theData == nil)
        [NSException raise:NSInvalidArgumentException format:@"data must not be nil"];
    
    if (self = [super initWithName:theName]) {
        data = [theData retain];
    }
    
    return self;
}

- (NSData *)data {
    return data;
}

- (void)dealloc {
    [data release];
    [super dealloc];
}

@end
