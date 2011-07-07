//
//  BspFace.h
//  TrenchBroom
//
//  Created by Kristian Duske on 06.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "BspModel.h"

@interface BspFace : NSObject {
@private
    TTextureInfo textureInfo;
    TVector3f* vertices;
    int vertexCount;
}

- (id)initWithTextureInfo:(TTextureInfo *)theTextureInfo vertices:(TVector3f *)theVertices vertexCount:(int)theVertexCount;

@end
