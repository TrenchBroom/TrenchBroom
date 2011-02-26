//
//  FaceFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Face;
@class VBOBuffer;
@class VBOMemBlock;
@class IntData;
@class RenderContext;

@interface FaceFigure : NSObject {
    @private
    Face* face;
    VBOBuffer* vbo;
    VBOMemBlock* block;
    int vboIndex;
    int vboCount;
}

- (id)initWithFace:(Face *)theFace vbo:(VBOBuffer *)theVbo;

- (Face *)face;
- (void)prepare:(RenderContext *)renderContext;
- (void)getIndex:(IntData *)theIndexBuffer count:(IntData *)theCountBuffer;

- (void)invalidate;
@end
