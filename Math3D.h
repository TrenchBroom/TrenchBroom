//
//  Math3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 19.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "ConvexRegion.h"
#import "Line2D.h"

extern float const AlmostZero;

@interface Math3D : NSObject {

}

+ (ConvexRegion *)intersectHalfPlanes:(NSArray *)halfPlanes;

@end

@interface Math3D(private)

+ (ConvexRegion *)intersectConvexRegions:(ConvexRegion *)c1 and:(ConvexRegion *)c2;

@end