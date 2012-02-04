/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Cocoa/Cocoa.h>
#import <OpenGL/glu.h>

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

    GLUquadric* vertexHandle;
    
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
