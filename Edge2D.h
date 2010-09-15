//
//  Edge2D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"
#import "Line2D.h"
#import "Vector2f.h"

@interface Edge2D : NSObject {
    @private
    Edge2D* previous;
    Edge2D* next;
    Vector2f* sVertex;
    Vector2f* outsideVector;
    Line2D* boundaryLine;
}

+ (Edge2D *)edgeWithBoundary:(Line2D *)boundary outside:(Vector2f *)outside;
+ (Edge2D *)edgeWithBoundary:(Line2D *)boundary previous:(Edge2D *)previousEdge outside:(Vector2f *)outside;
+ (Edge2D *)edgeWithBoundary:(Line2D *)boundary next:(Edge2D *)nextEdge outside:(Vector2f *)outside;
+ (Edge2D *)edgeWithBoundary:(Line2D *)boundary previous:(Edge2D *)previousEdge next:(Edge2D *)nextEdge outside:(Vector2f *)outside;
+ (Edge2D *)edgeWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex;
+ (Edge2D *)edgeWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex previous:(Edge2D *)previousEdge;
+ (Edge2D *)edgeWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex next:(Edge2D *)nextEdge;
+ (Edge2D *)edgeWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex previous:(Edge2D *)previousEdge next:(Edge2D *)nextEdge;

- (id)initWithBoundary:(Line2D *)boundary outside:(Vector2f *)outside;
- (id)initWithBoundary:(Line2D *)boundary previous:(Edge2D *)previousEdge outside:(Vector2f *)outside;
- (id)initWithBoundary:(Line2D *)boundary next:(Edge2D *)nextEdge outside:(Vector2f *)outside;
- (id)initWithBoundary:(Line2D *)boundary previous:(Edge2D *)previousEdge next:(Edge2D *)nextEdge outside:(Vector2f *)outside;
- (id)initWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex;
- (id)initWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex previous:(Edge2D *)previousEdge;
- (id)initWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex next:(Edge2D *)nextEdge;
- (id)initWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex previous:(Edge2D *)previousEdge next:(Edge2D *)nextEdge;

- (Vector2f *)startVertex;
- (Vector2f *)endVertex;
- (Vector2f *)smallVertex;
- (Vector2f *)largeVertex;
- (Vector2f *)outside;
- (Line2D *)boundary;

- (BOOL)isUpper;
- (BOOL)isLower;

- (BOOL)contains:(Vector2f *)point;

- (Vector2f *)intersectWith:(Edge2D *)edge;

- (void)setPrevious:(Edge2D *)previousEdge;
- (void)setNext:(Edge2D *)nextEdge;
- (void)open;
- (void)close:(Edge2D *)edge;

- (Edge2D *)previous;
- (Edge2D *)next;

- (Edge2D *)appendEdgeWithBoundary:(Line2D *)boundary outside:(Vector2f *)outside;
- (Edge2D *)appendEdgeWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex;
- (Edge2D *)prependEdgeWithBoundary:(Line2D *)boundary outside:(Vector2f *)outside;
- (Edge2D *)prependEdgeWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex;
    
@end
