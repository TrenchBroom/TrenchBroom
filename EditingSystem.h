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

@interface EditingSystem : NSObject {
    const TVector3f* xAxisPos;
    const TVector3f* xAxisNeg;
    const TVector3f* yAxisPos;
    const TVector3f* yAxisNeg;
    const TVector3f* zAxisPos;
    const TVector3f* zAxisNeg;
    TMatrix4f worldToLocal;
    TMatrix4f localToWorld;
}

- (id)initWithCamera:(Camera *)theCamera vertical:(BOOL)vertical;

- (const TVector3f *)xAxisPos;
- (const TVector3f *)xAxisNeg;
- (const TVector3f *)yAxisPos;
- (const TVector3f *)yAxisNeg;
- (const TVector3f *)zAxisPos;
- (const TVector3f *)zAxisNeg;

- (float)intersectWithRay:(const TRay *)theRay planePosition:(const TVector3f *)thePlanePos;

- (const TMatrix4f *)worldToLocalMatrix;
- (const TMatrix4f *)localToWorldMatrix;

- (void)eliminateLRFrom:(const TVector3f *)theVector result:(TVector3f *)theResult;
- (void)eliminateUDFrom:(const TVector3f *)theVector result:(TVector3f *)theResult;
- (void)eliminateFBFrom:(const TVector3f *)theVector result:(TVector3f *)theResult;

@end
