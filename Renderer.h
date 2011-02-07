//
//  Renderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Layer.h"

@class VBOBuffer;
@class RenderContext;
@class SelectionManager;

@interface Renderer : NSObject {
    @private
    VBOBuffer* vbo;
    NSObject<Layer>* geometryLayer;
    NSObject<Layer>* selectionLayer;
    NSObject<Layer>* handleLayer;
    SelectionManager* selectionManager;
}

- (id)initWithVbo:(VBOBuffer *)theVbo;

- (void)render:(RenderContext *)renderContext;

- (void)setSelectionManager:(SelectionManager *)theSelectionManager;

@end
