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

#import "CameraAnimation.h"
#import "Camera.h"

@implementation CameraAnimation

- (id)initWithCamera:(Camera *)theCamera duration:(NSTimeInterval)theDuration {
    NSAssert(theCamera != nil, @"camera must not be nil");
    
    if ((self = [super initWithDuration:theDuration animationCurve:NSAnimationEaseInOut])) {
        camera = theCamera;

        [super setFrameRate:60];
        [super setAnimationBlockingMode:NSAnimationNonblocking];
        [super setDelegate:self];
    }
    
    return self;
}

- (void)startAnimation {
    NSMutableSet* animations = [camera animations];
    
    NSSet* copy = [[NSSet alloc] initWithSet:animations];
    for (NSAnimation* animation in copy)
        [animation stopAnimation];
    [copy release];
    
    [animations addObject:self];
    [super startAnimation];
}

- (void)animationDidEnd:(NSAnimation *)animation {
    NSMutableSet* animations = [camera animations];
    
    [animations removeObject:self];
    [self release];
}

- (void)animationDidStop:(NSAnimation *)animation {
    NSMutableSet* animations = [camera animations];
    [animations removeObject:self];
    
    [self release];
}

@end
