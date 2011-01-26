//
//  VertexData2.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    BM_KEEP,
    BM_DROP,
    BM_SPLIT
} EBrushMark;

@class Face;

@interface VertexData : NSObject {
    @private
    NSMutableArray* vertices;
    NSMutableArray* edges;
    NSMutableArray* sides;
    NSMutableArray* sideToFace;
    NSMutableDictionary* faceToSide;
}

- (BOOL)cutWithFace:(Face *)face droppedFaces:(NSMutableArray **)droppedFaces;
- (NSArray*)verticesForFace:(Face *)face;
@end
