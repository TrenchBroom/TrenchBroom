//
//  Polygon3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class HalfSpace3D;
@class Vector3f;

@interface Polygon3D : NSObject {
    @private
    NSMutableArray* vertices;
    Vector3f* norm;
}
+ (Polygon3D *)polygonWithVertices:(NSArray *)someVertices norm:(Vector3f *)aNorm;
+ (Polygon3D *)polygonWithVertices:(NSArray *)someVertices;

- (id)initWithVertices:(NSArray *)someVertices norm:(Vector3f *)aNorm;
- (id)initWithVertices:(NSArray *)someVertices;

- (NSArray *)vertices;
- (Vector3f *)norm;

- (BOOL)isEqualToPolygon:(Polygon3D *)polygon;
@end
