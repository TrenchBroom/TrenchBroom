//
//  EntityRendererManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@class VBOBuffer;
@class EntityDefinition;
@protocol EntityRenderer;
@protocol Entity;

@interface EntityRendererManager : NSObject {
@private
    NSMutableDictionary* entityRenderers;
    VBOBuffer* vbo;
    NSData* palette;
}

- (id)initWithPalette:(NSData *)thePalette;

- (id <EntityRenderer>)entityRendererForDefinition:(EntityDefinition *)theDefinition;
- (id <EntityRenderer>)entityRendererForEntity:(id <Entity>)theEntity;

- (void)activate;
- (void)deactivate;

@end
