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

#import "BspTexture.h"


@implementation BspTexture

- (id)initWithName:(NSString *)theName image:(NSData *)theImage width:(int)theWidth height:(int)theHeight {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theImage != nil, @"image must not be nil");
    NSAssert(theWidth > 0, @"width must be a positive integer");
    NSAssert(theHeight > 0, @"height must be a positive integer");
    
    if ((self = [self init])) {
        name = [theName retain];
        image = [theImage retain];
        width = theWidth;
        height = theHeight;
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    [image release];
    [super dealloc];
}

- (NSString *)name; {
    return name;
}

- (NSData *)image {
    return image;
}

- (int)width {
    return width;
}

- (int)height {
    return height;
}

@end
