//
//  WadPaletteEntry.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "WadEntry.h"


@interface WadPaletteEntry : WadEntry {
    NSData* data;
}

- (id)initWithName:(NSString *)theName data:(NSData *)theData;

- (NSData *)data;

@end
