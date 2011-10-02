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

@class Texture;

@interface TextureViewLayoutCell : NSObject {
    @private
    Texture* texture;
    NSRect textureRect;
    NSRect cellRect;
    NSRect nameRect;
}

- (id)initAt:(NSPoint)location texture:(Texture *)theTexture nameSize:(NSSize)theNameSize;

- (NSRect)cellRect;
- (NSRect)textureRect;
- (NSRect)nameRect;

- (BOOL)contains:(NSPoint)point;

- (Texture *)texture;

@end
