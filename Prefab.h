//
//  Prefab.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol PrefabGroup;
@class BoundingBox;
@class MapDocument;
@class Vector3f;

@protocol Prefab <NSObject>

- (NSString *)name;
- (NSNumber *)prefabId;
- (BOOL)readOnly;
- (id <PrefabGroup>)prefabGroup;

- (NSArray *)entities;

- (Vector3f *)center;
- (BoundingBox *)bounds;
- (BoundingBox *)maxBounds;

- (NSComparisonResult)compareByName:(id <Prefab>)prefab;

@end
