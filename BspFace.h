//
//  BspFace.h
//  TrenchBroom
//
//  Created by Kristian Duske on 06.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Bsp.h"

@interface BspFace : NSObject {
@private
    TBoundingBox bounds;
    TTextureInfo textureInfo;
    TVector3f* vertices;
    int vertexCount;
}

- (id)initWithTextureInfo:(TTextureInfo *)theTextureInfo vertices:(TVector3f *)theVertices vertexCount:(int)theVertexCount;

- (TBoundingBox *)bounds;
- (TTextureInfo *)textureInfo;
- (TVector3f *)vertices;
- (int)vertexCount;
- (void)texCoords:(TVector2f *)theTexCoords forVertex:(TVector3f *)theVertex;

@end
