//
//  SelectionLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "SelectionLayer.h"
#import "Face.h"
#import "BoundsRenderer.h"

@implementation SelectionLayer

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager grid:(Grid *)theGrid camera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    if (self = [super initWithVbo:theVbo textureManager:theTextureManager grid:theGrid]) {
        boundsRenderer = [[BoundsRenderer alloc] initWithCamera:theCamera fontManager:theFontManager font:theFont];
    }
    
    return self;
}

- (void)addFace:(id <Face>)theFace {
    [super addFace:theFace];
    [boundsRenderer addBrush:[theFace brush]];
}

- (void)removeFace:(id <Face>)theFace {
    [super removeFace:theFace];
    [boundsRenderer removeBrush:[theFace brush]];
}

- (void)preRenderEdges {
    glColor4f(1, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
}

- (void)postRenderEdges {
    glEnable(GL_DEPTH_TEST);
}

- (void)render:(RenderContext *)renderContext {
    [super render:renderContext];
    glDisable(GL_DEPTH_TEST);
    [boundsRenderer render];
    glEnable(GL_DEPTH_TEST);
}

- (void)dealloc {
    [boundsRenderer release];
    [super dealloc];
}

@end
