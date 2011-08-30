//
//  CompilerManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "CompilerManager.h"
#import "CompilerTool.h"

static NSString* const CompilerDictionary = @"Compilers";
static NSString* const CompilerToolsArray = @"Tools";
static NSString* const CompilerToolName = @"Name";
static NSString* const CompilerToolPath = @"Path";
static NSString* const CompilerToolParametersArray = @"Parameters";

static CompilerManager* sharedInstance = nil;

@implementation CompilerManager

+ (CompilerManager *)sharedManager {
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
        tools = [[NSMutableArray alloc] init];
        profiles = [[NSMutableArray alloc] init];
        
        NSUserDefaults* userDefaults = [NSUserDefaults standardUserDefaults];
        NSDictionary* compilersDictionary = [userDefaults dictionaryForKey:CompilerDictionary];
        if (compilersDictionary != nil) {
            NSArray* compilerToolsArray = [compilersDictionary objectForKey:CompilerToolsArray];
            if (compilerToolsArray != nil) {
                
                NSEnumerator* toolEn = [compilerToolsArray objectEnumerator];
                NSDictionary* toolDictionary;
                while ((toolDictionary = [toolEn nextObject])) {
                    NSString* name = [toolDictionary objectForKey:CompilerToolName];
                    NSString* path = [toolDictionary objectForKey:CompilerToolPath];
                    NSArray* parameters = [toolDictionary objectForKey:CompilerToolParametersArray];
                    
                    CompilerTool* tool = [[CompilerTool alloc] initWithName:name path:path parameters:parameters];
                    [tools addObject:tool];
                    [tool release];
                }
            }
        }
        
    }
    
    return self;
}

- (void)dealloc {
    [tools release];
    [profiles release];
    [super dealloc];
}

#pragma mark KVC compliance

- (void)updateDefaults {
    NSMutableDictionary* compilersDictionary = [NSMutableDictionary dictionary];
    NSMutableArray* compilerToolsArray = [NSMutableArray array];
    [compilersDictionary setObject:compilerToolsArray forKey:CompilerToolsArray];
    
    NSEnumerator* toolEn = [tools objectEnumerator];
    CompilerTool* tool;
    while ((tool = [toolEn nextObject])) {
        NSMutableDictionary* compilerToolDictionary = [NSMutableDictionary dictionary];
        if ([tool name] != nil)
            [compilerToolDictionary setObject:[tool name] forKey:CompilerToolName];
        if ([tool path] != nil)
            [compilerToolDictionary setObject:[tool path] forKey:CompilerToolPath];
        
        NSMutableArray* compilerToolParametersArray = [NSMutableArray array];
        NSEnumerator* parameterEn = [[tool parameters] objectEnumerator];
        NSString* parameter;
        while ((parameter = [parameterEn nextObject]))
            [compilerToolParametersArray addObject:parameter];
     
        [compilerToolDictionary setObject:compilerToolParametersArray forKey:CompilerToolParametersArray];
        
        [compilerToolsArray addObject:compilerToolDictionary];
    }
    
    NSUserDefaults* userDefaults = [NSUserDefaults standardUserDefaults];
    [userDefaults setObject:compilersDictionary forKey:CompilerDictionary];
}

- (NSArray *)tools {
    return tools;
}

- (void)insertObject:(CompilerTool *)theTool inToolsAtIndex:(NSUInteger)theIndex {
    [tools insertObject:theTool atIndex:theIndex];
    [self updateDefaults];
}

- (void)removeObjectFromToolsAtIndex:(NSUInteger)theIndex {
    [tools removeObjectAtIndex:theIndex];
    [self updateDefaults];
}

/*
- (NSArray *)profiles {
    return profiles;
}

- (void)insertObject:(CompilerProfile *)theProfile inProfilesAtIndex:(NSUInteger)theIndex {
    [profiles insertObject:theProfile atIndex:theIndex];
}

- (void)removeObjectFromProfilesAtIndex:(NSUInteger)theIndex {
    [profiles removeObjectAtIndex:theIndex];
}
*/

@end
