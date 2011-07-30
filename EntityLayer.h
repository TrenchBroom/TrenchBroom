//
//  EntityLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 01.05.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "Layer.h"

@protocol Entity;
@protocol Filter;

@protocol EntityLayer <Layer>

- (void)addEntity:(id <Entity>)entity;
- (void)removeEntity:(id <Entity>)entity;
- (void)updateEntity:(id <Entity>)entity;

- (void)setFilter:(id <Filter>)theFilter;
- (void)setMods:(NSArray *)theMods;
- (void)refreshRendererCache;

@end
