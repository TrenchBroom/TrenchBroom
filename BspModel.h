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
    NSArray* faces;
    int vertexCount;
}

- (id)initWithFaces:(NSArray *)theFaces vertexCount:(int)theVertexCount;

- (NSArray *)faces;
- (int)vertexCount;

@end
