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

#import <Foundation/Foundation.h>
#import "Math.h"

@class MutableFace;
@protocol Face;

@interface FaceInfo : NSObject {
@private
    NSNumber* faceId;
    TVector3i point1;
    TVector3i point2;
    TVector3i point3;
    int xOffset;
    int yOffset;
    float xScale;
    float yScale;
    float rotation;
    NSString* texture;
}

+ (id)faceInfoFor:(id <Face>)theFace;

- (id)initWithFace:(id <Face>)theFace;

- (void)updateFace:(MutableFace *)theFace;

@end
