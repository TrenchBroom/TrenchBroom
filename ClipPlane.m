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

#import "ClipPlane.h"
#import "Map.h"
#import "Entity.h"
#import "Face.h"
#import "MutableFace.h"
#import "Brush.h"
#import "MutableBrush.h"
#import "PickingHit.h"
#import "PickingHitList.h"

@implementation ClipPlane

- (void)addPoint:(TVector3f *)thePoint hitList:(PickingHitList *)theHitList {
    NSAssert(numPoints >= 0 && numPoints < 3, @"number of points must be between 0 and 3");
    
    points[numPoints] = *thePoint;
    hitLists[numPoints] = [theHitList retain];
    numPoints++;
}

- (void)updatePoint:(int)index x:(float)x y:(float)y z:(float)z {
    NSAssert(index >= 0 && index < numPoints, @"index out of bounds");
    
    points[index].x = x;
    points[index].y = y;
    points[index].z = z;
}

- (void)removeLastPoint {
    NSAssert(numPoints > 0 && numPoints <= 3, @"number of points must be between 0 and 3");

    numPoints--;
    [hitLists[numPoints] release];
    hitLists[numPoints] = nil;
}

- (int)numPoints {
    return numPoints;
}

- (TVector3f *)point:(int)index {
    NSAssert(index >= 0 && index <= numPoints, @"index out of bounds");
    
    return &points[index];
}

- (PickingHitList *)hitList:(int)index {
    NSAssert(index >= 0 && index <= numPoints, @"index out of bounds");
    
    return hitLists[index];
}

- (void)setClipMode:(EClipMode)theClipMode {
    clipMode = theClipMode;
}

- (EClipMode)clipMode {
    return clipMode;
}

- (MutableFace *)face:(BOOL)front {
    if (numPoints < 2)
        return nil;

    TVector3f* p1 = [self point:0];
    TVector3f* p2 = [self point:1];
    TVector3f* p3 = NULL;
    
    if (numPoints < 3) {
        const TVector3f* norm = NULL;
        if (p1->x != p2->x && 
            p1->y != p2->y && 
            p1->z != p2->z) {
            norm = &ZAxisPos;
        } else if (p1->x == p2->x && 
                   p1->y != p2->y && 
                   p1->z != p2->z) {
            norm = &XAxisPos;
        } else if (p1->x != p2->x && 
                   p1->y == p2->y && 
                   p1->z != p2->z) {
            norm = &YAxisPos;
        } else if (p1->x != p2->x && 
                   p1->y != p2->y && 
                   p1->z == p2->z) {
            norm = &ZAxisPos;
        } else {
            NSSet* faces1 = [[self hitList:0] objectsOfType:HT_FACE];
            NSSet* faces2 = [[self hitList:1] objectsOfType:HT_FACE];
            
            NSSet* both = [[NSMutableSet alloc] initWithSet:faces1];
            [both intersectsSet:faces2];
            
            if ([both count] > 0) {
                id <Face> face = [[both objectEnumerator] nextObject];
                norm = [face norm];
            }
            
            [both release];
        }
        
        if (norm != NULL) {
            TVector3f t;
            scaleV3f(norm, -10000, &t);
            
            p3 = malloc(sizeof(TVector3f));
            p3->x = roundf(t.x);
            p3->y = roundf(t.y);
            p3->z = roundf(t.z);
            addV3f(p3, p1, p3);
        }    
    } else {
        p3 = [self point:2];
    }
    
    if (p3 == NULL)
        return nil;
    
    id <Face> template = [[[self hitList:0] firstHitOfType:HT_FACE ignoreOccluders:YES] object];
    const TBoundingBox* worldBounds = [template worldBounds];
    
    MutableFace* face = nil;
    if (front)
        face = [[MutableFace alloc] initWithWorldBounds:worldBounds point1:p1 point2:p2 point3:p3];
    else
        face = [[MutableFace alloc] initWithWorldBounds:worldBounds point1:p3 point2:p2 point3:p1];
    
    [face setTexture:[template texture]];
    [face setXOffset:[template xOffset]];
    [face setYOffset:[template yOffset]];
    [face setXScale:[template xScale]];
    [face setYScale:[template yScale]];
    [face setRotation:[template rotation]];
    
    if (numPoints < 3)
        free(p3);
    
    return [face autorelease];
}

- (void)clipBrush:(id <Brush>)brush firstResult:(id <Brush>*)firstResult secondResult:(id <Brush>*)secondResult {
    MutableFace* clipFace = clipMode == CM_BACK ? [self face:NO] : [self face:YES];
    if (clipFace != nil) {
        id <Map> map = [[brush entity] map];
        MutableBrush* newBrush = [[MutableBrush alloc] initWithWorldBounds:[map worldBounds] brushTemplate:brush];

        if (![newBrush addFace:clipFace]) {
            [newBrush release];
            newBrush = nil;
        } else {
            *firstResult = newBrush;
            [newBrush autorelease];
        }
        
        if (clipMode == CM_SPLIT) {
            clipFace = [self face:NO];
            if (clipFace != nil) {
                newBrush = [[MutableBrush alloc] initWithWorldBounds:[map worldBounds] brushTemplate:brush];
                if (![newBrush addFace:clipFace]) {
                    [newBrush release];
                    newBrush = nil;
                } else {
                    *secondResult = newBrush;
                    [newBrush autorelease];
                }
            }
        }
    }
}

- (void)reset {
    for (int i = 0; i < numPoints; i++) {
        [hitLists[i] release];
        hitLists[i] = nil;
    }
    
    numPoints = 0;
}

- (void)dealloc {
    [self reset];
    [super dealloc];
}

@end
