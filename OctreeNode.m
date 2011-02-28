//
//  OctreeNode.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OctreeNode.h"
#import "Vector3i.h"
#import "Vector3f.h"
#import "BoundingBox.h"
#import "Ray3D.h"
#import "Plane3D.h"
#import "math.h"
#import "MathCache.h"

@implementation OctreeNode

- (id)initWithMin:(Vector3i *)theMin max:(Vector3i *)theMax minSize:(int)theMinSize {
    if (self = [self init]) {
        min = [theMin retain];
        max = [theMax retain];
        minSize = theMinSize;
        objects = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (BOOL)bounds:(BoundingBox *)theBounds containedInMin:(Vector3i *)theMin max:(Vector3i *)theMax {
    return fgte([[theBounds min] x], [theMin x])
        && fgte([[theBounds min] y], [theMin y])
        && fgte([[theBounds min] z], [theMin z])
        && flte([[theBounds max] x], [theMax x])
        && flte([[theBounds max] y], [theMax y])
        && flte([[theBounds max] z], [theMax z]);
}


- (BOOL)addObject:(id)theObject bounds:(BoundingBox *)theBounds toChild:(int)theIndex {
    if (children[theIndex] == nil) {
        Vector3i* childMin = [[Vector3i alloc] init];
        Vector3i* childMax = [[Vector3i alloc] init];
        switch (theIndex) {
            case CP_WSB:
                [childMin setX:[min x]];
                [childMin setY:[min y]];
                [childMin setZ:[min z]];
                [childMax setX:([min x] + [max x]) / 2];
                [childMax setY:([min y] + [max y]) / 2];
                [childMax setZ:([min z] + [max z]) / 2];
                break;
            case CP_WST:
                [childMin setX:[min x]];
                [childMin setY:[min y]];
                [childMin setZ:([min z] + [max z]) / 2];
                [childMax setX:([min x] + [max x]) / 2];
                [childMax setY:([min y] + [max y]) / 2];
                [childMax setZ:[max z]];
                break;
            case CP_WNB:
                [childMin setX:[min x]];
                [childMin setY:([min y] + [max y]) / 2];
                [childMin setZ:[min z]];
                [childMax setX:([min x] + [max x]) / 2];
                [childMax setY:[max y]];
                [childMax setZ:([min z] + [max z]) / 2];
                break;
            case CP_WNT:
                [childMin setX:[min x]];
                [childMin setY:([min y] + [max y]) / 2];
                [childMin setZ:([min z] + [max z]) / 2];
                [childMax setX:([min x] + [max x]) / 2];
                [childMax setY:[max y]];
                [childMax setZ:[max z]];
                break;
            case CP_ESB:
                [childMin setX:([min x] + [max x]) / 2];
                [childMin setY:[min y]];
                [childMin setZ:[min z]];
                [childMax setX:[max x]];
                [childMax setY:([min y] + [max y]) / 2];
                [childMax setZ:([min z] + [max z]) / 2];
                break;
            case CP_EST:
                [childMin setX:([min x] + [max x]) / 2];
                [childMin setY:[min y]];
                [childMin setZ:([min z] + [max z]) / 2];
                [childMax setX:[max x]];
                [childMax setY:([min y] + [max y]) / 2];
                [childMax setZ:[max z]];
                break;
            case CP_ENB:
                [childMin setX:([min x] + [max x]) / 2];
                [childMin setY:([min y] + [max y]) / 2];
                [childMin setZ:[min z]];
                [childMax setX:[max x]];
                [childMax setY:[max y]];
                [childMax setZ:([min z] + [max z]) / 2];
                break;
            case CP_ENT:
                [childMin setX:([min x] + [max x]) / 2];
                [childMin setY:([min y] + [max y]) / 2];
                [childMin setZ:([min z] + [max z]) / 2];
                [childMax setX:[max x]];
                [childMax setY:[max y]];
                [childMax setZ:[max z]];
                break;
            default:
                [NSException raise:NSInvalidArgumentException format:@"child index out of bounds: %i", theIndex];
        }
        children[theIndex] = [[OctreeNode alloc] initWithMin:childMin max:childMax minSize:minSize];
        [childMin release];
        [childMax release];
    }
    return [children[theIndex] addObject:theObject bounds:theBounds];
}

- (BOOL)addObject:(id)theObject bounds:(BoundingBox *)theBounds {
    if (theObject == nil)
        [NSException raise:NSInvalidArgumentException format:@"object must not be nil"];
    if (theBounds == nil)
        [NSException raise:NSInvalidArgumentException format:@"bounds must not be nil"];
    
    if (![self bounds:theBounds containedInMin:min max:max])
        return NO;
    
    if ([max x] - [min x] > minSize)
        for (int i = 0; i < 8; i++)
            if ([self addObject:theObject bounds:theBounds toChild:i])
                return YES;
    
    [objects addObject:theObject];
    return YES;
}

- (BOOL)removeObject:(id)theObject bounds:(BoundingBox *)theBounds {
    if (theObject == nil)
        [NSException raise:NSInvalidArgumentException format:@"object must not be nil"];
    if (theBounds == nil)
        [NSException raise:NSInvalidArgumentException format:@"bounds must not be nil"];
    
    if (![self bounds:theBounds containedInMin:min max:max])
        return NO;
    
    for (int i = 0; i < 8; i++)
        if (children[i] != nil && [children[i] removeObject:theObject bounds:theBounds])
            return YES;
    
    [objects removeObject:theObject];
    return YES;
}

- (void)addObjectsForRay:(Ray3D *)theRay to:(NSMutableSet *)theSet {
    if (theRay == nil)
        [NSException raise:NSInvalidArgumentException format:@"ray must not be nil"];
    if (theSet == nil)
        [NSException raise:NSInvalidArgumentException format:@"set must not be nil"];
    
    Vector3f* origin = [theRay origin];
    BOOL hit = fgte([origin x], [min x]) && fgte([origin y], [min y]) && fgte([origin z], [min z]) && flte([origin x], [max x]) && flte([origin y], [max y]) && flte([origin z], [max z]);
    
    MathCache* cache = [MathCache sharedCache];
    Plane3D* plane = [cache plane3D];
    Vector3f* intMin = [cache vector3f];
    Vector3f* intMax = [cache vector3f];
    [intMin setInt:min];
    [intMax setInt:max];
    
    Vector3f* direction = [theRay direction];
    
    if (!hit) {
        if ([direction x] > 0) {
            [plane setPoint:intMin norm:[Vector3f xAxisNeg]];
            Vector3f* is = [plane intersectWithRay:theRay];
            hit = is != nil && fgte([is y], [min y]) && flte([is y], [max y]) && fgte([is z], [min z]) && flte([is z], [max z]);
        } else if ([direction x] < 0) {
            [plane setPoint:intMax norm:[Vector3f xAxisPos]];
            Vector3f* is = [plane intersectWithRay:theRay];
            hit = is != nil && fgte([is y], [min y]) && flte([is y], [max y]) && fgte([is z], [min z]) && flte([is z], [max z]);
        }
    }
    
    if (!hit) {
        if ([direction y] > 0) {
            [plane setPoint:intMin norm:[Vector3f yAxisNeg]];
            Vector3f* is = [plane intersectWithRay:theRay];
            hit = is != nil && fgte([is x], [min x]) && flte([is x], [max x]) && fgte([is z], [min z]) && flte([is z], [max z]);
        } else if ([direction y] < 0) {
            [plane setPoint:intMax norm:[Vector3f yAxisPos]];
            Vector3f* is = [plane intersectWithRay:theRay];
            hit = is != nil && fgte([is x], [min x]) && flte([is x], [max x]) && fgte([is z], [min z]) && flte([is z], [max z]);
        }
    }
    
    if (!hit) {
        if ([direction z] > 0) {
            [plane setPoint:intMin norm:[Vector3f zAxisNeg]];
            Vector3f* is = [plane intersectWithRay:theRay];
            hit = is != nil && [is x] >= [min x] && [is x] <= [max x] && [is y] >= [min y] && [is y] <= [max y];
        } else if ([direction z] < 0) {
            [plane setPoint:intMax norm:[Vector3f zAxisPos]];
            Vector3f* is = [plane intersectWithRay:theRay];
            hit = is != nil && [is x] >= [min x] && [is x] <= [max x] && [is y] >= [min y] && [is y] <= [max y];
        }
    }

    [cache returnVector3f:intMax];
    [cache returnVector3f:intMin];
    [cache returnPlane3D:plane];
    
    if (hit) {
        [theSet unionSet:objects];
        for (int i = 0; i < 8; i++)
            if (children[i] != nil)
                [children[i] addObjectsForRay:theRay to:theSet];
    }
}

- (void)dealloc {
    for (int i = 0; i < 8; i++)
        if (children[i] != nil)
            [children[i] release];
    
    [min release];
    [max release];
    [objects release];
    [super dealloc];
}

@end
