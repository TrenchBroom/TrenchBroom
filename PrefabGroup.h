//
//  PrefabGroup.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol Prefab;

@protocol PrefabGroup <NSObject, NSCopying>

- (NSNumber *)prefabGroupId;
- (NSString *)name;
- (BOOL)readOnly;
- (NSArray *)prefabs;
- (id <Prefab>) prefabWithName:(NSString *)prefabName;

- (NSComparisonResult)compareByName:(id <PrefabGroup>)prefabGroup;
@end
