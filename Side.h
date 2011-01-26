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

@interface Side : NSObject {
    @private
    NSMutableArray* edges;
    ESideMark mark;
}

- (id)initWithEdges:(NSArray *)theEdges flipped:(BOOL*)flipped;
- (id)initWithSideEdges:(NSArray *)theEdges;

- (SideEdge *)split;

- (ESideMark)mark;
- (void)setMark:(ESideMark)theMark;

- (NSArray *)vertices;
@end
