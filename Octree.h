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
    OctreeNode* root;
    MapDocument* map;
}

- (id)initWithDocument:(MapDocument *)theDocument minSize:(int)theMinSize;

- (NSArray *)pickObjectsWithRay:(TRay *)ray;
@end
