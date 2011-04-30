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
@class MapDocument;

@interface MutablePrefab : NSObject <Prefab, Map> {
    @private
    MutablePrefabGroup* prefabGroup;
    NSString* name;
    NSNumber* prefabId;
    BOOL readOnly;
    NSMutableArray* entities;
    TBoundingBox bounds;
    TBoundingBox maxBounds;
    TVector3f center;
    BOOL valid;
}

- (id)initWithName:(NSString *)theName group:(MutablePrefabGroup *)thePrefabGroup readOnly:(BOOL)isReadOnly;

- (void)setPrefabGroup:(MutablePrefabGroup *)thePrefabGroup;
@end
