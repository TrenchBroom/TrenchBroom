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

@interface Picker : NSObject {
    @private
    Octree* octree;
}

- (id)initWithDocument:(MapDocument *)theDocument;

- (PickingHitList *)pickObjects:(Ray3D *)theRay include:(NSSet *)includedObjects exclude:(NSSet *)excludedObjects;

@end
