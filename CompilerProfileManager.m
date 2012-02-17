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

#import "CompilerProfileManager.h"
#import "CompilerProfile.h"
#import "CompilerProfileCommand.h"
#import "PreferencesManager.h"

static NSString* const CompilerDictionary = @"Compilers";
static NSString* const CompilerProfilesArray = @"Profiles";
static NSString* const CompilerProfileName = @"Name";
static NSString* const CompilerProfileCommandsArray = @"Commands";
static NSString* const CompilerProfileCommandPath = @"Path";
static NSString* const CompilerProfileCommandArguments = @"Arguments";

static CompilerProfileManager* sharedInstance = nil;

@implementation CompilerProfileManager

+ (CompilerProfileManager *)sharedManager {
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
        profiles = [[NSMutableArray alloc] init];
        
        NSUserDefaults* userDefaults = [NSUserDefaults standardUserDefaults];
        NSDictionary* compilersDictionary = [userDefaults dictionaryForKey:CompilerDictionary];
        if (compilersDictionary != nil) {
            NSArray* profilesArray = [compilersDictionary objectForKey:CompilerProfilesArray];
            if (profilesArray != nil) {
                for (NSDictionary* profileDict in profilesArray) {
                    NSString* profileName = [profileDict objectForKey:CompilerProfileName];
                    
                    NSArray* commandsArray = [profileDict objectForKey:CompilerProfileCommandsArray];
                    NSMutableArray* commands = [[NSMutableArray alloc] initWithCapacity:[commandsArray count]];
                    
                    for (NSDictionary* commandDict in commandsArray) {
                        NSString* commandPath = [commandDict objectForKey:CompilerProfileCommandPath];
                        NSString* commandArguments = [commandDict objectForKey:CompilerProfileCommandArguments];
                        
                        CompilerProfileCommand* command = [[CompilerProfileCommand alloc] initWithPath:commandPath arguments:commandArguments];
                        [commands addObject:command];
                        [command release];
                    }
                    
                    CompilerProfile* profile = [[CompilerProfile alloc] initWithName:profileName commands:commands];
                    [profiles addObject:profile];
                    
                    [commands release];
                    [profile release];
                }
            }
        }
    }
    
    return self;
}

- (void)dealloc {
    [profiles release];
    [super dealloc];
}

#pragma mark KVC compliance

- (void)updateDefaults {
    NSMutableDictionary* compilersDict = [NSMutableDictionary dictionary];
    NSMutableArray* profileDictsArray = [NSMutableArray array];
    [compilersDict setObject:profileDictsArray forKey:CompilerProfilesArray];
    
    for (CompilerProfile* profile in profiles) {
        NSMutableDictionary* profileDict = [NSMutableDictionary dictionary];
        if ([profile name] != nil)
            [profileDict setObject:[profile name] forKey:CompilerProfileName];
        
        NSMutableArray* commandDictsArray = [NSMutableArray array];
        for (CompilerProfileCommand* command in [profile commands]) {
            NSMutableDictionary* commandDict = [NSMutableDictionary dictionary];
            if ([command path] != nil)
                [commandDict setObject:[command path] forKey:CompilerProfileCommandPath];
            if ([command arguments] != nil)
                [commandDict setObject:[command arguments] forKey:CompilerProfileCommandArguments];
            [commandDictsArray addObject:commandDict];
        }
     
        [profileDict setObject:commandDictsArray forKey:CompilerProfileCommandsArray];
        [profileDictsArray addObject:profileDict];
    }
    
    NSUserDefaults* userDefaults = [NSUserDefaults standardUserDefaults];
    [userDefaults setObject:compilersDict forKey:CompilerDictionary];
}

- (NSArray *)profiles {
    return profiles;
}

- (void)insertObject:(CompilerProfile *)theProfile inProfilesAtIndex:(NSUInteger)theIndex {
    [profiles insertObject:theProfile atIndex:theIndex];
    [self updateDefaults];

    PreferencesManager* preferences = [PreferencesManager sharedManager];
    int index = [preferences lastCompilerProfileIndex];
    if (index >= theIndex)
        [preferences setLastCompilerProfileIndex:index + 1];
}

- (void)removeObjectFromProfilesAtIndex:(NSUInteger)theIndex {
    [profiles removeObjectAtIndex:theIndex];
    [self updateDefaults];

    PreferencesManager* preferences = [PreferencesManager sharedManager];
    int index = [preferences lastCompilerProfileIndex];
    if (index > theIndex)
        [preferences setLastCompilerProfileIndex:index - 1];
    else if (index == theIndex)
        [preferences setLastCompilerProfileIndex:-1];
}

@end
