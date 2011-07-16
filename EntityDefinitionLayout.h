//
//  EntityDefinitionLayout.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@class EntityDefinitionManager;
@class EntityDefinition;
@class GLFontManager;

@interface EntityDefinitionLayout : NSObject {
@private
    NSMutableArray* rows;
    EntityDefinitionManager* entityDefinitionManager;
    GLFontManager* fontManager;
    NSFont* font;
    float outerMargin;
    float innerMargin;
    float width;
    float height;
    BOOL valid;
}

- (id)initWithEntityDefinitionManager:(EntityDefinitionManager *)theEntityDefinitionManager fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont;

- (NSArray *)rows;
- (float)height;

- (EntityDefinition *)entityDefinitionAt:(NSPoint)pos;

- (void)setWidth:(float)width;
- (void)invalidate;

@end
