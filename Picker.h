//
//  PickingManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@class MapDocument;
@class Octree;
@class PickingHitList;
@protocol Filter;

@interface Picker : NSObject {
    @private
    Octree* octree;
}

- (id)initWithDocument:(MapDocument *)theDocument;

- (PickingHitList *)pickObjects:(TRay *)ray filter:(id <Filter>)filter;

@end
