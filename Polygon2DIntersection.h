//
//  Polygon2DIntersection.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Polygon2D.h"
#import "Edge2D.h"

typedef enum {
    P1U, P1L, P2U, P2L
} CurrentEdge;

@interface Polygon2DIntersection : NSObject {
    Edge2D* p1u;
    Edge2D* p1l;
    Edge2D* p2u;
    Edge2D* p2l;
    
    Edge2D* fu;
    Edge2D* lu;
    Edge2D* fl;
    Edge2D* ll;
}

- (id)initWithPolygon1:(Polygon2D *)p1 polygon2:(Polygon2D *)p2;
- (Polygon2D *)intersection;

- (Edge2D *)forward:(Edge2D *)edge to:(float)x;
- (int)nextEvent;
- (void)addUpper:(Edge2D *)e;
- (void)addLower:(Edge2D *)e;
@end
