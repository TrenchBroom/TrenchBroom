/*
This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

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
