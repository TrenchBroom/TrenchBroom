//
//  Polygon3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "HalfSpace3D.h"
#import "Segment3D.h"

@interface Polygon3D : NSObject {
    NSMutableArray* vertices;
}
- (id)init;
- (id)initWithVertices:(NSArray *)someVertices;

- (Segment3D *)intersectWith:(HalfSpace3D *)halfSpace;
@end
