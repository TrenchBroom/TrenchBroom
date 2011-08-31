//
//  MapCompiler.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@class ConsoleWindowController;
@class CompilerProfile;
@class CompilerProfileRunner;

@interface MapCompiler : NSObject {
    CompilerProfileRunner* profileRunner;
}

- (id)initWithMapFileUrl:(NSURL *)theMapFileUrl profile:(CompilerProfile *)theProfile console:(ConsoleWindowController *)theConsole;

- (void)compile;

@end
