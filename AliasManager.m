/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "AliasManager.h"
#import "Alias.h"
#import "PakManager.h"
#import "PreferencesManager.h"

static AliasManager* sharedInstance = nil;

@interface AliasManager (private)

- (void)preferencesDidChange:(NSNotification *)notification;
- (NSString *)keyForName:(NSString *)theName paths:(NSArray *)thePaths;

@end

@implementation AliasManager (private)

- (void)preferencesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    if (DefaultsQuakePath != [userInfo objectForKey:DefaultsKey])
        return;
    
    [aliases removeAllObjects];
}

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

- (oneway void)release {
    //do nothing
}

- (id)autorelease {
    return self;
}

- (id)init {
    if ((self = [super init])) {
        aliases = [[NSMutableDictionary alloc] init];
        
        PreferencesManager* preferences = [PreferencesManager sharedManager];
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(preferencesDidChange:) name:DefaultsDidChange object:preferences];
    }
    
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [aliases release];
    [super dealloc];
}

- (Alias *)aliasWithName:(NSString *)theName paths:(NSArray *)thePaths {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(thePaths != nil, @"paths must not be nil");
    
    NSString* key = [self keyForName:theName paths:thePaths];
    Alias* alias = [aliases objectForKey:key];
    if (alias == nil) {
        NSLog(@"Loading alias model '%@', search paths: %@", theName, [thePaths componentsJoinedByString:@", "]);
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
