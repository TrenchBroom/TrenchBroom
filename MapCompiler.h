//
//  MapCompiler.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MapCompiler : NSObject {
    NSTextView* standardOutput;
    NSString* mapDirPath;
    NSString* mapFileName;
    NSString* bspFileName;
    NSString* bspPath;
    NSString* lightPath;
    NSString* visPath;
}

- (id)initWithMapFileUrl:(NSURL *)theMapFileUrl standardOutput:(NSTextView *)theStandardOutput;

- (void)compile;

@end
