//
//  Polygon2D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polygon2D.h"
#import "Edge2D.h"
#import "Vector2f.h"

@implementation Polygon2D
- (id)initWithVertices:(NSArray *)vertices {
    if (vertices == nil)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must not be nil"];
    if ([vertices count] < 3)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must contain at least three vertices"];
    
    if (self = [super init]) {
        Vector2f* start = [vertices objectAtIndex:0];
        Vector2f* end = [vertices objectAtIndex:1];
        edges = [[Edge2D alloc] initWithStart:start end:end];
        Edge2D* current = edge;
        
        for (int i = 2; i < [vertices count]; i++) {
            start = end;
            end = [vertices objectAtIndex:i];
            current = [current insertAfterStart:start end:end];
        }
        
        [current close:edges];
    }
    
    return self;
}

- (id)initWithEdges:(Edge2D *)newEdges {
    if (newEdges == nil)
        [NSException raise:NSInvalidArgumentException format:@"edge list must not be nil"];

    if (self = [super init]) {
        edges = [newEdges retain];
    }
    
    return self;
}

- (NSArray *)vertices {
    NSMutableArray* vertices = [[NSMutableArray alloc] init];
    Edge2D* current = edges;
    do {
        [vertices addObject:[current startVertex]];
        current = [current next];
    } while (current != edges);
}

- (Edge2D *)edges {
    return edges;
}

- (void)dealloc {
    // open the polygon to avoid reference cycle
    [edges open];
    [edges release];
    [super dealloc];
}
@end
