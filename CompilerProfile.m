//
//  CompilerTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "CompilerProfile.h"
#import "CompilerProfileManager.h"
#import "CompilerProfileCommand.h"
#import "CompilerProfileRunner.h"

@implementation CompilerProfile

- (id)init {
    if ((self = [super init])) {
        commands = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithName:(NSString *)theName commands:(NSArray *)theCommands {
    if ((self = [super init])) {
        name = [theName retain];
        commands = [[NSMutableArray alloc] initWithArray:theCommands];
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    [commands release];
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (void)setName:(NSString *)theName {
    [name release];
    name = [theName retain];
    [[CompilerProfileManager sharedManager] updateDefaults];
}

- (NSArray *)commands {
    return commands;
}

- (void)insertObject:(CompilerProfileCommand *)theCommand inCommandsAtIndex:(NSUInteger)theIndex {
    [commands insertObject:theCommand atIndex:theIndex];
    [[CompilerProfileManager sharedManager] updateDefaults];
}

- (void)removeObjectFromCommandsAtIndex:(NSUInteger)theIndex {
    [commands removeObjectAtIndex:theIndex];
    [[CompilerProfileManager sharedManager] updateDefaults];
}

- (CompilerProfileRunner *)runnerWithConsole:(ConsoleWindowController *)theConsole workDir:(NSString *)theWorkDir replacements:(NSDictionary *)theReplacements {
    return [[[CompilerProfileRunner alloc] initWithProfile:self console:theConsole workDir:theWorkDir replacements:theReplacements] autorelease];
}

@end
