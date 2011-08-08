//
//  PrefabManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

extern NSString* const PrefabAdded;
extern NSString* const PrefabRemoved;
extern NSString* const PrefabGroupAdded;
extern NSString* const PrefabGroupRemoved;
extern NSString* const PrefabGroupChanged;
extern NSString* const PrefabKey;
extern NSString* const PrefabGroupKey;

@protocol Prefab;
@protocol PrefabGroup;

@interface PrefabManager : NSObject {
    @private
    NSMutableDictionary* nameToPrefabGroup;
    NSMutableArray* prefabGroups;
    BOOL sorted;
    TBoundingBox worldBounds;
}

+ (PrefabManager *)sharedPrefabManager;

- (void)loadPrefabsAtPath:(NSString *)thePath readOnly:(BOOL)readOnly;

- (NSArray *)prefabGroups;
- (id <PrefabGroup>)prefabGroupWithName:(NSString *)name create:(BOOL)create;
- (id <PrefabGroup>)prefabGroupWithNamePrefix:(NSString *)prefix;
- (NSUInteger)indexOfPrefabGroupWithName:(NSString *)name;

- (id <Prefab>)createPrefabFromData:(NSData *)prefabData name:(NSString *)prefabName group:(id <PrefabGroup>)prefabGroup readOnly:(BOOL)readOnly;
- (id <Prefab>)createPrefabFromBrushTemplates:(NSSet *)brushTemplates name:(NSString *)prefabName group:(id <PrefabGroup>)prefabGroup;

- (void)removePrefab:(id <Prefab>)prefab;


@end
