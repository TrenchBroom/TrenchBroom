//
//  CompilerManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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
                NSEnumerator* profileEn = [profilesArray objectEnumerator];
                NSDictionary* profileDict;
                while ((profileDict = [profileEn nextObject])) {
                    NSString* profileName = [profileDict objectForKey:CompilerProfileName];
                    
                    NSArray* commandsArray = [profileDict objectForKey:CompilerProfileCommandsArray];
                    NSMutableArray* commands = [[NSMutableArray alloc] initWithCapacity:[commandsArray count]];
                    
                    NSEnumerator* commandEn = [commandsArray objectEnumerator];
                    NSDictionary* commandDict;
                    while ((commandDict = [commandEn nextObject])) {
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
    
    NSEnumerator* profileEn = [profiles objectEnumerator];
    CompilerProfile* profile;
    while ((profile = [profileEn nextObject])) {
        NSMutableDictionary* profileDict = [NSMutableDictionary dictionary];
        if ([profile name] != nil)
            [profileDict setObject:[profile name] forKey:CompilerProfileName];
        
        NSMutableArray* commandDictsArray = [NSMutableArray array];
        NSEnumerator* commandEn = [[profile commands] objectEnumerator];
        CompilerProfileCommand* command;
        while ((command = [commandEn nextObject])) {
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
