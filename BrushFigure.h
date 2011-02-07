//
//  RenderBrush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Brush;
@class Face;
@class IntData;
@class VBOBuffer;
@class VBOMemBlock;
@class RenderContext;

@interface BrushFigure : NSObject {
    @private
    Brush* brush;
    VBOBuffer* vbo;
    VBOMemBlock* block;
    NSMutableDictionary* faceEntries;
}

- (id)initWithBrush:(Brush *)theBrush vbo:(VBOBuffer *)theVbo;

- (Brush *)brush;

- (void)prepare:(RenderContext *)renderContext;
- (void)indexForFace:(Face *)face indexBuffer:(IntData *)theIndexBuffer countBuffer:(IntData *)theCountBuffer;
@end
