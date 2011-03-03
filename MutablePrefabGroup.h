//
//  PrefabGroup.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "PrefabGroup.h"

@class MutablePrefab;

@interface MutablePrefabGroup : NSObject <PrefabGroup> {
    @private
    NSNumber* prefabGroupId;
    NSString* name;
    NSMutableArray* prefabs;
    BOOL sorted;
}

- (id)initWithName:(NSString *)theName;
- (void)setName:(NSString *)theName;

- (void)addPrefab:(MutablePrefab *)thePrefab;
- (void)removePrefab:(MutablePrefab *)thePrefab;
@end
