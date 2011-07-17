//
//  BspFace.m
//  TrenchBroom
//
//  Created by Kristian Duske on 06.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BspFace.h"


@implementation BspFace

- (id)initWithTextureInfo:(TTextureInfo *)theTextureInfo vertices:(TVector3f *)theVertices vertexCount:(int)theVertexCount {
    NSAssert(theTextureInfo != NULL, @"texture info must not be NULL");
    NSAssert(theVertices != NULL, @"vertex array must not be NULL");
    NSAssert(theVertexCount >= 3, @"vertex count must be at least 3");
    
    if ((self = [self init])) {
        textureInfo = *theTextureInfo;
        vertexCount = theVertexCount;
        vertices = malloc(vertexCount * sizeof(TVector3f));
        memcpy(vertices, theVertices, vertexCount * sizeof(TVector3f));
        
        bounds.min = vertices[0];
        bounds.max = vertices[0];
        
        for (int i = 1; i < vertexCount; i++)
            mergeBoundsWithPoint(&bounds, &vertices[i], &bounds);
    }
    
    return self;
}

- (TBoundingBox *)bounds {
    return &bounds;
}

- (TTextureInfo *)textureInfo {
    return &textureInfo;
}

- (TVector3f *)vertices {
    return vertices;
}

- (int)vertexCount {
    return vertexCount;
}

- (void)texCoords:(TVector2f *)theTexCoords forVertex:(TVector3f *)theVertex {
    NSAssert(theTexCoords != NULL, @"texture coordinate vector must not be NULL");
    NSAssert(theVertex != NULL, @"vertex must not be nil");
    
    theTexCoords->x = (dotV3f(theVertex, &textureInfo.sAxis) + textureInfo.sOffset) / [textureInfo.texture width];
    theTexCoords->y = (dotV3f(theVertex, &textureInfo.tAxis) + textureInfo.tOffset) / [textureInfo.texture height];
}

- (void)dealloc {
    free(vertices);
    [super dealloc];
}

@end
