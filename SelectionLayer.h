//
//  SelectionLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "GeometryLayer.h"
#import "DefaultEntityLayer.h"

@class BrushBoundsRenderer;
@class Camera;
@class GLFontManager;

@interface SelectionLayer : GeometryLayer <EntityLayer> {
    EntityBoundsRenderer* entityBoundsRenderer;
    EntityAliasRenderer* entityAliasRenderer;
}

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager options:(Options *)theOptions camera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont;

@end
