//
//  EntityLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Layer.h"

@class EntityDefinitionManager;
@class EntityBoundsRenderer;
@protocol Entity;

@interface EntityLayer : NSObject <Layer> {
    EntityBoundsRenderer* boundsRenderer;
}

- (void)addEntity:(id <Entity>)entity;
- (void)removeEntity:(id <Entity>)entity;
- (void)updateEntity:(id <Entity>)entity;

@end
