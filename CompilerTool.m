//
//  CompilerTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "CompilerTool.h"
#import "CompilerManager.h"

@implementation CompilerTool

- (id)init {
    if ((self = [super init])) {
        parameters = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithName:(NSString *)theName path:(NSString *)thePath parameters:(NSArray *)theParameters {
    if ((self = [super init])) {
        name = [theName retain];
        path = [thePath retain];
        parameters = [theParameters retain];
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    [path release];
    [parameters release];
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (void)setName:(NSString *)theName {
    [name release];
    name = [theName retain];
    [[CompilerManager sharedManager] updateDefaults];
}

- (NSString *)path {
    return path;
}

- (void)setPath:(NSString *)thePath {
    [path release];
    path = [thePath retain];
    [[CompilerManager sharedManager] updateDefaults];
}

- (NSArray *)parameters {
    return parameters;
}

- (void)insertObject:(NSString *)theParameter inParametersAtIndex:(NSUInteger)theIndex {
    [parameters insertObject:theParameter atIndex:theIndex];
    [[CompilerManager sharedManager] updateDefaults];
}

- (void)removeObjectFromParametersAtIndex:(NSUInteger)theIndex {
    [parameters removeObjectAtIndex:theIndex];
    [[CompilerManager sharedManager] updateDefaults];
}

@end
