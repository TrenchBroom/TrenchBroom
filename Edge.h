//
//  Edge.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Line.h"

@interface Edge : NSObject {
    Edge* previous;
    Edge* next;
    Line* line;
    Vector3f* startVertex;
}

- (id)initWithLine:(Line *)l;
- (id)initWithLine:(Line *)l previous:(Edge *)p;
- (id)initWithLine:(Line *)l next:(Edge *)n;
- (id)initWithLine:(Line *)l previous:(Edge *)p next:(Edge *)n;

- (Vector3f *)startVertex;
- (Vector3f *)endVertex;
- (Line *)line;

- (Edge *)insertAfter:(Line *)l until:(Edge *)e;
- (Edge *)insertBefore:(Line *)l;
- (Edge *)replaceWith:(Line *)l;

- (void)setPrevious:(Edge *)p;
- (void)setNext:(Edge *)n;

- (Edge *)previous;
- (Edge *)next;

- (Vector3f *)intersectionWithLine:(Line *)l;

@end
