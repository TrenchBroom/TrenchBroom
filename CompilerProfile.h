//
//  CompilerTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

@class CompilerProfileCommand;
@class ConsoleWindowController;
@class CompilerProfileRunner;

@interface CompilerProfile : NSObject {
    NSString* name;
    NSMutableArray* commands;
}

- (id)initWithName:(NSString *)theName commands:(NSArray *)theCommands;

- (NSString *)name;
- (void)setName:(NSString *)theName;

- (NSArray *)commands;
- (void)insertObject:(CompilerProfileCommand *)theCommand inCommandsAtIndex:(NSUInteger)theIndex;
- (void)removeObjectFromCommandsAtIndex:(NSUInteger)theIndex;

- (CompilerProfileRunner *)runnerWithConsole:(ConsoleWindowController *)theConsole workDir:(NSString *)theWorkDir replacements:(NSDictionary *)theReplacements;

@end
