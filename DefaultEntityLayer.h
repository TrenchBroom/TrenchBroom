//
//  EntityLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "EntityLayer.h"

@class EntityBoundsRenderer;
@class EntityAliasRenderer;
@class TextRenderer;
@class Options;
@class GLResources;
@class GLFontManager;
@class Camera;

@interface DefaultEntityLayer : NSObject <EntityLayer> {
    Options* options;
    GLFontManager* fontManager;
    EntityBoundsRenderer* boundsRenderer;
    EntityAliasRenderer* aliasRenderer;
    TextRenderer* classnameRenderer;
    
    NSMutableSet* addedEntities;
    NSMutableSet* removedEntities;
}

- (id)initWithGLResources:(GLResources *)theGLResources camera:(Camera *)theCamera options:(Options *)theOptions;

@end
