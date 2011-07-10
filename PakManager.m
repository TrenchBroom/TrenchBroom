//
//  PakManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 08.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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

        NSEnumerator* contentEn = [contents objectEnumerator];
        NSString* content;
        while ((content = [contentEn nextObject])) {
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

- (void)release {
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
    
    NSEnumerator* pakPathEn = [thePakPaths reverseObjectEnumerator];
    NSString* pakPath;
    while ((pakPath = [pakPathEn nextObject])) {
        NSArray* pakDirectories = [self pakDirectoriesAtPath:pakPath];
        if (pakDirectories != nil) {
            NSEnumerator* pakDirectoryEn = [pakDirectories reverseObjectEnumerator];
            PakDirectory* pakDirectory;
            while ((pakDirectory = [pakDirectoryEn nextObject])) {
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
