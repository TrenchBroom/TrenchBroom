//
//  Octree.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Map;
@class OctreeNode;
@class Ray3D;

@interface Octree : NSObject {
    @private
    OctreeNode* root;
    Map* map;
}

- (id)initWithMap:(Map *)theMap minSize:(int)theMinSize;

- (void)addObjectsForRay:(Ray3D *)theRay to:(NSMutableSet *)theSet;
@end
