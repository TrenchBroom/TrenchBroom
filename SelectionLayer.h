//
//  SelectionLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "GeometryLayer.h"
#import "DefaultEntityLayer.h"

@class TextRenderer;
@class BoundsRenderer;
@class Camera;
@class GLFontManager;

@interface SelectionLayer : GeometryLayer <EntityLayer> {
    NSMutableSet* addedEntities;
    NSMutableSet* removedEntities;
    BoundsRenderer* brushBoundsRenderer;
    EntityBoundsRenderer* entityBoundsRenderer;
    EntityAliasRenderer* entityAliasRenderer;
    TextRenderer* entityClassnameRenderer;
    int edgePass;
    GLFontManager* fontManager;
    Camera* camera;
}

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager options:(Options *)theOptions camera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont;

@end
