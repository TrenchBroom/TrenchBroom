//
//  OctreeNode.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OctreeNode.h"
#import "math.h"

@interface OctreeNode (private)

- (BOOL)bounds:(const TBoundingBox *)theBounds containedInMin:(TVector3i *)theMin max:(TVector3i *)theMax;

@end

@implementation OctreeNode (private)

- (BOOL)bounds:(const TBoundingBox *)theBounds containedInMin:(TVector3i *)theMin max:(TVector3i *)theMax {
    return fgte(theBounds->min.x, theMin->x)
    && fgte(theBounds->min.y, theMin->y)
    && fgte(theBounds->min.z, theMin->z)
    && flte(theBounds->max.x, theMax->x)
    && flte(theBounds->max.y, theMax->y)
    && flte(theBounds->max.z, theMax->z);
}

@end

@implementation OctreeNode

- (id)initWithMin:(TVector3i *)theMin max:(TVector3i *)theMax minSize:(int)theMinSize {
    if ((self = [self init])) {
        min = *theMin;
        max = *theMax;
        minSize = theMinSize;
        objects = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (BOOL)addObject:(id)theObject bounds:(const TBoundingBox *)theBounds toChild:(int)theIndex {
    if (children[theIndex] == nil) {
        TVector3i childMin, childMax;
        switch (theIndex) {
            case CP_WSB:
                childMin.x = min.x;
                childMin.y = min.y;
                childMin.z = min.z;
                childMax.x = (min.x + max.x) / 2;
                childMax.y = (min.y + max.y) / 2;
                childMax.z = (min.z + max.z) / 2;
                break;
            case CP_WST:
                childMin.x = min.x;
                childMin.y = min.y;
                childMin.z = (min.z + max.z) / 2;
                childMax.x = (min.x + max.x) / 2;
                childMax.y = (min.y + max.y) / 2;
                childMax.z = max.z;
                break;
            case CP_WNB:
                childMin.x = min.x;
                childMin.y = (min.y + max.y) / 2;
                childMin.z = min.z;
                childMax.x = (min.x + max.x) / 2;
                childMax.y = max.y;
                childMax.z = (min.z + max.z) / 2;
                break;
            case CP_WNT:
                childMin.x = min.x;
                childMin.y = (min.y + max.y) / 2;
                childMin.z = (min.z + max.z) / 2;
                childMax.x = (min.x + max.x) / 2;
                childMax.y = max.y;
                childMax.z = max.z;
                break;
            case CP_ESB:
                childMin.x = (min.x + max.x) / 2;
                childMin.y = min.y;
                childMin.z = min.z;
                childMax.x = max.x;
                childMax.y = (min.y + max.y) / 2;
                childMax.z = (min.z + max.z) / 2;
                break;
            case CP_EST:
                childMin.x = (min.x + max.x) / 2;
                childMin.y = min.y;
                childMin.z = (min.z + max.z) / 2;
                childMax.x = max.x;
                childMax.y = (min.y + max.y) / 2;
                childMax.z = max.z;
                break;
            case CP_ENB:
                childMin.x = (min.x + max.x) / 2;
                childMin.y = (min.y + max.y) / 2;
                childMin.z = min.z;
                childMax.x = max.x;
                childMax.y = max.y;
                childMax.z = (min.z + max.z) / 2;
                break;
            case CP_ENT:
                childMin.x = (min.x + max.x) / 2;
                childMin.y = (min.y + max.y) / 2;
                childMin.z = (min.z + max.z) / 2;
                childMax.x = max.x;
                childMax.y = max.y;
                childMax.z = max.z;
                break;
            default:
                [NSException raise:NSInvalidArgumentException format:@"child index out of bounds: %i", theIndex];
        }
        children[theIndex] = [[OctreeNode alloc] initWithMin:&childMin max:&childMax minSize:minSize];
    }
    return [children[theIndex] addObject:theObject bounds:theBounds];
}

- (BOOL)addObject:(id)theObject bounds:(const TBoundingBox *)theBounds {
    if (![self bounds:theBounds containedInMin:&min max:&max])
        return NO;
    
    if (max.x - min.x > minSize)
        for (int i = 0; i < 8; i++)
            if ([self addObject:theObject bounds:theBounds toChild:i])
                return YES;
    
    [objects addObject:theObject];
    return YES;
}

- (BOOL)removeObject:(id)theObject bounds:(const TBoundingBox *)theBounds {
    if (![self bounds:theBounds containedInMin:&min max:&max])
        return NO;
    
    for (int i = 0; i < 8; i++)
        if (children[i] != nil && [children[i] removeObject:theObject bounds:theBounds])
            return YES;
    
    [objects removeObject:theObject];
    return YES;
}

- (void)addObjectsForRay:(TRay *)ray to:(NSMutableArray *)list {
    BOOL hit = fgte(ray->origin.x, min.x)
        && fgte(ray->origin.y, min.y)
        && fgte(ray->origin.z, min.z)
        && flte(ray->origin.x, max.x)
        && flte(ray->origin.y, max.y)
        && flte(ray->origin.z, max.z);
    
    if (!hit) {
        TPlane plane;
        if (!hit) {
            if (ray->direction.x > 0) {
                plane.point.x = min.x;
                plane.point.y = min.y;
                plane.point.z = min.z;
                plane.norm = XAxisNeg;
                
                float dist = intersectPlaneWithRay(&plane, ray);
                if (!isnan(dist)) {
                    TVector3f is;
                    rayPointAtDistance(ray, dist, &is);
                    hit = fgte(is.y, min.y) && flte(is.y, max.y) && fgte(is.z, min.z) && flte(is.z, max.z);
                }
            } else if (ray->direction.x < 0) {
                plane.point.x = max.x;
                plane.point.y = max.y;
                plane.point.z = max.z;
                plane.norm = XAxisPos;
                
                float dist = intersectPlaneWithRay(&plane, ray);
                if (!isnan(dist)) {
                    TVector3f is;
                    rayPointAtDistance(ray, dist, &is);
                    hit = fgte(is.y, min.y) && flte(is.y, max.y) && fgte(is.z, min.z) && flte(is.z, max.z);
                }
            }
        }
        
        if (!hit) {
            if (ray->direction.y > 0) {
                plane.point.x = min.x;
                plane.point.y = min.y;
                plane.point.z = min.z;
                plane.norm = YAxisNeg;
                
                float dist = intersectPlaneWithRay(&plane, ray);
                if (!isnan(dist)) {
                    TVector3f is;
                    rayPointAtDistance(ray, dist, &is);
                    hit = fgte(is.x, min.x) && flte(is.x, max.x) && fgte(is.z, min.z) && flte(is.z, max.z);
                }
            } else if (ray->direction.y < 0) {
                plane.point.x = max.x;
                plane.point.y = max.y;
                plane.point.z = max.z;
                plane.norm = YAxisPos;
                
                float dist = intersectPlaneWithRay(&plane, ray);
                if (!isnan(dist)) {
                    TVector3f is;
                    rayPointAtDistance(ray, dist, &is);
                    hit = fgte(is.x, min.x) && flte(is.x, max.x) && fgte(is.z, min.z) && flte(is.z, max.z);
                }
            }
        }
        
        if (!hit) {
            if (ray->direction.z > 0) {
                plane.point.x = min.x;
                plane.point.y = min.y;
                plane.point.z = min.z;
                plane.norm = ZAxisNeg;
                
                float dist = intersectPlaneWithRay(&plane, ray);
                if (!isnan(dist)) {
                    TVector3f is;
                    rayPointAtDistance(ray, dist, &is);
                    hit = fgte(is.x, min.x) && flte(is.x, max.x) && fgte(is.y, min.y) && flte(is.y, max.y);
                }
            } else if (ray->direction.z < 0) {
                plane.point.x = max.x;
                plane.point.y = max.y;
                plane.point.z = max.z;
                plane.norm = ZAxisPos;
                
                float dist = intersectPlaneWithRay(&plane, ray);
                if (!isnan(dist)) {
                    TVector3f is;
                    rayPointAtDistance(ray, dist, &is);
                    hit = fgte(is.x, min.x) && flte(is.x, max.x) && fgte(is.y, min.y) && flte(is.y, max.y);
                }
            }
        }
    }
    
    if (hit) {
        NSEnumerator* objectEn = [objects objectEnumerator];
        id object;
        while ((object = [objectEn nextObject]))
            [list addObject:object];
        
        for (int i = 0; i < 8; i++)
            if (children[i] != nil)
                [children[i] addObjectsForRay:ray to:list];
    }
}

- (void)dealloc {
    for (int i = 0; i < 8; i++)
        if (children[i] != nil)
            [children[i] release];
    
    [objects release];
    [super dealloc];
}

@end
