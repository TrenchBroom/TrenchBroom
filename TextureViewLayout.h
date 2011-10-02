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

#import <Cocoa/Cocoa.h>
#import "TextureFilter.h"

@class Texture;
@class GLFontManager;

@interface TextureViewLayout : NSObject {
    @private
    NSMutableArray* rows;
    NSMutableArray* textures;
    float width;
    float innerMargin;
    float outerMargin;
    id<TextureFilter> filter;
    GLFontManager* fontManager;
    NSFont* font;
    BOOL valid;
}

- (id)initWithWidth:(float)theWidth innerMargin:(float)theInnerMargin outerMargin:(float)theOuterMargin fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont;

- (void)addTexture:(Texture *)theTexture;
- (void)addTextures:(NSArray *)theTextures;
- (void)clear;

- (void)setWidth:(float)theWidth;

- (void)layout;

- (float)height;
- (NSArray *)rows;
- (NSArray *)rowsInY:(float)y height:(float)height;
- (Texture *)textureAt:(NSPoint)location;

- (void)setTextureFilter:(id <TextureFilter>)theFilter;

@end
