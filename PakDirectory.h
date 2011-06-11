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
    NSString* path;
    NSFileHandle* handle;
    NSMutableDictionary* entries;
}

- (id)initWithPath:(NSString *)thePath;

- (NSString *)path;
- (NSData *)entryForName:(NSString *)theName;

- (NSComparisonResult)compareByName:(PakDirectory *)other;
@end
