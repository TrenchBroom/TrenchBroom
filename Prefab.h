//
//  Prefab.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@protocol PrefabGroup;
@class MapDocument;

@protocol Prefab <NSObject>

- (NSString *)name;
- (NSNumber *)prefabId;
- (BOOL)readOnly;
- (id <PrefabGroup>)prefabGroup;

- (NSArray *)entities;

- (TVector3f *)center;
- (TBoundingBox *)bounds;
- (TBoundingBox *)maxBounds;

- (NSComparisonResult)compareByName:(id <Prefab>)prefab;

@end
