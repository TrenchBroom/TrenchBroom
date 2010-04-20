//
//  Math3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 19.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Math3D.h"
#import <math.h>

float const AlmostZero = 0.0000001f;

@implementation Math3D(private)

+(ConvexRegion *)intersectConvexRegions:(ConvexRegion *)c1 and:(ConvexRegion *)c2 {
    
    float y1 = [c1 top];
    float y2 = [c2 top];
    
    float y = fmaxf(y1, y2);
    
    int c1l = [c1 leftEdgeAt:y startingWith:0];
    int c1r = [c1 rightEdgeAt:y startingWith:0];
    int c2l = [c2 leftEdgeAt:y startingWith:0];
    int c2r = [c2 rightEdgeAt:y startingWith:0];

    
}

@end

@implementation Math3D

+(ConvexRegion *)intersectHalfPlanes:(NSArray *)h {
    int n = [h count];
    if (n == 1) {
        Line2D* e = [h lastObject];
        ConvexRegion* c = [[ConvexRegion alloc] initWithEdge:e];
        return [c autorelease];
    }
    
    int d = n / 2;
    NSArray* h1 = [h subarrayWithRange:NSMakeRange(0, d)];
    NSArray* h2 = [h subarrayWithRange:NSMakeRange(d, n - d)];
    
    ConvexRegion* c1 = [self intersectHalfPlanes:h1];
    ConvexRegion* c2 = [self intersectHalfPlanes:h2];
    
    return [self intersectConvexRegions:c1 and:c2];
}

@end
