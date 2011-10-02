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

#import "WadTextureEntry.h"


@implementation WadTextureEntry
- (id)initWithName:(NSString *)theName width:(int)theWidth height:(int)theHeight mip0:(NSData *)mip0Data mip1:(NSData *)mip1Data mip2:(NSData *)mip2Data mip3:(NSData *)mip3Data {
    if (self = [super initWithName:theName]) {
        width = theWidth;
        height = theHeight;
        mip0 = [mip0Data retain];
        mip1 = [mip1Data retain];
        mip2 = [mip2Data retain];
        mip3 = [mip3Data retain];
    }
    
    return self;
}

- (int)width {
    return width;
}

- (int)height {
    return height;
}

- (NSData *)mip0 {
    return mip0;
}

- (NSData *)mip1 {
    return mip1;
}

- (NSData *)mip2 {
    return mip2;
}

- (NSData *)mip3 {
    return mip3;
}

- (void)dealloc {
    [mip0 release];
    [mip1 release];
    [mip2 release];
    [mip3 release];
    [super dealloc];
}

@end
