//
//  PrefabGroup.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "MutablePrefabGroup.h"
#import "MutablePrefab.h"
#import "IdGenerator.h"

@implementation MutablePrefabGroup

- (id)init {
    if (self = [super init]) {
        prefabGroupId = [[[IdGenerator sharedGenerator] getId] retain];
        prefabs = [[NSMutableArray alloc] init];
        nameToPrefab = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithName:(NSString *)theName {
    if (self = [self init]) {
        name = [theName retain];
    }
    
    return self;
}

- (id)copyWithZone:(NSZone *)zone {
    MutablePrefabGroup* copy = [[MutablePrefabGroup allocWithZone:zone] initWithName:name];
    copy->prefabGroupId = [prefabGroupId retain];
    [copy->prefabs release];
    copy->prefabs = [prefabs retain];
    return copy;
}

- (NSNumber *)prefabGroupId {
    return prefabGroupId;
}

- (NSString *)name {
    return name;
}

- (BOOL)readOnly {
    NSEnumerator* prefabEn = [prefabs objectEnumerator];
    id <Prefab> prefab;
    while ((prefab = [prefabEn nextObject]))
        if ([prefab readOnly])
            return YES;
    
    return NO;
}

- (NSArray *)prefabs{
    if (!sorted) {
        [prefabs sortUsingSelector:@selector(compareByName:)];
        sorted = YES;
    }
    
    return prefabs;
}

- (id <Prefab>) prefabWithName:(NSString *)prefabName {
    return [nameToPrefab objectForKey:[prefabName lowercaseString]];
}

- (void)setName:(NSString *)theName {
    [name release];
    name = [theName retain];
}

- (void)addPrefab:(MutablePrefab *)thePrefab {
    NSString* prefabName = [[thePrefab name] lowercaseString];
    if ([nameToPrefab objectForKey:prefabName] != nil)
        return;
    
    [prefabs addObject:thePrefab];
    [nameToPrefab setObject:thePrefab forKey:prefabName];
    [thePrefab setPrefabGroup:self];
    sorted = NO;
}

- (void)removePrefab:(MutablePrefab *)thePrefab {
    [thePrefab setPrefabGroup:nil];
    [nameToPrefab removeObjectForKey:[[thePrefab name] lowercaseString]];
    [prefabs removeObject:thePrefab];
}

- (NSComparisonResult)compareByName:(id <PrefabGroup>)prefabGroup {
    return [name localizedCaseInsensitiveCompare:[prefabGroup name]];
}

- (NSString *)description {
    return name;
}

- (void)dealloc {
    [prefabGroupId release];
    [name release];
    [prefabs release];
    [nameToPrefab release];
    [super dealloc];
}

@end
