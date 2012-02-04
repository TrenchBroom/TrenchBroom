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

#import "TextureViewLayoutCell.h"
#import "Texture.h"

@implementation TextureViewLayoutCell

- (id)initAt:(NSPoint)location texture:(Texture *)theTexture nameSize:(NSSize)theNameSize {
    if (self = [self init]) {
        texture = [theTexture retain];
             
        cellRect = NSMakeRect(location.x, location.y, fmax([texture width], theNameSize.width), [texture height] + theNameSize.height + 2);
        textureRect = NSMakeRect(location.x + (cellRect.size.width - [texture width]) / 2, location.y, [texture width], [texture height]);
        nameRect = NSMakeRect(location.x + (cellRect.size.width - theNameSize.width) / 2, location.y + [texture height] + 1, theNameSize.width, theNameSize.height);
    }
    
    return self;
}

- (NSRect)cellRect {
    return cellRect;
}

- (NSRect)textureRect {
    return textureRect;
}

- (NSRect)nameRect {
    return nameRect;
}

- (BOOL)contains:(NSPoint)point {
    return point.x >= cellRect.origin.x && 
           point.x <= cellRect.origin.x + cellRect.size.width && 
           point.y >= cellRect.origin.y && 
           point.y <= cellRect.origin.y + cellRect.size.height;
}

- (Texture *)texture {
    return texture;
}

- (void)dealloc {
    [texture release];
    [super dealloc];
}

@end
