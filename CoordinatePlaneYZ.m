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

#import "CoordinatePlaneYZ.h"
#import "Vector3f.h"

@implementation CoordinatePlaneYZ

- (BOOL)clockwise:(Vector3f *)theNorm {
    return [theNorm x] > 0;
}

- (Vector3f *)project:(Vector3f *)thePoint {
    return [[[Vector3f alloc] initWithFloatX:[thePoint y] y:[thePoint z] z:[thePoint x]] autorelease];
}

- (float)xOf:(Vector3f *)thePoint {
            return [thePoint y];
}

- (float)yOf:(Vector3f *)thePoint {
            return [thePoint z];
}

- (float)zOf:(Vector3f *)thePoint {
            return [thePoint x];
}

- (void)set:(Vector3f *)thePoint toX:(float)x y:(float)y z:(float)z {
    [thePoint setX:[thePoint z] y:[thePoint x] z:[thePoint y]];
}

@end
