//
//  FaceHandleRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 27.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOBuffer;
@protocol Face;

@interface FaceHandleRenderer : NSObject {
    NSMutableSet* faces;
    VBOBuffer* vbo;
    int vertexCount;
    BOOL valid;
}

- (void)addFace:(id <Face>)theFace;
- (void)removeFace:(id <Face>)theFace;

- (void)render;

- (void)invalidate;

@end
