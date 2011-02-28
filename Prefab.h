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
    NSMutableArray* entities;
    BoundingBox* bounds;
    Vector3f* center;
}
- (NSArray *)entities;

- (void)translateToOrigin;
- (BoundingBox *)bounds;
- (Vector3f *)center;
@end
