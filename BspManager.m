//
//  BspManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BspManager.h"
#import "Bsp.h"
#import "PakManager.h"

static BspManager* sharedInstance = nil;

@interface BspManager (private)

- (NSString *)keyForName:(NSString *)theName paths:(NSArray *)thePaths;

@end

@implementation BspManager (private)

- (NSString *)keyForName:(NSString *)theName paths:(NSArray *)thePaths {
    NSMutableString* key = [[NSMutableString alloc] init];
    
    NSEnumerator* pathEn = [thePaths objectEnumerator];
    NSString* path;
    while ((path = [pathEn nextObject])) {
        [key appendString:path];
        [key appendString:@";"];
    }
    
    [key appendString:theName];
    return [key autorelease];
}

@end

@implementation BspManager

+ (BspManager *)sharedManager {
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

- (id)init {
    if ((self = [super init])) {
        bsps = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [bsps release];
    [super dealloc];
}

- (Bsp *)bspWithName:(NSString *)theName paths:(NSArray *)thePaths {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(thePaths != nil, @"paths must not be nil");
    
    NSString* key = [self keyForName:theName paths:thePaths];
    Bsp* bsp = [bsps objectForKey:key];
    if (bsp == nil) {
        PakManager* pakManager = [PakManager sharedManager];
        NSData* entry = [pakManager entryWithName:theName pakPaths:thePaths];
        if (entry != nil) {
            bsp = [[Bsp alloc] initWithName:theName data:entry];
            [bsps setObject:bsp forKey:key];
            [bsp release];
        }
    }
    
    return bsp;
}

@end
