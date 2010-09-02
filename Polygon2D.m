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
- (id)initWithVertices:(NSArray *)newVertices {
    if (newVertices == nil)
        [NSException raise:NSInvalidArgumentException format:@"newVertices must not be nil"];
    if ([newVertices count] < 3)
        [NSException raise:NSInvalidArgumentException format:@"newVertices must contain at least three vertices"];
    
    if (self = [super init]) {
        int s = 0; // index of smallest vertex in newVertices array
        int l = 0; // index of largest vertex in vertices array
        int i;
        int c = [newVertices count];
        vertices = [[NSMutableArray alloc] initWithCapacity:c];
        
        // find index of smallest vertex
        for (i = 0; i < c; i++)
            if ([[newVertices objectAtIndex:i] lexicographicCompare:[newVertices objectAtIndex:s]] == NSOrderedAscending)
                s = i;
        
        // add vertices so that smallest vertex is first in array and find index of largest vertex
        for (i = 0; i < c; i++) {
            Vector2f* v = [newVertices objectAtIndex:(i + s) % c];
            [vertices addObject:v];
            if ([[vertices objectAtIndex:l] smallerThan:v])
                l = i;
        }

        Vector2f* start = [vertices objectAtIndex:0];
        Vector2f* end = [vertices objectAtIndex:1];
        firstLower = [[Edge2D alloc] initWithStart:start end:end];
        Edge2D* currentEdge = firstLower;
        for (i = 2; i < c; i++) {
            start = end;
            end = [vertices objectAtIndex:i];
            currentEdge = [currentEdge insertAfterStart:start end:end];
        }
        
        start = end;
        end = [vertices objectAtIndex:0];
        firstUpper = [currentEdge insertAfterStart:start end:end];
        
        // close polygon
        [firstUpper setNext:firstLower]; // firstLower now has retain count of 2
        [firstLower setPrevious:firstUpper];
    }
    
    return self;
}

- (NSArray *)vertices {
    return vertices;
}

- (NSArray *)upperEdges {
    return firstUpper;
}

- (NSArray *)lowerEdges {
    return firstLower;
}

- (void)dealloc {
    [firstUpper setNext:nil]; // reduces firstLower retain count by 1
    [[firstUpper previous] setNext:nil]; // reduces firstUpper retain count by 1
    [firstLower release];
    [vertices release];
    [super dealloc];
}
@end
