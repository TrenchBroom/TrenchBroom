//
//  LineRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "EdgeRenderer.h"

@class VBOBuffer;
@protocol RenderFilter;

@interface ThinEdgeRenderer : NSObject <EdgeRenderer> {
    NSMutableSet* edges;
    int vertexCount;
    VBOBuffer* vbo;
    BOOL valid;
    id <RenderFilter> filter;
}

@end
