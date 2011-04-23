//
//  BaseProperty.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BaseProperty.h"


@implementation BaseProperty

- (id)initWithBaseName:(NSString *)theBaseName {
    NSAssert(theBaseName != nil, @"base name must not be nil");
    
    if (self = [self init]) {
        baseName = [theBaseName retain];
    }
    
    return self;
}

- (EEntityDefinitionPropertyType)type {
    return EDP_BASE;
}

- (NSString *)baseName {
    return baseName;
}

- (void)dealloc {
    [baseName release];
    [super dealloc];
}

@end
