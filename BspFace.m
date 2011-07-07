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
    }
    
    return self;
}

- (TTextureInfo *)textureInfo {
    return &textureInfo;
}

- (TVector3f *)vertexAtIndex:(int)theIndex {
    NSAssert(theIndex >= 0 && theIndex < vertexCount, @"vertex index out of bounds");
    return &vertices[theIndex];
}

- (int)vertexCount {
    return vertexCount;
}

- (void)texCoords:(TVector2f *)theTexCoords forVertex:(TVector3f *)theVertex {
    NSAssert(theTexCoords != NULL, @"texture coordinate vector must not be NULL");
    NSAssert(theVertex != NULL, @"vertex must not be nil");
    
    theTexCoords->x = dotV3f(theVertex, &textureInfo.sAxis) + textureInfo.sOffset;
    theTexCoords->y = dotV3f(theVertex, &textureInfo.tAxis) + textureInfo.tOffset;
}

- (void)dealloc {
    free(vertices);
    [super dealloc];
}

@end
