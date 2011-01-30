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

- (BOOL)contains:(BoundingBox *)theBounds {
    if ([[theBounds max] x] < [min x])
        return NO;
    if ([[theBounds max] y] < [min y])
        return NO;
    if ([[theBounds max] z] < [min z])
        return NO;
    if ([[theBounds min] x] > [max x])
        return NO;
    if ([[theBounds min] y] > [max y])
        return NO;
    if ([[theBounds min] z] > [max z])
        return NO;
    
    return YES;
}

- (BOOL)addObject:(id)theObject bounds:(BoundingBox *)theBounds toChild:(int)theIndex {
    if (children[theIndex] != nil)
        return [children[theIndex] addObject:theObject bounds:theBounds];

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
    
    if ([[theBounds min] x] < [childMin x])
        return NO;
    if ([[theBounds min] y] < [childMin y])
        return NO;
    if ([[theBounds min] z] < [childMin z])
        return NO;
    if ([[theBounds max] x] > [childMax x])
        return NO;
    if ([[theBounds max] y] > [childMax y])
        return NO;
    if ([[theBounds max] z] > [childMax z])
        return NO;

    children[theIndex] = [[OctreeNode alloc] initWithMin:childMin max:childMax minSize:minSize];
    [childMin release];
    [childMax release];
    
    return [children[theIndex] addObject:theObject bounds:theBounds];
}

- (BOOL)addObject:(id)theObject bounds:(BoundingBox *)theBounds {
    if (theObject == nil)
        [NSException raise:NSInvalidArgumentException format:@"object must not be nil"];
    if (theBounds == nil)
        [NSException raise:NSInvalidArgumentException format:@"bounds must not be nil"];
    
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
    
    if (![self contains:theBounds])
        return NO;
    
    for (int i = 0; i < 8; i++)
        if (children[i] != nil && [children[i] addObject:theObject bounds:theBounds])
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
    BOOL hit = [origin x] >= [min x] && [origin y] >= [min y] && [origin z] >= [min z] && [origin x] <= [max x] && [origin y] <= [max y] && [origin z] <= [max z];
    
    Plane3D* plane = [[Plane3D alloc] init];
    Vector3f* direction = [theRay direction];
    
    if (!hit) {
        if ([direction x] > 0) {
            [plane setPoint:[Vector3f vectorWithIntVector:min] norm:[Vector3f xAxisNeg]];
            Vector3f* is = [plane intersectWithRay:theRay];
            hit = is != nil && [is y] >= [min y] && [is y] <= [max y] && [is z] >= [min z] && [is z] <= [max z];
        } else if ([direction x] < 0) {
            [plane setPoint:[Vector3f vectorWithIntVector:max] norm:[Vector3f xAxisPos]];
            Vector3f* is = [plane intersectWithRay:theRay];
            hit = is != nil && [is y] >= [min y] && [is y] <= [max y] && [is z] >= [min z] && [is z] <= [max z];
        }
    }
    
    if (!hit) {
        if ([direction y] > 0) {
            [plane setPoint:[Vector3f vectorWithIntVector:min] norm:[Vector3f yAxisNeg]];
            Vector3f* is = [plane intersectWithRay:theRay];
            hit = is != nil && [is x] >= [min x] && [is x] <= [max x] && [is z] >= [min z] && [is z] <= [max z];
        } else if ([direction y] < 0) {
            [plane setPoint:[Vector3f vectorWithIntVector:max] norm:[Vector3f yAxisPos]];
            Vector3f* is = [plane intersectWithRay:theRay];
            hit = is != nil && [is x] >= [min x] && [is x] <= [max x] && [is z] >= [min z] && [is z] <= [max z];
        }
    }
     
    if (!hit) {
        if ([direction z] > 0) {
            [plane setPoint:[Vector3f vectorWithIntVector:min] norm:[Vector3f zAxisNeg]];
            Vector3f* is = [plane intersectWithRay:theRay];
            hit = is != nil && [is x] >= [min x] && [is x] <= [max x] && [is y] >= [min y] && [is y] <= [max y];
        } else if ([direction z] < 0) {
            [plane setPoint:[Vector3f vectorWithIntVector:max] norm:[Vector3f zAxisPos]];
            Vector3f* is = [plane intersectWithRay:theRay];
            hit = is != nil && [is x] >= [min x] && [is x] <= [max x] && [is y] >= [min y] && [is y] <= [max y];
        }
    }
    [plane release];
    
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
