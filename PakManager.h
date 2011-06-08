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
- (PakDirectoryEntry *)entryFromPakDir:(NSString *)thePakDir entryName:(NSString *)theEntryName;

@end
