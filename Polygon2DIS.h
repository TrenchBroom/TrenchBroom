//
//  Polygon2DIntersection.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Edge2D;
@class Polygon2D;

typedef enum {
    P1U, P1L, P2U, P2L
} CEdge;

@interface Polygon2DIS : NSObject {
    @private
    Edge2D* polygon1UpperEdge;
    Edge2D* polygon1LowerEdge;
    Edge2D* polygon2UpperEdge;
    Edge2D* polygon2LowerEdge;
    
    Edge2D* firstUpperEdge;
    Edge2D* lastUpperEdge;
    Edge2D* firstLowerEdge;
    Edge2D* lastLowerEdge;
}

- (id)initWithPolygon1:(Polygon2D *)polygon1 polygon2:(Polygon2D *)polygon2;
- (Polygon2D *)intersection;
@end
