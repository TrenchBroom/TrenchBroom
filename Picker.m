/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "Picker.h"
#import "Octree.h"
#import "Entity.h"
#import "Brush.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Filter.h"

@implementation Picker

- (id)initWithMap:(MapDocument *)theMap {
    if ((self = [self init])) {
        octree = [[Octree alloc] initWithMap:theMap minSize:64];
    }
    
    return self;
}

- (PickingHitList *)pickObjects:(const TRay *)ray filter:(id <Filter>)filter {
    PickingHitList* hitList = [[PickingHitList alloc] init];
    NSArray* objects = [octree pickObjectsWithRay:ray];
    
    NSEnumerator* objectEn = [objects objectEnumerator];
    id object;
    while ((object = [objectEn nextObject])) {
        if ([object conformsToProtocol:@protocol(Brush)]) {
            id <Brush> brush = (id <Brush>)object;
            if (filter == nil || [filter isBrushPickable:brush])
                [brush pick:ray hitList:hitList];
        } else if ([object conformsToProtocol:@protocol(Entity)]) {
            id <Entity> entity = (id <Entity>)object;
            if (filter == nil || [filter isEntityPickable:entity])
                [entity pick:ray hitList:hitList];
        }
    }
    
    return [hitList autorelease];
}

- (void)pickCloseFaces:(const TRay *)theRay brushes:(NSArray *)theBrushes maxDistance:(float)theMaxDistance hitList:(PickingHitList *)theHitList {
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [brush pickClosestFace:theRay maxDistance:theMaxDistance hitList:theHitList];
}

- (void)pickVertices:(const TRay *)theRay brushes:(NSArray *)theBrushes handleRadius:(float)theHandleRadius hitList:(PickingHitList *)theHitList {
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [brush pickVertices:theRay handleRadius:theHandleRadius hitList:theHitList];
}

- (void)dealloc {
    [octree release];
    [super dealloc];
}
@end
