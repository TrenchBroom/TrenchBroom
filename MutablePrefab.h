//
//  Prefab.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Prefab.h"
#import "Map.h"

@class MutablePrefabGroup;
@class BoundingBox;
@class Vector3f;

@interface MutablePrefab : NSObject <Prefab, Map> {
    @private
    MutablePrefabGroup* prefabGroup;
    NSString* name;
    NSNumber* prefabId;
    BOOL readOnly;
    NSMutableArray* entities;
    BoundingBox* bounds;
    BoundingBox* maxBounds;
    Vector3f* center;
}

- (id)initWithName:(NSString *)theName group:(MutablePrefabGroup *)thePrefabGroup readOnly:(BOOL)isReadOnly;

- (void)translateToOrigin;
- (void)setPrefabGroup:(MutablePrefabGroup *)thePrefabGroup;
@end
