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
    @private
    NSMutableArray* vertices;
}
+ (Polygon3D *)polygonWithVertices:(NSArray *)someVertices;

- (id)init;
- (id)initWithVertices:(NSArray *)someVertices;

- (NSArray *)vertices;

- (BOOL)isEqualToPolygon:(Polygon3D *)polygon;
@end
