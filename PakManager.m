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
    if (self = [super init]) {
        directories = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [directories release];
    [super dealloc];
}

- (PakDirectoryEntry *)entryFromPakDir:(NSString *)thePakDir entryName:(NSString *)theEntryName {
    NSMutableArray* directoriesInPakDir = [directories objectForKey:thePakDir];
    if (directoriesInPakDir == nil) {
        NSFileManager* fileManager = [NSFileManager defaultManager];
        BOOL isDir = NO;
        BOOL exists = [fileManager fileExistsAtPath:thePakDir isDirectory:&isDir];
        if (!exists || !isDir) {
            NSLog(@"Warning: Pak directory %@ does not exist", thePakDir);
            return nil;
        }
        
        NSArray* contents = [fileManager contentsOfDirectoryAtPath:thePakDir error:NULL];
        if (contents == nil) {
            NSLog(@"Warning: An error occured accessing pak directory %@", thePakDir);
            return nil;
        }

        directoriesInPakDir = [[NSMutableArray alloc] init];
        
        NSEnumerator* contentsEn = [contents objectEnumerator];
        NSString* content;
        while ((content = [contentsEn nextObject])) {
            if ([[content pathExtension] isEqualToString:@".pak"]) {
                NSString* pakPath = [thePakDir stringByAppendingPathComponent:content];
                if ([fileManager isReadableFileAtPath:pakPath]) {
                    NSData* pakData = [[NSData alloc] initWithContentsOfMappedFile:pakPath];
                    PakDirectory* pakDirectory = [[PakDirectory alloc] initWithName:content data:pakData];
                    [directoriesInPakDir addObject:pakDirectory];
                    
                    [pakDirectory release];
                    [pakData release];
                }
            }
        }
        
        if ([directoriesInPakDir count] == 0) {
            [directoriesInPakDir release];
            NSLog(@"Warning: Pak directory %@ does not contain any pak files", thePakDir);
            return nil;
        } else {
            [directoriesInPakDir sortUsingSelector:@selector(compareByName:)];
            [directories setObject:directoriesInPakDir forKey:thePakDir];
        }
        
        [directoriesInPakDir release];
    }
    
    NSEnumerator* directoryEn = [directoriesInPakDir objectEnumerator];
    PakDirectory* directory;
    while ((directory = [directoryEn nextObject])) {
        PakDirectoryEntry* entry = [directory entryForName:theEntryName];
        if (entry != nil)
            return entry;
    }
    
    NSLog(@"Warning: Pak directory %@ does not contain an entry with name @%", thePakDir, theEntryName);
    return nil;
}

@end
