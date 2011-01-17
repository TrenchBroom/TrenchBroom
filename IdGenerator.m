//
//  IdGenerator.m
//  TrenchBroom
//
//  Created by Kristian Duske on 17.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "IdGenerator.h"

static IdGenerator *sharedInstance = nil;

@implementation IdGenerator
+ (IdGenerator *)sharedGenerator {
    @synchronized(self) {
        if (sharedInstance == nil)
            sharedInstance = [[self alloc] init];
    }
    return sharedInstance;
}

+ (id)allocWithZone:(NSZone *)zone {
    @synchronized(self) {
        if (sharedInstance == nil) {
            sharedInstance = [super allocWithZone:zone];
            return sharedInstance;  // assignment and return on first allocation
        }
    }
    return nil; // on subsequent allocation attempts return nil
}

- (id) init {
    if (self = [super init]) {
        currentId = 0;
    }
    
    return self;
}

- (NSNumber *)getId {
    return [NSNumber numberWithInt:currentId++];
}

- (id)copyWithZone:(NSZone *)zone {
    return self;
}

- (id)retain {
    return self;
}

- (NSUInteger)retainCount {
    return UINT_MAX;  // denotes an object that cannot be released
}

- (void)release {
    //do nothing
}

- (id)autorelease {
    return self;
}

@end
