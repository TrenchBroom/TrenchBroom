//
//  EntityInfo.m
//  TrenchBroom
//
//  Created by Kristian Duske on 08.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityInfo.h"
#import "Entity.h"
#import "MutableEntity.h"

@implementation EntityInfo

+ (id)entityInfoFor:(id <Entity>)theEntity {
    return [[[EntityInfo alloc] initWithEntity:theEntity] autorelease];
}

- (id)initWithEntity:(id <Entity>)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    
    if ((self = [self init])) {
        entityId = [[theEntity entityId] retain];
        properties = [[NSDictionary alloc] initWithDictionary:[theEntity properties] copyItems:YES];
    }
    
    return self;
}

- (void)updateEntity:(MutableEntity *)theEntity {
    NSAssert([entityId isEqualToNumber:[theEntity entityId]], @"entity id must be equal");
    [theEntity replaceProperties:properties];
}

@end
