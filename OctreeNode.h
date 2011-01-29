//
//  OctreeNode.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    CP_WSB = 0,
    CP_WST = 1,
    CP_WNB = 2,
    CP_WNT = 3,
    CP_ESB = 4,
    CP_EST = 5,
    CP_ENB = 6,
    CP_ENT = 7
    
} EChildPosition;

@class Vector3i;
@class BoundingBox;
@class Ray3D;

@interface OctreeNode : NSObject {
    @private
    int minSize;
    Vector3i* min;
    Vector3i* max;
    NSMutableSet* objects;
    OctreeNode* children[8];
}

- (id)initWithMin:(Vector3i *)theMin max:(Vector3i *)theMax minSize:(int)theMinSize;

- (BOOL)addObject:(id)theObject bounds:(BoundingBox *)theBounds;
- (BOOL)removeObject:(id)theObject bounds:(BoundingBox *)theBounds;

- (void)addObjectsForRay:(Ray3D *)theRay to:(NSMutableSet *)theSet;

@end
