//
//  Renderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const RendererChanged;

@class VBOBuffer;
@class MapWindowController;
@class TextureManager;
@class GLFontManager;
@class RenderChangeSet;
@class EntityRendererManager;
@class TextRenderer;
@class BoundsRenderer;
@protocol EntityLayer;
@protocol Figure;
@protocol Filter;

@interface Renderer : NSObject {
    @private
    MapWindowController* windowController;
    TextureManager* textureManager;
    GLFontManager* fontManager;
    VBOBuffer* faceVbo;
    NSMutableDictionary* faceIndexBuffers;
    NSMutableDictionary* faceCountBuffers;
    NSMutableDictionary* selectedFaceIndexBuffers;
    NSMutableDictionary* selectedFaceCountBuffers;
    VBOBuffer* entityBoundsVbo;
    VBOBuffer* selectedEntityBoundsVbo;
    int entityBoundsVertexCount;
    int selectedEntityBoundsVertexCount;

    TextRenderer* classnameRenderer;
    TextRenderer* selectedClassnameRenderer;
    EntityRendererManager* entityRendererManager;
    NSMutableArray* modelEntities;
    NSMutableArray* selectedModelEntities;
    NSMutableDictionary* entityRenderers;
    BOOL entityRendererCacheValid;
    NSArray* mods;
    
    BoundsRenderer* selectionBoundsRenderer;
    
    RenderChangeSet* changeSet;
    NSMutableArray* feedbackFigures;
    id <Filter> filter;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (void)addFeedbackFigure:(id <Figure>)theFigure;
- (void)removeFeedbackFigure:(id <Figure>)theFigure;

- (void)render;

@end
