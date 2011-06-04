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
#import "Options.h"

@implementation SelectionLayer

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager options:(Options *)theOptions camera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    if (self = [super initWithVbo:theVbo textureManager:theTextureManager options:theOptions]) {
        entityBoundsRenderer = [[EntityBoundsRenderer alloc] init];
    }
    
    return self;
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

- (void)render {
    [super render];
    
    if ([options renderEntities]) {
        glDisable(GL_DEPTH_TEST);
        glColor4f(1, 0, 0, 1);
        [entityBoundsRenderer renderWithColor:NO];
        glEnable(GL_DEPTH_TEST);
    }
}

- (void)setFilter:(id <Filter>)theFilter {
    [super setFilter:theFilter];
    [entityBoundsRenderer setFilter:theFilter];
}

- (void)dealloc {
    [entityBoundsRenderer release];
    [super dealloc];
}

@end
