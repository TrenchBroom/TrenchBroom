//
//  Renderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Observable.h"
#import "Layer.h"

extern NSString* const RendererChanged;

@class Map;
@class VBOBuffer;
@class RenderContext;
@class SelectionManager;
@class Camera;

@interface Renderer : Observable {
    @private
    Map* map;
    VBOBuffer* vbo;
    NSObject<Layer>* geometryLayer;
    NSObject<Layer>* selectionLayer;
    NSObject<Layer>* handleLayer;
    SelectionManager* selectionManager;
    Camera* camera;
}

- (id)initWithMap:(Map *)theMap vbo:(VBOBuffer *)theVbo;

- (void)render:(RenderContext *)renderContext;
- (void)updateView:(NSRect)bounds;

- (void)setSelectionManager:(SelectionManager *)theSelectionManager;
- (void)setCamera:(Camera *)theCamera;

@end
