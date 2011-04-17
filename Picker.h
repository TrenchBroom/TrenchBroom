//
//  PickingManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class MapDocument;
@class Octree;
@class Ray3D;
@class PickingHitList;
@protocol Filter;

@interface Picker : NSObject {
    @private
    Octree* octree;
}

- (id)initWithDocument:(MapDocument *)theDocument;

- (PickingHitList *)pickObjects:(Ray3D *)ray filter:(id <Filter>)filter;

@end
