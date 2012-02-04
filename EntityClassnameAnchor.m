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

#import "EntityClassnameAnchor.h"
#import "Entity.h"

@implementation EntityClassnameAnchor

- (id)initWithEntity:(id <Entity>)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    
    if ((self = [self init])) {
        entity = [theEntity retain];
    }
    
    return self;
}

- (void)dealloc {
    [entity release];
    [super dealloc];
}

- (void)position:(TVector3f *)thePosition {
    TVector3f* ec = [entity center];
    TBoundingBox* b = [entity bounds];
    TVector3f bs;
    
    sizeOfBounds(b, &bs);
    
    *thePosition = *ec;
    thePosition->z += bs.z / 2;
    thePosition->z += 3;
}

@end
