//
//  EntityClassnameAnchor.m
//  TrenchBroom
//
//  Created by Kristian Duske on 13.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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
