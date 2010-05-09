//
//  ConvexVolume.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3f.h"
#import "Plane.h"

@interface ConvexVolume : NSObject {
    
    NSMutableSet* planes;
    NSMutableSet* vertices;
    NSMutableDictionary *edgesPerPlane;
    NSMutableDictionary *verticesPerPlane;
}

- (void)addPlane:(Plane *)plane;

@end
