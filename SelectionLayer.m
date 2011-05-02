//
//  SelectionLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "SelectionLayer.h"
#import "Face.h"
#import "Entity.h"
#import "BrushBoundsRenderer.h"
#import "EntityBoundsRenderer.h"

@implementation SelectionLayer

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager options:(Options *)theOptions camera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    if (self = [super initWithVbo:theVbo textureManager:theTextureManager options:theOptions]) {
        brushBoundsRenderer = [[BrushBoundsRenderer alloc] initWithCamera:theCamera fontManager:theFontManager font:theFont];
        entityBoundsRenderer = [[EntityBoundsRenderer alloc] init];
    }
    
    return self;
}

- (void)addFace:(id <Face>)theFace {
    [super addFace:theFace];
    [brushBoundsRenderer addBrush:[theFace brush]];
}

- (void)removeFace:(id <Face>)theFace {
    [super removeFace:theFace];
    [brushBoundsRenderer removeBrush:[theFace brush]];
}

- (void)preRenderEdges {
    glColor4f(1, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
}

- (void)postRenderEdges {
    glEnable(GL_DEPTH_TEST);
}

- (BOOL)doRenderFaces {
    return YES;
}

- (BOOL)doRenderEdges {
    return YES;
}

- (void)addEntity:(id <Entity>)entity {
    [entityBoundsRenderer addEntity:entity];
}

- (void)removeEntity:(id <Entity>)entity {
    [entityBoundsRenderer removeEntity:entity];
}

- (void)updateEntity:(id <Entity>)entity {
    [entityBoundsRenderer removeEntity:entity];
    [entityBoundsRenderer addEntity:entity];
}

- (void)render:(RenderContext *)renderContext {
    [super render:renderContext];
    glDisable(GL_DEPTH_TEST);
    [brushBoundsRenderer render];
    glColor4f(1, 0, 0, 1);
    [entityBoundsRenderer renderWithColor:NO];
    glEnable(GL_DEPTH_TEST);
}

- (void)setFilter:(id <Filter>)theFilter {
    [super setFilter:theFilter];
    [entityBoundsRenderer setFilter:theFilter];
}

- (void)dealloc {
    [brushBoundsRenderer release];
    [entityBoundsRenderer release];
    [super dealloc];
}

@end
