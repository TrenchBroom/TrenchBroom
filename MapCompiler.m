//
//  MapCompiler.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "MapCompiler.h"

@interface MapCompiler (private)

- (void)stdOutReadCompleted:(NSNotification *)notification;
- (void)bspTaskFinished:(NSNotification *)notification;
- (void)lightTaskFinished:(NSNotification *)notification;
- (void)visTaskFinished:(NSNotification *)notification;

- (void)launchBsp;
- (void)launchLight;
- (void)launchVis;

@end

@implementation MapCompiler (private)

- (void)stdOutReadCompleted:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSData* data = [userInfo objectForKey:NSFileHandleNotificationDataItem];
    
    NSString* string = [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
    NSAttributedString* attributedString = [[NSAttributedString alloc] initWithString:string attributes:[NSDictionary dictionaryWithObject:[NSFont fontWithName:@"Monaco" size:10] forKey:NSFontAttributeName]];
    
    [[standardOutput textStorage] appendAttributedString:attributedString];
    [standardOutput scrollToEndOfDocument:self];
    
    [string release];
    [attributedString release];
    
    NSFileHandle* stdOutHandle = [notification object];
    [stdOutHandle readInBackgroundAndNotify];
}

- (void)bspTaskFinished:(NSNotification *)notification {
    NSTask* task = [notification object];
    NSFileHandle* stdOutHandle = [[task standardOutput] fileHandleForReading];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    
    [center removeObserver:self name:NSFileHandleReadCompletionNotification object:stdOutHandle];
    [center removeObserver:self name:NSTaskDidTerminateNotification object:task];
    [task release];
    
    [self launchLight];
}

- (void)lightTaskFinished:(NSNotification *)notification {
    NSTask* task = [notification object];
    NSFileHandle* stdOutHandle = [[task standardOutput] fileHandleForReading];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    
    [center removeObserver:self name:NSFileHandleReadCompletionNotification object:stdOutHandle];
    [center removeObserver:self name:NSTaskDidTerminateNotification object:task];
    [task release];
    
    [self release];
}

- (void)visTaskFinished:(NSNotification *)notification {
    NSTask* task = [notification object];
    NSFileHandle* stdOutHandle = [[task standardOutput] fileHandleForReading];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    
    [center removeObserver:self name:NSFileHandleReadCompletionNotification object:stdOutHandle];
    [center removeObserver:self name:NSTaskDidTerminateNotification object:task];
    [task release];
    
    [self launchLight];
}

- (void)launchBsp {
    NSTask* bspTask = [[NSTask alloc] init];
    [bspTask setCurrentDirectoryPath:mapDirPath];
    [bspTask setLaunchPath:bspPath];
    [bspTask setArguments:[NSArray arrayWithObjects:mapFileName, bspFileName, nil]];
    
    NSPipe* stdOutPipe = [[NSPipe alloc] init];
    NSFileHandle* stdOutHandle = [stdOutPipe fileHandleForReading];
    [bspTask setStandardOutput:stdOutPipe];
    [stdOutPipe release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(stdOutReadCompleted:) name:NSFileHandleReadCompletionNotification object:stdOutHandle];
    [center addObserver:self selector:@selector(bspTaskFinished:) name:NSTaskDidTerminateNotification object:bspTask];

    [stdOutHandle readInBackgroundAndNotify];
    [bspTask launch];
}

- (void)launchLight {
    NSTask* lightTask = [[NSTask alloc] init];
    [lightTask setCurrentDirectoryPath:mapDirPath];
    [lightTask setLaunchPath:lightPath];
    [lightTask setArguments:[NSArray arrayWithObjects:@"-threads", @"4", @"-extra", bspFileName, nil]];
    
    NSPipe* stdOutPipe = [[NSPipe alloc] init];
    NSFileHandle* stdOutHandle = [stdOutPipe fileHandleForReading];
    [lightTask setStandardOutput:stdOutPipe];
    [stdOutPipe release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(stdOutReadCompleted:) name:NSFileHandleReadCompletionNotification object:stdOutHandle];
    [center addObserver:self selector:@selector(lightTaskFinished:) name:NSTaskDidTerminateNotification object:lightTask];

    [stdOutHandle readInBackgroundAndNotify];
    [lightTask launch];
}

- (void)launchVis {
    
    NSTask* visTask = [[NSTask alloc] init];
    [visTask setCurrentDirectoryPath:mapDirPath];
    [visTask setLaunchPath:visPath];
    [visTask setArguments:[NSArray arrayWithObjects:@"-threads", @"4", @"-level", @"4", bspFileName, nil]];
    
    NSPipe* stdOutPipe = [[NSPipe alloc] init];
    NSFileHandle* stdOutHandle = [stdOutPipe fileHandleForReading];
    [visTask setStandardOutput:stdOutPipe];
    [stdOutPipe release];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(stdOutReadCompleted:) name:NSFileHandleReadCompletionNotification object:stdOutHandle];
    [center addObserver:self selector:@selector(visTaskFinished:) name:NSTaskDidTerminateNotification object:visTask];
    
    [stdOutHandle readInBackgroundAndNotify];
    [visTask launch];
}

@end

@implementation MapCompiler

- (id)initWithMapFileUrl:(NSURL *)theMapFileUrl standardOutput:(NSTextView *)theStandardOutput {
    NSAssert(theMapFileUrl != nil, @"map file URL must not be nil");
    NSAssert(theStandardOutput != nil, @"standard output must not be nil");
    
    if ((self = [self init])) {
        standardOutput = [theStandardOutput retain];
        
        NSString* mapFilePath = [theMapFileUrl path];
        mapDirPath = [[mapFilePath stringByDeletingLastPathComponent] retain];
        mapFileName = [[mapFilePath lastPathComponent] retain];
        NSString* baseFileName = [mapFileName stringByDeletingPathExtension];
        bspFileName = [[baseFileName stringByAppendingPathExtension:@"bsp"] retain];
        
        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* resourcePath = [mainBundle resourcePath];
        NSString* compilersPath = [NSString pathWithComponents:[NSArray arrayWithObjects:resourcePath, @"Compilers", nil]];
        
        bspPath = [[compilersPath stringByAppendingPathComponent:@"TreeQBSP"] retain];
        visPath = [[compilersPath stringByAppendingPathComponent:@"Vis"] retain];
        lightPath = [[compilersPath stringByAppendingPathComponent:@"BJMHLight"] retain];
    }
    
    return self;
}

- (void)dealloc {
    [mapDirPath release];
    [mapFileName release];
    [bspFileName release];
    [bspPath release];
    [visPath release];
    [lightPath release];
    [standardOutput release];
    [super dealloc];
}

- (void)compile {
    [self launchBsp];
}

@end
