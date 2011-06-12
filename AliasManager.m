//
//  AliasManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 12.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "AliasManager.h"
#import "Alias.h"
#import "PakManager.h"

static AliasManager* sharedInstance = nil;

@interface AliasManager (private)

- (NSString *)keyForName:(NSString *)theName paths:(NSArray *)thePaths;

@end

@implementation AliasManager (private)

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

@implementation AliasManager

+ (AliasManager *)sharedManager {
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
    if (self = [super init]) {
        aliases = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [aliases release];
    [super dealloc];
}

- (Alias *)aliasWithName:(NSString *)theName paths:(NSArray *)thePaths {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(thePaths != nil, @"paths must not be nil");
    
    NSString* key = [self keyForName:theName paths:thePaths];
    Alias* alias = [aliases objectForKey:key];
    if (alias == nil) {
        PakManager* pakManager = [PakManager sharedManager];
        NSData* entry = [pakManager entryWithName:theName pakPaths:thePaths];
        if (entry != nil) {
            alias = [[Alias alloc] initWithName:theName data:entry];
            [aliases setObject:alias forKey:key];
            [alias release];
        }
    }
    
    return alias;
}

@end
