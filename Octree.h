//
//  Octree.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@class MapDocument;
@class OctreeNode;

@interface Octree : NSObject {
    @private
    int minSize;
    OctreeNode* root;
}

- (id)initWithMap:(MapDocument *)theMap minSize:(int)theMinSize;

- (NSArray *)pickObjectsWithRay:(TRay *)ray;

@end
