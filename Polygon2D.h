//
//  Polygon2D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Edge2D.h"
#import "Vector2f.h"

@interface Polygon2D : NSObject {
@private
    Edge2D* edges;
}

- (id)initWithVertices:(NSArray *)someVertices;
- (id)initWithEdges:(Edge2D *)someEdges;

- (Edge2D *)edges;
- (NSArray *)vertices;

- (Polygon2D *)intersectWith:(Polygon2D *)aPolygon;

@end
