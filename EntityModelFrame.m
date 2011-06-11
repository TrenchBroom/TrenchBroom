//
//  EntityModelFrame.m
//  TrenchBroom
//
//  Created by Kristian Duske on 10.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityModelFrame.h"

@implementation EntityModelFrame

- (id)initWithName:(NSString *)theName triangles:(TFrameTriangle *)theTriangles triangleCount:(int)theTriangleCount {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theTriangles != NULL, @"triangle array must not be NULL");
    NSAssert(triangleCount > 0, @"triangle count must be greater than 0");
    
    if (self = [self init]) {
        name = [[NSString alloc] initWithString:theName];
        triangleCount = theTriangleCount;
        triangles = malloc(triangleCount * sizeof(TFrameTriangle));
        memcpy(triangles, theTriangles, triangleCount * sizeof(TFrameTriangle));
        
        bounds.min = triangles[0].vertices[0].position;
        bounds.max = bounds.min;
        
        for (int i = 0; i < triangleCount; i++)
            for (int j = 0; j < 3; j++)
                mergeBoundsWithPoint(&bounds, &triangles[i].vertices[j].position, &bounds);
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    free(triangles);
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (int)triangleCount {
    return triangleCount;
}

- (const TFrameTriangle *)triangleAtIndex:(int)theIndex {
    NSAssert(theIndex >= 0 && theIndex < triangleCount, @"index out of bounds");
    return &triangles[theIndex];
}

@end
