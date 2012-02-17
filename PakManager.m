/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "PakManager.h"
#import "PakDirectory.h"
#import "PakDirectoryEntry.h"

static PakManager* sharedInstance = nil;

@interface PakManager (private)

- (NSArray *)pakDirectoriesAtPath:(NSString *)thePath;

@end

@implementation PakManager (private)

- (NSArray *)pakDirectoriesAtPath:(NSString *)thePath {
    NSMutableArray* pakDirectories = [directories objectForKey:thePath];
    if (pakDirectories == nil) {
        NSFileManager* fileManager = [NSFileManager defaultManager];
        BOOL isDir = NO;
        BOOL exists = [fileManager fileExistsAtPath:thePath isDirectory:&isDir];
        if (!exists || !isDir) {
            NSLog(@"%@ does not exist or is not a directory", thePath);
            return nil;
        }
        
        NSArray* contents = [fileManager contentsOfDirectoryAtPath:thePath error:NULL];
        if (contents == nil) {
            NSLog(@"Warning: An error occured while accessing directory %@", thePath);
            return nil;
        }
        
        pakDirectories = [[NSMutableArray alloc] init];

        for (NSString* content in contents) {
            NSString* pakFileName = [content lowercaseString];
            if ([[pakFileName pathExtension] isEqualToString:@"pak"]) {
                NSString* pakFilePath = [thePath stringByAppendingPathComponent:pakFileName];
                if ([fileManager isReadableFileAtPath:pakFilePath]) {
                    PakDirectory* pakDirectory = [[PakDirectory alloc] initWithPath:pakFilePath];
                    [pakDirectories addObject:pakDirectory];
                    [pakDirectory release];
                }
            }
        }
        
        if ([pakDirectories count] == 0) {
            [pakDirectories release];
            NSLog(@"Warning: %@ does not contain any pak files", thePath);
            return nil;
        } else {
            [pakDirectories sortUsingSelector:@selector(compareByName:)];
            [directories setObject:pakDirectories forKey:thePath];
        }
        
        [pakDirectories release];
    }
    
    return pakDirectories;
}

@end

@implementation PakManager

+ (PakManager *)sharedManager {
    @synchronized(self) {
        if (sharedInstance == nil)
            sharedInstance = [[self alloc] init];
    }
    return sharedInstance;
}

+ (id)allocWithZone:(NSZone *)zone {
    @synchronized(self) {
        if (sharedInstance == nil) {
            sharedInstance = [super allocWithZone:zone];
            return sharedInstance;  // assignment and return on first allocation
        }
    }
    return nil; // on subsequent allocation attempts return nil
}

- (id)copyWithZone:(NSZone *)zone {
    return self;
}

- (id)retain {
    return self;
}

- (NSUInteger)retainCount {
    return UINT_MAX;  // denotes an object that cannot be released
}

- (oneway void)release {
    //do nothing
}

- (id)autorelease {
    return self;
}

- (id)init {
    if ((self = [super init])) {
        directories = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [directories release];
    [super dealloc];
}

- (NSData *)entryWithName:(NSString *)theEntryName pakPaths:(NSArray *)thePakPaths {
    NSAssert(theEntryName != nil, @"entry name must not be nil");
    NSAssert(thePakPaths != nil, @"pak path array must not be nil");
    NSAssert([thePakPaths count] > 0, @"pak path array must not be empty");
    
    for (NSString* pakPath in [thePakPaths reverseObjectEnumerator]) {
        NSArray* pakDirectories = [self pakDirectoriesAtPath:pakPath];
        if (pakDirectories != nil) {
            for (PakDirectory* pakDirectory in [pakDirectories reverseObjectEnumerator]) {
                NSData* entry = [pakDirectory entryForName:theEntryName];
                if (entry != nil)
                    return entry;
            }
        }
    }
    
    NSLog(@"No entry with name %@ was found in pak paths %@", theEntryName, thePakPaths);
    return nil;
}

@end
