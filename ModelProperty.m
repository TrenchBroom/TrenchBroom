//
//  ModelProperty.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ModelProperty.h"


@implementation ModelProperty

- (id)initWithModelPath:(NSString *)theModelPath {
    NSAssert(theModelPath != nil, @"model path must not be nil");
    
    if (self = [self init]) {
        modelPath = [theModelPath retain];
    }
    
    return self;
}

- (EEntityDefinitionPropertyType)type {
    return EDP_MODEL;
}

- (NSString *)modelPath {
    return modelPath;
}

- (void)dealloc {
    [modelPath release];
    [super dealloc];
}

@end
