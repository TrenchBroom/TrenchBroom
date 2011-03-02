//
//  PrefabManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PrefabManager.h"
#import "MapParser.h"
#import "Prefab.h"

NSString* const PrefabAdded = @"PrefabAdded";
NSString* const PrefabRemoved = @"PrefabRemoved";
NSString* const PrefabKey = @"Prefab";

static PrefabManager* sharedInstance = nil;

@implementation PrefabManager
+ (PrefabManager *)sharedPrefabManager {
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
        prefabs = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)loadPrefabsAtPath:(NSString *)thePath {
    NSFileManager* fileManager = [NSFileManager defaultManager];
    
    BOOL directory = NO;
    BOOL exists = [fileManager fileExistsAtPath:thePath isDirectory:&directory];
    
    if (!exists || !directory) {
        NSLog(@"Cannot load prefabs because '%@' does not exist or is not a directory");
        return;
    }
    
    NSDirectoryEnumerator* dirEn = [fileManager enumeratorAtPath:thePath];
    NSString* subPath;
    while ((subPath = [dirEn nextObject])) {
        NSString* ext = [subPath pathExtension];
        if ([ext isEqualToString:@"map"]) {
            NSArray* pathComponents = [NSArray arrayWithObjects:thePath, subPath, nil];
            NSString* prefabPath = [NSString pathWithComponents:pathComponents];
            NSLog(@"Loading prefab from '%@'", prefabPath);

            NSData* prefabData = [NSData dataWithContentsOfMappedFile:prefabPath];
            NSString* prefabName = [prefabPath lastPathComponent];
            
            [self loadPrefab:prefabData name:prefabName];
        }
    }
    
}

- (void)loadPrefab:(NSData *)prefabData name:(NSString *)prefabName {
    if (prefabData == nil)
        [NSException raise:NSInvalidArgumentException format:@"prefab data must not be nil"];
    if (prefabName == nil)
        [NSException raise:NSInvalidArgumentException format:@"prefab name must not be nil"];
    
    MapParser* parser = [[MapParser alloc] initWithData:prefabData];
    Prefab* prefab = [[Prefab alloc] init];
    
    [parser parseMap:prefab withProgressIndicator:nil];
    [parser release];

    [prefab translateToOrigin];
    
    [prefabs setObject:prefab forKey:prefabName];
    NSLog(@"Loaded prefab '%@'", prefabName);
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:PrefabAdded object:self userInfo:[NSDictionary dictionaryWithObject:prefab forKey:PrefabKey]];
    
    [prefab release];
}

- (NSArray *)prefabs {
    NSMutableArray* result = [[NSMutableArray alloc] initWithArray:[prefabs allValues]];
    
    // TODO sort by name
    
    return [result autorelease];
}

- (void)dealloc {
    [prefabs release];
    [super dealloc];
}

@end
