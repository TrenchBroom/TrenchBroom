//
//  EntityDefinitionLayout.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "EntityDefinitionManager.h"

@class EntityDefinitionLayoutCell;
@class EntityDefinition;
@class GLFontManager;
@protocol EntityDefinitionFilter;

@interface EntityDefinitionLayout : NSObject {
@private
    NSMutableArray* rows;
    NSArray* entityDefinitions;
    GLFontManager* fontManager;
    NSFont* font;
    float outerMargin;
    float innerMargin;
    float width;
    float height;
    BOOL valid;
    id <EntityDefinitionFilter> filter;
}

- (id)initWithFontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont;

- (NSArray *)rows;
- (float)height;

- (EntityDefinitionLayoutCell *)cellAt:(NSPoint)pos;
- (EntityDefinition *)entityDefinitionAt:(NSPoint)pos;

- (void)setEntityDefinitions:(NSArray *)theEntityDefinitions;
- (void)setEntityDefinitionFilter:(id <EntityDefinitionFilter>)theFilter;
- (void)setWidth:(float)width;
- (void)invalidate;
- (void)clear;

@end
