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

#import <Cocoa/Cocoa.h>

@class Texture;
@class TextureViewLayoutCell;
@class GLString;

@interface TextureViewLayoutRow : NSObject {
    @private
    float outerMargin;
    float innerMargin;
    float y;
    float width;
    float height;
    NSMutableArray* cells;
}
- (id)initAtY:(float)yPos width:(float)theWidth innerMargin:(float)theInnerMargin outerMargin:(float)theOuterMargin;

- (BOOL)addTexture:(Texture *)texture name:(GLString *)theName;
- (NSArray *)cells;

- (float)y;
- (float)height;

- (BOOL)containsY:(float)yCoord;
- (TextureViewLayoutCell *)cellAt:(NSPoint)location;
@end
