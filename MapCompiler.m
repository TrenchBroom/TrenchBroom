//
//  MapCompiler.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "MapCompiler.h"
#import "ConsoleWindowController.h"
#import "CompilerProfile.h"
#import "CompilerProfileRunner.h"

static NSString* const MapFileReplacement = @"$map";
static NSString* const BspFileReplacement = @"$bsp";

@implementation MapCompiler

- (id)initWithMapFileUrl:(NSURL *)theMapFileUrl profile:(CompilerProfile *)theProfile console:(ConsoleWindowController *)theConsole {
    NSAssert(theMapFileUrl != nil, @"map file URL must not be nil");
    NSAssert(theProfile != nil, @"profile must not be nil");
    NSAssert(theConsole != nil, @"console must not be nil");
    
    if ((self = [self init])) {
        NSString* mapFilePath = [theMapFileUrl path];
        NSString* mapDirPath = [mapFilePath stringByDeletingLastPathComponent];
        NSString* mapFileName = [mapFilePath lastPathComponent];
        NSString* baseFileName = [mapFileName stringByDeletingPathExtension];
        NSString* bspFileName = [baseFileName stringByAppendingPathExtension:@"bsp"];
        
        NSMutableDictionary* replacements = [[NSMutableDictionary alloc] init];
        [replacements setObject:mapFileName forKey:MapFileReplacement];
        [replacements setObject:bspFileName forKey:BspFileReplacement];
        
        profileRunner = [[theProfile runnerWithConsole:theConsole workDir:mapDirPath replacements:replacements] retain];
        [replacements release];
    }
    
    return self;
}

- (void)dealloc {
    [profileRunner release];
    [super dealloc];
}

- (void)compile {
    [profileRunner run];
}

@end
