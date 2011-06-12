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
#import "EntityAliasRenderer.h"
#import "Options.h"

@implementation SelectionLayer

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager options:(Options *)theOptions camera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    if (self = [super initWithVbo:theVbo textureManager:theTextureManager options:theOptions]) {
        entityBoundsRenderer = [[EntityBoundsRenderer alloc] init];
        entityAliasRenderer = [[EntityAliasRenderer alloc] init];
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

- (void)setTextureMode {
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

- (void)setFaceColorMode {
    glColor4f(1, 0, 0, 1);
}

- (BOOL)doRenderFaces {
    return YES;
}

- (BOOL)doRenderEdges {
    return YES;
}

- (void)addEntity:(id <Entity>)entity {
    [entityBoundsRenderer addEntity:entity];
    [entityAliasRenderer addEntity:entity];
}

- (void)removeEntity:(id <Entity>)entity {
    [entityBoundsRenderer removeEntity:entity];
    [entityAliasRenderer removeEntity:entity];
}

- (void)updateEntity:(id <Entity>)entity {
    [self removeEntity:entity];
    [self addEntity:entity];
}

- (void)render {
    [super render];
    
    if ([options renderEntities]) {
        glColor4f(1, 0, 0, 1);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        [entityAliasRenderer render];

        glDisable(GL_DEPTH_TEST);
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
    [entityAliasRenderer release];
    [super dealloc];
}

@end
