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
#import "GLUtils.h"

@implementation SelectionLayer

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager options:(Options *)theOptions camera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    if ((self = [super initWithVbo:theVbo textureManager:theTextureManager options:theOptions])) {
        entityBoundsRenderer = [[EntityBoundsRenderer alloc] init];
        entityAliasRenderer = [[EntityAliasRenderer alloc] init];
    }
    
    return self;
}

- (void)preRenderEdges {
    if (edgePass == 1) {
        glSetEdgeOffset(0.5);
        glColor4f(1, 0, 0, 0.2f);
        glDisable(GL_DEPTH_TEST);
    } else if (edgePass == 2) {
        glSetEdgeOffset(1);
        glColor4f(1, 0, 0, 1);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }
}

- (void)postRenderEdges {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glResetEdgeOffset();
}

- (void)setTextureMode {
//    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

- (void)setFaceColorMode {
//    glColor4f(1, 0, 0, 1);
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
    edgePass = 1;
    [super render];
    
    [sharedVbo activate];
    edgePass = 2;
    [super renderEdges];
    [sharedVbo deactivate];
    
    if ([options renderEntities]) {
//        glColor4f(1, 0, 0, 1);
//        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        [entityAliasRenderer render];

        glSetEdgeOffset(0.5);
        glColor4f(1, 0, 0, 0.2f);
        glDisable(GL_DEPTH_TEST);
        [entityBoundsRenderer renderWithColor:NO];

        glSetEdgeOffset(1);
        glColor4f(1, 0, 0, 1);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        [entityBoundsRenderer renderWithColor:NO];

        glDepthFunc(GL_LESS);
        glResetEdgeOffset();
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
