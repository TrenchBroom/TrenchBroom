//
//  Polygon3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polygon3D.h"
#import "HalfSpace3D.h"
#import "Plane3D.h"
#import "Line3D.h"
#import "Math3D.h"
#import "Vector3f.h"

@implementation Polygon3D
+ (Polygon3D *)polygonWithVertices:(NSArray *)someVertices {
    return [[[Polygon3D alloc] initWithVertices:someVertices] autorelease];
}

- (id)init {
    if (self == [super init])
        vertices = [[NSMutableArray alloc] init];

    return self;
}

- (id)initWithVertices:(NSArray *)someVertices {
    if (someVertices == nil)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must not be nil"];
    if ([someVertices count] < 3)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must contain at least three vertices"];
        
        
    if (self = [super init]) {
        int si = smallestVertex(someVertices);
        if (si == 0) {
            vertices = [[NSMutableArray alloc] initWithArray:someVertices];
        } else {
            int c = [someVertices count];
            vertices = [[NSMutableArray alloc] initWithCapacity:c];
            int i;
            for (i = 0; i < c; i++)
                [vertices addObject:[someVertices objectAtIndex:(i + si) % c]];
        }
    }
    
    return self;
}

- (NSArray *)vertices {
    return vertices;
}

- (BOOL)isEqualToPolygon:(Polygon3D *)polygon {
    if ([self isEqual:polygon])
        return YES;
    
    NSArray* otherVertices = [polygon vertices];
    if ([vertices count] != [otherVertices count])
        return NO;
    
    for (int i = 0; i < [vertices count]; i++) {
        Vector3f* vertex = [vertices objectAtIndex:i];
        Vector3f* otherVertex = [otherVertices objectAtIndex:i];
        
        if (![vertex isEqualToVector:otherVertex])
            return NO;
    }
    
    return YES;
}

- (NSString *)description {
    NSMutableString* descr = [NSMutableString string];
    NSEnumerator* vertexEn = [vertices objectEnumerator];
    Vector3f* vertex;
    while ((vertex = [vertexEn nextObject]))
        [descr appendFormat:@"\n  %@", [vertex description]];
    return descr;
}

- (void)dealloc {
    [vertices release];
    [super dealloc];
}
@end
