//
//  PolygonRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOBuffer;
@class TextureManager;
@protocol PolygonFigure;
@protocol FigureFilter;

@interface PolygonRenderer : NSObject {
    @private
    NSMutableSet* figures;
    NSMutableDictionary* indexBuffers;
    NSMutableDictionary* countBuffers;
    id <FigureFilter> filter;
    VBOBuffer* vbo;
    TextureManager* textureManager;
    BOOL valid;
}

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager;

- (void)addFigure:(id <PolygonFigure>)theFigure;
- (void)removeFigure:(id <PolygonFigure>)theFigure;

- (void)setFilter:(id <FigureFilter>)theFilter;
- (void)renderTextured:(BOOL)textured;

- (void)invalidate;

@end
