//
//  CompilerProfileCommand.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "CompilerProfileCommand.h"
#import "CompilerProfileManager.h"

@implementation CompilerProfileCommand

- (id)initWithPath:(NSString *)thePath arguments:(NSString *)theArguments {
    if ((self = [self init])) {
        path = [thePath retain];
        arguments = [theArguments retain];
    }
    
    return self;
}

- (void)dealloc {
    [path release];
    [arguments release];
    [super dealloc];
}

- (NSString *)path {
    return path;
}

- (void)setPath:(NSString *)thePath {
    [path release];
    path = [thePath retain];
    [[CompilerProfileManager sharedManager] updateDefaults];
}

- (NSString *)arguments {
    return arguments;
}

- (void)setArguments:(NSString *)theArguments {
    [arguments release];
    arguments = [theArguments retain];
    [[CompilerProfileManager sharedManager] updateDefaults];
}

@end
