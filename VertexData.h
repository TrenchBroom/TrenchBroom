//
//  VertexData.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    OS_KEEP, // object is contained, keep it
    OS_SPLIT, // object is split by plane
    OS_DROP // object is not contained, drop it
} EObjectStatus;

@class Face;

@interface VertexData : NSObject {
    NSMutableArray* vertices;
    NSMutableArray* edges;
    NSMutableArray* sides;
    NSMutableDictionary* faceToSide;
    NSMutableArray* sideToFace;
    NSMutableArray* freeVertexSlots;
    NSMutableArray* freeEdgeSlots;
    NSMutableArray* freeSideSlots;
    NSObject* null;
}

- (BOOL)cutWithFace:(Face *)face droppedFaces:(NSMutableArray **)droppedFaces;
- (NSArray *)verticesForFace:(Face *)face;

@end
