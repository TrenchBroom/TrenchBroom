//
//  EntityBoundsRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOBuffer;
@protocol Entity;
@protocol Filter;

@interface EntityBoundsRenderer : NSObject {
    VBOBuffer* quads;
    NSMutableSet* entities;
    BOOL valid;
    id <Filter> filter;
}

- (void)addEntity:(id <Entity>)entity;
- (void)removeEntity:(id <Entity>)entity;

- (void)renderWithColor:(BOOL)doRenderWithColor;

- (void)setFilter:(id <Filter>)theFilter;
@end
