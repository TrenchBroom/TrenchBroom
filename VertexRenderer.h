//
//  VertexRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

@class VBOBuffer;
@protocol Brush;

@interface VertexRenderer : NSObject {
    VBOBuffer* vbo;
    NSMutableDictionary* blocks;
    NSMutableSet* addedBrushes;
    NSMutableSet* removedBrushes;
    float handleSize;
}

- (id)initWithHandleSize:(float)theHandleSize;

- (void)addBrushes:(NSSet *)theBrushes;
- (void)removeBrushes:(NSSet *)theBrushes;

- (void)addBrush:(id <Brush>)theBrush;
- (void)removeBrush:(id <Brush>)theBrush;

- (void)render;

@end
