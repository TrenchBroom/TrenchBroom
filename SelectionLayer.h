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
@class GLResources;
@class GLFontManager;
@class SelectionManager;

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
    SelectionManager* selectionManager;
}

- (id)initWithVbo:(VBOBuffer *)theVbo glResources:(GLResources *)theGLResources selectionManager:(SelectionManager *)theSelectionManager options:(Options *)theOptions camera:(Camera *)theCamera;

@end
