//
//  EntityView.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@class GLResources;
@class EntityDefinitionLayout;
@class EntityDefinitionManager;
@class EntityDefinition;

@interface EntityView : NSOpenGLView {
    NSPoint dragDistance;
    EntityDefinition* draggedEntityDefinition;
    EntityDefinitionManager* entityDefinitionManager;
    GLResources* glResources;
    EntityDefinitionLayout* layout;
    IBOutlet id target;
}

- (void)setGLResources:(GLResources *)theGLResources entityDefinitionManager:(EntityDefinitionManager *)theEntityDefinitionManager;

@end
