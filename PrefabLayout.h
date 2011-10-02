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

@class PrefabManager;
@class GLFontManager;
@protocol Prefab;

@interface PrefabLayout : NSObject {
    @private
    NSMutableArray* groupRows;
    PrefabManager* prefabManager;
    GLFontManager* fontManager;
    NSFont* font;
    int prefabsPerRow;
    float outerMargin;
    float innerMargin;
    float groupMargin;
    float width;
    float height;
    BOOL valid;
}

- (id)initWithPrefabManager:(PrefabManager *)thePrefabManager prefabsPerRow:(int)thePrefabsPerRow fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont;

- (NSArray *)groupRows;
- (float)height;

- (id <Prefab>)prefabAt:(NSPoint)pos;

- (void)setPrefabsPerRow:(int)thePrefabsPerRow;
- (void)setWidth:(float)width;
- (void)invalidate;
@end
