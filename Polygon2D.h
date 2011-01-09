//
//  Polygon2D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Edge2D;

@interface Polygon2D : NSObject {
@private
    Edge2D* edges;
}

+ (Polygon2D *)polygonWithVertices:(NSArray *)someVertices;
+ (Polygon2D *)polygonWithSortedEdges:(Edge2D *)someEdges;

- (id)initWithVertices:(NSArray *)someVertices;
- (id)initWithSortedEdges:(Edge2D *)someEdges;

- (Edge2D *)edges;
- (NSArray *)vertices;

- (Polygon2D *)intersectWith:(Polygon2D *)aPolygon;

@end
