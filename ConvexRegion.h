//
//  ConvexRegion.h
//  TrenchBroom
//
//  Created by Kristian Duske on 19.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3f.h"
#import "Line2D.h"

@interface ConvexRegion : NSObject {
    Vector2f *topVertex;
    Vector2f *bottomVertex;
    NSMutableArray *leftVertices;
    NSMutableArray *rightVertices;
    
    NSMutableArray* leftEdges;
    NSMutableArray* rightEdges;
}

- (id)initWithEdge:(Line2D *)edge;
- (void)addEdge:(Line2D *)edge;

- (int)leftEdgeAt:(float)y startingWith:(int)edgeIndex;
- (int)rightEdgeAt:(float)y startingWith:(int)edgeIndex;

- (Line2D *)leftEdgeAtIndex:(int)index;
- (Line2D *)rightEdgeAtIndex:(int)index;

- (Vector2f *)lowerBoundary:(int)edgeIndex edges:(NSArray *)edges vertices:(NSArray *)vertices;
- (Vector2f *)upperBoundary:(int)edgeIndex edges:(NSArray *)edges vertices:(NSArray *)vertices;

- (float)top;
@end

@interface ConvexRegion(private)

- (void)addLeftEdge:(Line2D *)edge;
- (void)addRightEdge:(Line2D *)edge;

- (int)edgeAt:(float)y edges:(NSArray *)edges vertices:(NSArray *)vertices startingWith:(int)edgeIndex;
@end
