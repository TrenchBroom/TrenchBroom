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
#import "Figure.h"
#import "Math.h"

@interface ClipPlaneFeedbackFigure : NSObject <Figure> {
    TVector3i point1;
    TVector3i point2;
    TVector3i point3;
}

- (id)initWithPoint1:(TVector3i *)thePoint1 point2:(TVector3i *)thePoint2 point3:(TVector3i *)thePoint3;

@end
