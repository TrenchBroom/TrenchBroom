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
#import "MutablePrefab.h"
#import "PrefabGroup.h"
#import "MutablePrefabGroup.h"
#import "MutableEntity.h"
#import "MutableBrush.h"

NSString* const PrefabAdded = @"PrefabAdded";
NSString* const PrefabRemoved = @"PrefabRemoved";
NSString* const PrefabGroupAdded = @"PrefabGroupAdded";
NSString* const PrefabGroupRemoved = @"PrefabGroupRemoved";
NSString* const PrefabGroupChanged = @"PrefabGroupChanged";
NSString* const PrefabKey = @"Prefab";
NSString* const PrefabGroupKey = @"PrefabGroup";

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
        nameToPrefabGroup = [[NSMutableDictionary alloc] init];
        prefabGroups = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (void)loadPrefabsAtPath:(NSString *)thePath readOnly:(BOOL)readOnly {
    NSFileManager* fileManager = [NSFileManager defaultManager];
    
    BOOL directory = NO;
    BOOL exists = [fileManager fileExistsAtPath:thePath isDirectory:&directory];
    
    if (!exists || !directory) {
        NSLog(@"Cannot load prefabs because '%@' does not exist or is not a directory");
        return;
    }
    
    NSArray* groupNames = [fileManager contentsOfDirectoryAtPath:thePath error:NULL];
    if (groupNames != nil) {
        NSEnumerator* groupNameEn = [groupNames objectEnumerator];
        NSString* groupName;
        while ((groupName = [groupNameEn nextObject])) {
            NSLog(@"Loading group '%@'", groupName);
            NSString* groupPath = [NSString pathWithComponents:[NSArray arrayWithObjects:thePath, groupName, nil]];
            
            NSArray* prefabNames = [fileManager contentsOfDirectoryAtPath:groupPath error:NULL];
            NSEnumerator* prefabNameEn = [prefabNames objectEnumerator];
            NSString* prefabName;
            while ((prefabName = [prefabNameEn nextObject])) {
                if ([[prefabName pathExtension] isEqualToString:@"map"]) {
                    NSString* prefabPath = [NSString pathWithComponents:[NSArray arrayWithObjects:thePath, groupName, prefabName, nil]];
                    NSLog(@"Loading prefab from '%@'", prefabPath);
                    
                    NSData* prefabData = [NSData dataWithContentsOfMappedFile:prefabPath];
                    id <PrefabGroup> prefabGroup = [self prefabGroupWithName:groupName create:YES];
                    [self createPrefabFromData:prefabData name:[prefabName stringByDeletingPathExtension] group:prefabGroup readOnly:readOnly];
                }
            }
        }
    }
}

- (id <PrefabGroup>)prefabGroupWithName:(NSString *)prefabGroupName create:(BOOL)create {
    id <PrefabGroup> prefabGroup = [nameToPrefabGroup objectForKey:prefabGroupName];
    if (prefabGroup == nil && create) {
        prefabGroup = [[MutablePrefabGroup alloc] initWithName:prefabGroupName];
        [prefabGroups addObject:prefabGroup];
        [nameToPrefabGroup setObject:prefabGroup forKey:prefabGroupName];
        [prefabGroup release];
        sorted = NO;
        
        NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:prefabGroup forKey:PrefabGroupKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PrefabGroupAdded object:self userInfo:userInfo];
        [userInfo release];
    }
    
    return prefabGroup;
}

- (void)addPrefab:(MutablePrefab *)prefab group:(MutablePrefabGroup *)prefabGroup {
    [prefabGroup addPrefab:prefab];
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:prefab forKey:PrefabKey];
   
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:PrefabAdded object:self userInfo:userInfo];
    [userInfo release];
}

- (id <Prefab>)createPrefabFromData:(NSData *)prefabData name:(NSString *)prefabName group:(id <PrefabGroup>)prefabGroup readOnly:(BOOL)readOnly {
    if (prefabData == nil)
        [NSException raise:NSInvalidArgumentException format:@"prefab data must not be nil"];
    if (prefabName == nil)
        [NSException raise:NSInvalidArgumentException format:@"prefab name must not be nil"];
    if (prefabGroup == nil)
        [NSException raise:NSInvalidArgumentException format:@"prefab group must not be nil"];
    
    MapParser* parser = [[MapParser alloc] initWithData:prefabData];
    MutablePrefab* prefab = [[MutablePrefab alloc] initWithName:prefabName group:prefabGroup readOnly:readOnly];
    
    [parser parseMap:prefab withProgressIndicator:nil];
    [parser release];

    [prefab translateToOrigin];
    [self addPrefab:prefab group:prefabGroup];
    
    NSLog(@"Loaded prefab '%@'", prefabName);
    return [prefab autorelease];
}

- (id <Prefab>)createPrefabFromBrushTemplates:(NSSet *)brushTemplates name:(NSString *)prefabName group:(id <PrefabGroup>)prefabGroup {
    if (brushTemplates == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush template set data must not be nil"];
    if (prefabName == nil)
        [NSException raise:NSInvalidArgumentException format:@"prefab name must not be nil"];
    if (prefabGroup == nil)
        [NSException raise:NSInvalidArgumentException format:@"prefab group must not be nil"];

    if ([brushTemplates count] == 0)
        return nil;
    
    MutablePrefab* prefab = [[MutablePrefab alloc] initWithName:prefabName];
    MutableEntity* entity = [[MutableEntity alloc] init];
    [prefab addEntity:entity];
    [entity release];
    
    NSEnumerator* brushEn = [brushTemplates objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        id <Brush> newBrush = [[MutableBrush alloc] initWithTemplate:brush];
        [entity addBrush:newBrush];
        [newBrush release];
    }
    
    [prefab translateToOrigin];
    [self addPrefab:prefab group:(MutablePrefabGroup *)prefabGroup];
    
    return [prefab autorelease];
}

- (void)removePrefab:(id <Prefab>)prefab {
    if (prefab == nil)
        [NSException raise:NSInvalidArgumentException format:@"prefab must not be nil"];
    
    if ([prefab readOnly])
        [NSException raise:NSInvalidArgumentException format:@"cannot remove read only prefab"];
    
    MutablePrefabGroup* prefabGroup = (MutablePrefabGroup *)[prefab prefabGroup];

    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:prefab forKey:PrefabKey];
    [userInfo setObject:prefabGroup forKey:PrefabGroupKey];

    [prefabGroup removePrefab:(MutablePrefab *)prefab];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:PrefabRemoved object:self userInfo:userInfo];
    [userInfo release];

    if ([[prefabGroup prefabs] count] == 0) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:prefabGroup forKey:PrefabGroupKey];
        
        [prefabGroups removeObject:prefabGroup];
        [nameToPrefabGroup removeObjectForKey:[prefabGroup name]];

        [center postNotificationName:PrefabGroupRemoved object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (NSArray *)groups {
    if (!sorted) {
        [prefabGroups sortUsingSelector:@selector(compareByName:)];
        sorted = YES;
    }
    
    return prefabGroups;
}

- (void)dealloc {
    [prefabGroups release];
    [nameToPrefabGroup release];
    [super dealloc];
}

@end
