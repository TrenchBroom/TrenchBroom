//
//  Wad.h
//  TrenchBroom
//
//  Created by Kristian Duske on 20.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class WadEntry;

@interface Wad : NSObject {
    @private
    NSMutableDictionary* entries;
    NSString* name;
}

- (id)initWithName:(NSString *)aName;

- (WadEntry *)createEntryWithType:(EWadEntryType)type name:(NSString *)aName data:(NSData *)data;
- (WadEntry *)entryForName:(NSString *)aName;

- (NSString *)name;

@end
