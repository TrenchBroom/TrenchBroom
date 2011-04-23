//
//  ChoiceProperty.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ChoiceProperty.h"


@implementation ChoiceProperty

- (id)initWithName:(NSString*)theName arguments:(NSArray *)theArguments {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theArguments != nil, @"argument array must not be nil");
    
    if (self = [self init]) {
        name = [theName retain];
        arguments = [theArguments retain];
    }
    
    return self;
}

- (EEntityDefinitionPropertyType)type {
    return EDP_CHOICE;
}

- (NSString *)name {
    return name;
}

- (NSArray *)arguments {
    return arguments;
}

- (void)dealloc {
    [name release];
    [arguments release];
    [super dealloc];
}

@end
