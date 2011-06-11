//
//  PakManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 08.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class PakDirectoryEntry;

@interface PakManager : NSObject {
    NSMutableDictionary* directories;
}

+ (PakManager *)sharedManager;

- (NSData *)entryWithName:(NSString *)theEntryName pakPaths:(NSArray *)thePakPaths;

@end
