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

#import <Cocoa/Cocoa.h>
#import "Math.h"

extern NSString* const PrefabAdded;
extern NSString* const PrefabRemoved;
extern NSString* const PrefabGroupAdded;
extern NSString* const PrefabGroupRemoved;
extern NSString* const PrefabGroupChanged;
extern NSString* const PrefabKey;
extern NSString* const PrefabGroupKey;

@class TextureManager;
@protocol Prefab;
@protocol PrefabGroup;

@interface PrefabManager : NSObject {
    @private
    NSMutableDictionary* nameToPrefabGroup;
    NSMutableArray* prefabGroups;
    BOOL sorted;
    TBoundingBox worldBounds;
    TextureManager* textureManager;
}

+ (PrefabManager *)sharedPrefabManager;

- (void)loadPrefabsAtPath:(NSString *)thePath readOnly:(BOOL)readOnly;

- (NSArray *)prefabGroups;
- (id <PrefabGroup>)prefabGroupWithName:(NSString *)name create:(BOOL)create;
- (id <PrefabGroup>)prefabGroupWithNamePrefix:(NSString *)prefix;
- (NSUInteger)indexOfPrefabGroupWithName:(NSString *)name;

- (id <Prefab>)createPrefabFromData:(NSData *)prefabData name:(NSString *)prefabName group:(id <PrefabGroup>)prefabGroup readOnly:(BOOL)readOnly;
- (id <Prefab>)createPrefabFromBrushTemplates:(NSArray *)brushTemplates name:(NSString *)prefabName group:(id <PrefabGroup>)prefabGroup;

- (void)renamePrefab:(id <Prefab>)prefab newName:(NSString *)prefabName newPrefabGroupName:(NSString *)prefabGroupName;
- (void)removePrefab:(id <Prefab>)prefab;


@end
