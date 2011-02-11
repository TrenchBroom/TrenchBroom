//
//  Observer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Observer.h"


@implementation Observer
- (id)initWithTarget:(id)theTarget selector:(SEL)theSelector name:(NSString *)theName{
    if (theTarget == nil)
        [NSException raise:NSInvalidArgumentException format:@"target must not be nil"];
    if (theSelector == NULL)
        [NSException raise:NSInvalidArgumentException format:@"selector must not be null"];
    if (theName == nil)
        [NSException raise:NSInvalidArgumentException format:@"name must not be nil"];
    
    if (self = [self init]) {
        target = [theTarget retain];
        selector = theSelector;
        name = [theName retain];
    }
    
    return self;
}

- (void)notify:(NSNotification *)notification {
    [target performSelector:selector withObject:notification];
}

- (id)target {
    return target;
}

- (NSString *)name {
    return name;
}

- (void)dealloc {
    [target release];
    [name release];
    [super dealloc];
}


@end
