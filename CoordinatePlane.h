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

@class Vector3f;

@protocol CoordinatePlane <NSObject>

- (Vector3f *)project:(Vector3f *)thePoint;
- (BOOL)clockwise:(Vector3f *)theNorm;

- (float)xOf:(Vector3f *)thePoint;
- (float)yOf:(Vector3f *)thePoint;
- (float)zOf:(Vector3f *)thePoint;

- (void)set:(Vector3f *)thePoint toX:(float)x y:(float)y z:(float)z;

@end
