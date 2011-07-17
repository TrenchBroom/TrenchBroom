//
//  Bsp.h
//  TrenchBroom
//
//  Created by Kristian Duske on 06.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Math.h"

@interface BspModel : NSObject {
@private
    TVector3f center;
    TBoundingBox bounds;
    TBoundingBox maxBounds;
    NSArray* faces;
    int vertexCount;
}

- (id)initWithFaces:(NSArray *)theFaces vertexCount:(int)theVertexCount center:(TVector3f *)theCenter bounds:(TBoundingBox *)theBounds maxBounds:(TBoundingBox *)theMaxBounds;

- (const TVector3f *)center;
- (const TBoundingBox *)bounds;
- (const TBoundingBox *)maxBounds;
- (NSArray *)faces;
- (int)vertexCount;

@end
