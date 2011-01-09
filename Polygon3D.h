//
//  Polygon3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class HalfSpace3D;

@interface Polygon3D : NSObject {
    NSMutableArray* vertices;
}
- (id)init;
- (id)initWithVertices:(NSArray *)someVertices;

- (NSArray *)vertices;

- (BOOL)intersectWithHalfSpace:(HalfSpace3D *)halfSpace;

- (BOOL)isEqualToPolygon:(Polygon3D *)polygon;
@end
