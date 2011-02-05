//
//  Side.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "VertexData.h"

typedef enum {
    SM_KEEP,
    SM_DROP,
    SM_SPLIT,
    SM_NEW,
    SM_UNKNOWN
} ESideMark;

@class Edge;
@class SideEdge;
@class Vector3f;

@interface Side : NSObject {
    @private
    NSMutableArray* edges;
    ESideMark mark;
    NSMutableArray* vertices;
    Vector3f* center;
}

- (id)initWithEdges:(NSArray *)theEdges flipped:(BOOL*)flipped;
- (id)initWithSideEdges:(NSArray *)theEdges;

- (SideEdge *)split;

- (ESideMark)mark;
- (void)setMark:(ESideMark)theMark;

- (NSArray *)vertices;

/*!
    @function
    @abstract   Returns the center of this side.
    @result     The center of this side.
*/
- (Vector3f *)center;
@end
