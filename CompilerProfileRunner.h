//
//  CompilerProfileRunner.h
//  TrenchBroom
//
//  Created by Kristian Duske on 31.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

@class ConsoleWindowController;
@class CompilerProfile;

@interface CompilerProfileRunner : NSObject {
    ConsoleWindowController* console;
    
    CompilerProfile* profile;
    NSUInteger commandIndex;
    
    NSString* workDir;
    NSDictionary* replacements;
}

- (id)initWithProfile:(CompilerProfile *)theProfile console:(ConsoleWindowController *)theConsole workDir:(NSString *)theWorkDir replacements:(NSDictionary *)theReplacements;

- (void)run;

@end
