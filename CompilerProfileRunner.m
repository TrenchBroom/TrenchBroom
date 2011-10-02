/*
This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "CompilerProfileRunner.h"
#import "CompilerProfileCommand.h"
#import "ConsoleWindowController.h"

@interface CompilerProfileRunner (private)

- (void)log:(NSData *)theData;
- (void)stdOutReadCompleted:(NSNotification *)notification;
- (void)taskFinished:(NSNotification *)notification;
- (void)runCurrentCommand;

@end

@implementation CompilerProfileRunner (private)

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

- (void)taskFinished:(NSNotification *)notification {
    NSTask* task = [notification object];
    NSFileHandle* stdOutHandle = [[task standardOutput] fileHandleForReading];
    [self log:[stdOutHandle readDataToEndOfFile]];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    
    [center removeObserver:self name:NSFileHandleReadCompletionNotification object:stdOutHandle];
    [center removeObserver:self name:NSTaskDidTerminateNotification object:task];
    [task release];

    commandIndex += 1;
    if (commandIndex < [[profile commands] count])
        [self runCurrentCommand];
    else
        [self release];
}

- (void)runCurrentCommand {
    CompilerProfileCommand* command = [[profile commands] objectAtIndex:commandIndex];
    
    NSArray* argComponents = [[command arguments] componentsSeparatedByCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
    NSMutableArray* argArray = [[NSMutableArray alloc] init];
    
    NSEnumerator* argEn = [argComponents objectEnumerator];
    NSString* arg;
    while ((arg = [argEn nextObject])) {
        NSEnumerator* replacementEn = [replacements keyEnumerator];
        NSString* searchStr;
        while ((searchStr = [replacementEn nextObject])) {
            if ([arg isEqualToString:searchStr]) {
                [argArray addObject:[replacements objectForKey:searchStr]];
                break;
            }
        }
        
        if (searchStr == nil)
            [argArray addObject:arg];
    }
    
    NSString* launchPath = [command path];
    if (![launchPath hasPrefix:@"/"]) {
        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* resourcePath = [mainBundle resourcePath];
        if (![resourcePath hasSuffix:@"/"])
            resourcePath = [resourcePath stringByAppendingString:@"/"];
        launchPath = [resourcePath stringByAppendingString:launchPath];
    }
    
    NSTask* task = [[NSTask alloc] init];
    [task setCurrentDirectoryPath:workDir];
    [task setLaunchPath:launchPath];
    [task setArguments:argArray];
    [argArray release];
    
    NSPipe* stdOutPipe = [[NSPipe alloc] init];
    NSFileHandle* stdOutHandle = [stdOutPipe fileHandleForReading];
    [task setStandardOutput:stdOutPipe];
    [stdOutPipe release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(stdOutReadCompleted:) name:NSFileHandleReadCompletionNotification object:stdOutHandle];
    [center addObserver:self selector:@selector(taskFinished:) name:NSTaskDidTerminateNotification object:task];
    
    [stdOutHandle readInBackgroundAndNotify];
    [task launch];
}

@end

@implementation CompilerProfileRunner

- (id)initWithProfile:(CompilerProfile *)theProfile console:(ConsoleWindowController *)theConsole workDir:(NSString *)theWorkDir replacements:(NSDictionary *)theReplacements {
    NSAssert(theProfile != nil, @"compiler profile must not be nil");
    NSAssert(theConsole != nil, @"console must not be nil");
    NSAssert(theWorkDir != nil, @"work dir must not be nil");
    NSAssert(theReplacements != nil, @"replacement dictionary must not be nil");
    
    if ((self = [self init])) {
        profile = [theProfile retain];
        console = [theConsole retain];
        workDir = [theWorkDir retain];
        replacements = [theReplacements retain];
    }
    
    return self;
}

- (void)dealloc {
    [profile release];
    [console release];
    [workDir release];
    [replacements release];
    [super dealloc];
}

- (void)run {
    [console logBold:[NSString stringWithFormat:@"===== Running Compiler Profile %@ =====\n\n", [profile name]]];
    
    if ([[profile commands] count] > 0) {
        commandIndex = 0;
        [self retain];
        [self runCurrentCommand];
    } else {
        [console logBold:[NSString stringWithFormat:@"Warning: no commands defined in this profile\n"]];
    }
}

@end
