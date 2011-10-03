/*
Copyright (C) 2010-2011 Kristian Duske

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

@interface EditingPlane : NSObject {
    const TVector3f* xAxis;
    const TVector3f* yAxis;
    const TVector3f* frontAxis;
    const TVector3f* backAxis;
    const TVector3f* upAxis;
    const TVector3f* rightAxis;
    const TVector3f* downAxis;
    const TVector3f* leftAxis;
}

- (id)initWithCamera:(Camera *)theCamera;

- (const TVector3f *)frontAxis;
- (const TVector3f *)backAxis;
- (const TVector3f *)upAxis;
- (const TVector3f *)rightAxis;
- (const TVector3f *)downAxis;
- (const TVector3f *)leftAxis;

- (float)intersectWithRay:(const TRay *)theRay planePosition:(const TVector3f *)thePlanePos;

@end
