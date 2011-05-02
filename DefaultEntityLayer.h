//
//  EntityLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "EntityLayer.h"

@class EntityDefinitionManager;
@class EntityBoundsRenderer;
@protocol Entity;

@interface DefaultEntityLayer : NSObject <EntityLayer> {
    EntityBoundsRenderer* boundsRenderer;
}

@end
