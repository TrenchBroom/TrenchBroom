//
//  MapCompiler.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "MapCompiler.h"
#import "ConsoleWindowController.h"

@interface MapCompiler (private)

- (void)log:(NSData *)theData;
- (void)stdOutReadCompleted:(NSNotification *)notification;
- (void)bspTaskFinished:(NSNotification *)notification;
- (void)lightTaskFinished:(NSNotification *)notification;
- (void)visTaskFinished:(NSNotification *)notification;

- (void)launchBsp;
- (void)launchLight;
- (void)launchVis;

@end

@implementation MapCompiler (private)

- (void)log:(NSData *)theData {
    NSString* string = [[NSString alloc] initWithData:theData encoding:NSASCIIStringEncoding];
    [console log:string];
    [string release];
}

- (void)stdOutReadCompleted:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    [self log:[userInfo objectForKey:NSFileHandleNotificationDataItem]];
    
    NSFileHandle* stdOutHandle = [notification object];
    [stdOutHandle readInBackgroundAndNotify];
}

- (void)bspTaskFinished:(NSNotification *)notification {
    NSTask* task = [notification object];
    NSFileHandle* stdOutHandle = [[task standardOutput] fileHandleForReading];
    [self log:[stdOutHandle readDataToEndOfFile]];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    
    [center removeObserver:self name:NSFileHandleReadCompletionNotification object:stdOutHandle];
    [center removeObserver:self name:NSTaskDidTerminateNotification object:task];
    [task release];
    
    [self launchVis];
}

- (void)lightTaskFinished:(NSNotification *)notification {
    NSTask* task = [notification object];
    NSFileHandle* stdOutHandle = [[task standardOutput] fileHandleForReading];
    [self log:[stdOutHandle readDataToEndOfFile]];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    
    [center removeObserver:self name:NSFileHandleReadCompletionNotification object:stdOutHandle];
    [center removeObserver:self name:NSTaskDidTerminateNotification object:task];
    [task release];
    
    [self release];
}

- (void)visTaskFinished:(NSNotification *)notification {
    NSTask* task = [notification object];
    NSFileHandle* stdOutHandle = [[task standardOutput] fileHandleForReading];
    [self log:[stdOutHandle readDataToEndOfFile]];
    
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

    [console logBold:@"\n========== Launching QBSP ===========\n\n"];
    
    [stdOutHandle readInBackgroundAndNotify];
    [bspTask launch];
}

- (void)launchLight {
    NSTask* lightTask = [[NSTask alloc] init];
    [lightTask setCurrentDirectoryPath:mapDirPath];
    [lightTask setLaunchPath:lightPath];
    [lightTask setArguments:[NSArray arrayWithObjects:@"-threads", @"2", @"-extra", bspFileName, nil]];
    
    NSPipe* stdOutPipe = [[NSPipe alloc] init];
    NSFileHandle* stdOutHandle = [stdOutPipe fileHandleForReading];
    [lightTask setStandardOutput:stdOutPipe];
    [stdOutPipe release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(stdOutReadCompleted:) name:NSFileHandleReadCompletionNotification object:stdOutHandle];
    [center addObserver:self selector:@selector(lightTaskFinished:) name:NSTaskDidTerminateNotification object:lightTask];

    [console logBold:@"\n========== Launching Light ===========\n\n"];
    
    [stdOutHandle readInBackgroundAndNotify];
    [lightTask launch];
}

- (void)launchVis {
    
    NSTask* visTask = [[NSTask alloc] init];
    [visTask setCurrentDirectoryPath:mapDirPath];
    [visTask setLaunchPath:visPath];
    [visTask setArguments:[NSArray arrayWithObjects:@"-threads", @"2", @"-level", @"4", bspFileName, nil]];
    
    NSPipe* stdOutPipe = [[NSPipe alloc] init];
    NSFileHandle* stdOutHandle = [stdOutPipe fileHandleForReading];
    [visTask setStandardOutput:stdOutPipe];
    [stdOutPipe release];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(stdOutReadCompleted:) name:NSFileHandleReadCompletionNotification object:stdOutHandle];
    [center addObserver:self selector:@selector(visTaskFinished:) name:NSTaskDidTerminateNotification object:visTask];
    
    [console logBold:@"\n========== Launching Vis ===========\n\n"];
    
    [stdOutHandle readInBackgroundAndNotify];
    [visTask launch];
}

@end

@implementation MapCompiler

- (id)initWithMapFileUrl:(NSURL *)theMapFileUrl console:(ConsoleWindowController *)theConsole {
    NSAssert(theMapFileUrl != nil, @"map file URL must not be nil");
    NSAssert(theConsole != nil, @"console must not be nil");
    
    if ((self = [self init])) {
        console = [theConsole retain];
        
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
    [console release];
    [super dealloc];
}

- (void)compile {
    [self launchBsp];
}

@end
