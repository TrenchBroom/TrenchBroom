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
#import "MapWriter.h"
#import "NSFileManager+AppSupportCategory.h"

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

- (oneway void)release {
    //do nothing
}

- (id)autorelease {
    return self;
}

- (id)init {
    if ((self = [super init])) {
        nameToPrefabGroup = [[NSMutableDictionary alloc] init];
        prefabGroups = [[NSMutableArray alloc] init];
        worldBounds.min.x = -0x8000;
        worldBounds.min.y = -0x8000;
        worldBounds.min.z = -0x8000;
        worldBounds.max.x = +0x8000;
        worldBounds.max.y = +0x8000;
        worldBounds.max.z = +0x8000;
    }
    
    return self;
}

- (void)loadPrefabsAtPath:(NSString *)thePath readOnly:(BOOL)readOnly {
    NSFileManager* fileManager = [NSFileManager defaultManager];
    
    BOOL directory = NO;
    BOOL exists = [fileManager fileExistsAtPath:thePath isDirectory:&directory];
    
    if (!exists || !directory) {
        NSLog(@"Cannot load prefabs because '%@' does not exist or is not a directory", thePath);
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

- (id <PrefabGroup>)prefabGroupWithName:(NSString *)name create:(BOOL)create {
    id <PrefabGroup> prefabGroup = [nameToPrefabGroup objectForKey:[name lowercaseString]];
    if (prefabGroup == nil && create) {
        prefabGroup = [[MutablePrefabGroup alloc] initWithName:name];
        [prefabGroups addObject:prefabGroup];
        [nameToPrefabGroup setObject:prefabGroup forKey:[name lowercaseString]];
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
    MapParser* parser = [[MapParser alloc] initWithData:prefabData];
    MutablePrefab* prefab = [[MutablePrefab alloc] initWithWorldBounds:&worldBounds name:prefabName group:prefabGroup readOnly:readOnly];
    
    [parser parseMap:prefab withProgressIndicator:nil];
    [parser release];

    [self addPrefab:prefab group:prefabGroup];
    
    NSLog(@"Loaded prefab '%@'", prefabName);
    return [prefab autorelease];
}

- (void)writePrefab:(MutablePrefab *)prefab {
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSString* appSupportPath = [fileManager findApplicationSupportFolder];
    NSString* groupName = [[prefab prefabGroup] name];
    NSString* fileName = [NSString stringWithFormat:@"%@.map", [prefab name]];
    
    NSString* directoryPath = [NSString pathWithComponents:[NSArray arrayWithObjects:appSupportPath, @"Prefabs", groupName, nil]];
    [fileManager createDirectoryAtPath:directoryPath withIntermediateDirectories:YES attributes:nil error:NULL];
    
    MapWriter* mapWriter = [[MapWriter alloc] initWithMap:prefab];
    [mapWriter writeToFileAtPath:[NSString pathWithComponents:[NSArray arrayWithObjects:directoryPath, fileName, nil]]];
    [mapWriter release];
}

- (id <Prefab>)createPrefabFromBrushTemplates:(NSSet *)brushTemplates name:(NSString *)prefabName group:(id <PrefabGroup>)prefabGroup {
    if ([brushTemplates count] == 0)
        return nil;
    
    MutablePrefab* prefab = [[MutablePrefab alloc] initWithWorldBounds:&worldBounds name:prefabName group:prefabGroup readOnly:NO];
    NSMutableDictionary* entities = [[NSMutableDictionary alloc] init];
    
    NSEnumerator* brushEn = [brushTemplates objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        id <Entity> brushEntity = [brush entity];
        MutableEntity* newEntity = [entities objectForKey:[brushEntity entityId]];
        if (newEntity == nil) {
            if ([brushEntity isWorldspawn]) {
                newEntity = [[MutableEntity alloc] init];
                [newEntity setProperty:@"classname" value:@"worldspawn"];
            } else {
                newEntity = [[MutableEntity alloc] initWithProperties:[[brush entity] properties]];
            }
            [prefab addEntity:newEntity];
            [entities setObject:newEntity forKey:[[brush entity] entityId]];
            [newEntity release];
        }
        
        id <Brush> newBrush = [[MutableBrush alloc] initWithWorldBounds:&worldBounds brushTemplate:brush];
        [newEntity addBrush:newBrush];
        [newBrush release];
    }
    
    [self addPrefab:prefab group:(MutablePrefabGroup *)prefabGroup];
    [self writePrefab:prefab];
    [entities release];
    
    return [prefab autorelease];
}

- (void)removePrefab:(id <Prefab>)prefab {
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
        [nameToPrefabGroup removeObjectForKey:[[prefabGroup name] lowercaseString]];

        [center postNotificationName:PrefabGroupRemoved object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (NSArray *)prefabGroups {
    if (!sorted) {
        [prefabGroups sortUsingSelector:@selector(compareByName:)];
        sorted = YES;
    }
    
    return prefabGroups;
}

- (id <PrefabGroup>)prefabGroupWithNamePrefix:(NSString *)prefix {
    NSString* lowercasePrefix = [prefix lowercaseString];
    NSEnumerator* groupEn = [prefabGroups objectEnumerator];
    id <PrefabGroup> group;
    while ((group = [groupEn nextObject]))
        if ([[[group name] lowercaseString] hasPrefix:lowercasePrefix])
            return group;
    return nil;
}

- (NSUInteger)indexOfPrefabGroupWithName:(NSString *)name {
    id <PrefabGroup> group = [nameToPrefabGroup objectForKey:[name lowercaseString]];
    if (group == nil)
        return NSNotFound;
    
    return [prefabGroups indexOfObject:group];
}

- (void)dealloc {
    [prefabGroups release];
    [nameToPrefabGroup release];
    [super dealloc];
}

@end
