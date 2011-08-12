//
//  OctreeNode.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

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

@interface OctreeNode : NSObject {
    @private
    int minSize;
    TVector3i min;
    TVector3i max;
    NSMutableSet* objects;
    OctreeNode* children[8];
}

- (id)initWithMin:(TVector3i *)theMin max:(TVector3i *)theMax minSize:(int)theMinSize;

- (BOOL)addObject:(id)theObject bounds:(const TBoundingBox *)theBounds;
- (BOOL)removeObject:(id)theObject bounds:(const TBoundingBox *)theBounds;

- (void)addObjectsForRay:(TRay *)ray to:(NSMutableArray *)list;

@end
