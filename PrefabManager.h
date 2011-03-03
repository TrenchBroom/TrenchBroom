//
//  PrefabManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

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
}

+ (PrefabManager *)sharedPrefabManager;

- (void)loadPrefabsAtPath:(NSString *)thePath readOnly:(BOOL)readOnly;

- (id <PrefabGroup>)prefabGroupWithName:(NSString *)prefabGroupName create:(BOOL)create;

- (id <Prefab>)createPrefabFromData:(NSData *)prefabData name:(NSString *)prefabName group:(id <PrefabGroup>)prefabGroup readOnly:(BOOL)readOnly;
- (id <Prefab>)createPrefabFromBrushTemplates:(NSSet *)brushTemplates name:(NSString *)prefabName group:(id <PrefabGroup>)prefabGroup;

- (void)removePrefab:(id <Prefab>)prefab;

- (NSArray *)groups;

@end
