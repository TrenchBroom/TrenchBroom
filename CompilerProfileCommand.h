//
//  CompilerProfileCommand.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

@class ConsoleWindowController;

@interface CompilerProfileCommand : NSObject {
    NSString* path;
    NSString* arguments;
}

- (id)initWithPath:(NSString *)thePath arguments:(NSString *)theArguments;

- (NSString *)path;
- (void)setPath:(NSString *)thePath;

- (NSString *)arguments;
- (void)setArguments:(NSString *)theArguments;

@end
