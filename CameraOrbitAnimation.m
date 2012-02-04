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

#import "CameraOrbitAnimation.h"
#import "Camera.h"

@implementation CameraOrbitAnimation

- (id)initWithCamera:(Camera *)theCamera orbitCenter:(const TVector3f *)theOrbitCenter hDelta:(float)theHDelta vDelta:(float)theVDelta duration:(NSTimeInterval)duration {
    NSAssert(theCamera != nil, @"camera must not be nil");
    NSAssert(theOrbitCenter != NULL, @"orbit center must not be NULL");
    
    if ((self = [super initWithCamera:theCamera duration:duration])) {
        orbitCenter = *theOrbitCenter;
        hDelta = theHDelta;
        vDelta = theVDelta;
    }
    
    return self;
}

- (void)setCurrentProgress:(NSAnimationProgress)progress {
    [super setCurrentProgress:progress];

    float hAngle = hDelta * progress;
    float vAngle = vDelta * progress;
    
    [camera orbitCenter:&orbitCenter hAngle:hAngle vAngle:vAngle];
}

@end
