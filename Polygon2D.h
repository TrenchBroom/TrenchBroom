//
//  Polygon2D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Edge2D.h"

@interface Polygon2D : NSObject {
    Edge2D* edges;
}

- (id)initWithVertices:(NSArray *)newVertices;
- (id)initWithEdges:(Edge2D *)edges;

- (Edge2D *)edges;
- (Edge2D *)upperEdges;
- (Edge2D *)lowerEdges;
- (NSArray *)vertices;

- (Polygon2D *)intersectWith:(Polygon2D *)polygon;

@end
