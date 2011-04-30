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

@interface EntityBoundsRenderer : NSObject {
    VBOBuffer* quads;
    NSMutableSet* addedEntities;
    NSMutableSet* removedEntities;
}

- (void)addEntity:(id <Entity>)entity;
- (void)removeEntity:(id <Entity>)entity;

- (void)render;
@end
