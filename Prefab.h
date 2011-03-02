//
//  Prefab.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Map.h"

@protocol Map;
@class BoundingBox;
@class Vector3f;

@interface Prefab : NSObject <Map> {
    @private
    NSNumber* prefabId;
    NSMutableArray* entities;
    BoundingBox* bounds;
    BoundingBox* maxBounds;
    Vector3f* center;
}

- (NSNumber *)prefabId;
- (NSArray *)entities;

- (void)translateToOrigin;
- (BoundingBox *)bounds;
- (BoundingBox *)maxBounds;
- (Vector3f *)center;
@end
