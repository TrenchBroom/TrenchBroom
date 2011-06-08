//
//  PakDirectory.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class PakDirectoryEntry;

@interface PakDirectory : NSObject {
    NSString* name;
    NSMutableDictionary* entries;
}

- (id)initWithName:(NSString *)theName data:(NSData *)theData;

- (NSString *)name;
- (void)addEntry:(PakDirectoryEntry *)theEntry;
- (PakDirectoryEntry *)entryForName:(NSString *)theName;

- (NSComparisonResult)compareByName:(PakDirectory *)other;
@end
