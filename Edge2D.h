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
    Edge2D* previous;
    Edge2D* next;
    Vector2f* startVertex;
    Vector2f* norm;
    Line2D* line;
}

- (id)initWithLine:(Line2D *)l norm:(Vector2f *)o;
- (id)initWithLine:(Line2D *)l previous:(Edge2D *)p norm:(Vector2f *)o;
- (id)initWithLine:(Line2D *)l next:(Edge2D *)n norm:(Vector2f *)o;
- (id)initWithLine:(Line2D *)l previous:(Edge2D *)p next:(Edge2D *)n norm:(Vector2f *)o;
- (id)initWithStart:(Vector2f *)s end:(Vector2f *)e;
- (id)initWithStart:(Vector2f *)s end:(Vector2f *)e previous:(Edge2D *)p;
- (id)initWithStart:(Vector2f *)s end:(Vector2f *)e next:(Edge2D *)n;
- (id)initWithStart:(Vector2f *)s end:(Vector2f *)e previous:(Edge2D *)p next:(Edge2D *)n;

- (Vector2f *)startVertex;
- (Vector2f *)endVertex;
- (Vector2f *)smallVertex;
- (Vector2f *)largeVertex;
- (Vector2f *)norm;
- (Line2D *)line;

- (BOOL)isUpper;
- (BOOL)isLower;

- (BOOL)containsX:(float)x;
- (BOOL)contains:(Vector2f *)p;

- (Vector2f *)intersectWith:(Edge2D *)e;

- (void)setPrevious:(Edge2D *)p;
- (void)setNext:(Edge2D *)n;
- (void)open;
- (void)close:(Edge2D *)n;

- (Edge2D *)previous;
- (Edge2D *)next;

- (Edge2D *)insertAfterLine:(Line2D *)l norm:(Vector2f *)o;
- (Edge2D *)insertAfterStart:(Vector2f *)s end:(Vector2f *)e;
- (Edge2D *)insertBeforeLine:(Line2D *)l norm:(Vector2f *)o;
- (Edge2D *)insertBeforeStart:(Vector2f *)s end:(Vector2f *)e;
    
@end
