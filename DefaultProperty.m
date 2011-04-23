//
//  DefaultProperty.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "DefaultProperty.h"


@implementation DefaultProperty

- (id)initWithName:(NSString *)theName value:(NSString *)theValue {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theValue != nil, @"value must not be nil");
    
    if (self = [self init]) {
        name = [theName retain];
        value = [theValue retain];
    }
    
    return self;
}

- (EEntityDefinitionPropertyType)type {
    return EDP_DEFAULT;
}

- (NSString *)name {
    return name;
}

- (NSString *)value {
    return value;
}

- (void)dealloc {
    [name release];
    [value release];
    [super dealloc];
}

@end
