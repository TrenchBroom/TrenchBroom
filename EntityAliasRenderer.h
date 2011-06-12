//
//  AliasRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOBuffer;
@protocol Entity;
@protocol Filter;

@interface EntityAliasRenderer : NSObject {
    VBOBuffer* vbo;
    NSMutableSet* entities;
    NSMutableDictionary* aliasRenderers;
    NSMutableDictionary* entityRenderers;
    NSData* palette;
    id <Filter> filter;
}

- (void)addEntity:(id <Entity>)entity;
- (void)removeEntity:(id <Entity>)entity;

- (void)render;

- (void)setFilter:(id <Filter>)theFilter;
@end
