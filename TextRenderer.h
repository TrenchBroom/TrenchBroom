/*
Copyright (C) 2010-2012 Kristian Duske

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
#import "Math.h"

@class Camera;
@class GLFontManager;
@class GLString;
@protocol TextAnchor;

@interface TextRenderer : NSObject {
@private
    Camera* camera;
    GLFontManager* fontManager;
    NSMutableDictionary* strings;
    NSMutableDictionary* anchors;
}

- (id)initWithFontManager:(GLFontManager *)theFontManager camera:(Camera *)theCamera;

- (void)addString:(NSString *)theString forKey:(id <NSCopying>)theKey withFont:(NSFont *)theFont withAnchor:(id <TextAnchor>)theAnchor;
- (void)removeStringForKey:(id <NSCopying>)theKey;

- (void)addString:(GLString *)theString forKey:(id <NSCopying>)theKey withAnchor:(id <TextAnchor>)theAnchor;
- (void)moveStringWithKey:(id <NSCopying>)theKey toTextRenderer:(TextRenderer *)theTextRenderer;

- (void)renderColor:(const TVector4f *)theColor;

- (void)removeAllStrings;

@end
