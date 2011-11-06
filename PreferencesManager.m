/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "PreferencesManager.h"

static PreferencesManager* sharedInstance = nil;

@implementation PreferencesManager

+ (PreferencesManager *)sharedManager {
    @synchronized(self) {
        if (sharedInstance == nil)
            sharedInstance = [[self alloc] init];
    }
    return sharedInstance;
}

+ (id)allocWithZone:(NSZone *)zone {
    @synchronized(self) {
        if (sharedInstance == nil) {
            sharedInstance = [super allocWithZone:zone];
            return sharedInstance;  // assignment and return on first allocation
        }
    }
    return nil; // on subsequent allocation attempts return nil
}

- (id)copyWithZone:(NSZone *)zone {
    return self;
}

- (id)retain {
    return self;
}

- (NSUInteger)retainCount {
    return UINT_MAX;  // denotes an object that cannot be released
}

- (oneway void)release {
    //do nothing
}

- (id)autorelease {
    return self;
}

- (NSString *)quakePath {
    return [[NSUserDefaults standardUserDefaults] stringForKey:DefaultsQuakePath];
}

- (void)setQuakePath:(NSString *)theQuakePath {
    NSString* currentQuakePath = [self quakePath];
    if ([currentQuakePath isEqualToString:theQuakePath])
        return;

    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:DefaultsQuakePath forKey:DefaultsKey];
    [userInfo setObject:currentQuakePath forKey:DefaultsOldValue];
    [userInfo setObject:theQuakePath forKey:DefaultsNewValue];
    
    [[NSUserDefaults standardUserDefaults] setValue:theQuakePath forKey:DefaultsQuakePath];
    [[NSNotificationCenter defaultCenter] postNotificationName:DefaultsDidChange object:self userInfo:userInfo];
    [userInfo release];
    
    [self setLastExecutablePath:nil];
}

- (NSString *)quakeExecutable {
    return [[NSUserDefaults standardUserDefaults] stringForKey:DefaultsQuakeExecutable];
}

- (void)setQuakeExecutable:(NSString*)theQuakeExecutable {
    NSString* currentQuakeExecutable = [self quakeExecutable];
    if ([currentQuakeExecutable isEqualToString:theQuakeExecutable])
        return;
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:DefaultsQuakeExecutable forKey:DefaultsKey];
    [userInfo setObject:currentQuakeExecutable forKey:DefaultsOldValue];
    [userInfo setObject:theQuakeExecutable forKey:DefaultsNewValue];
    
    [[NSUserDefaults standardUserDefaults] setValue:theQuakeExecutable forKey:DefaultsQuakeExecutable];
    [[NSNotificationCenter defaultCenter] postNotificationName:DefaultsDidChange object:self userInfo:userInfo];
    [userInfo release];
}

- (NSString *)lastExecutablePath {
    return [[NSUserDefaults standardUserDefaults] stringForKey:DefaultsLastExecutablePath];
}

- (void)setLastExecutablePath:(NSString *)theQuakePath {
    NSString* currentLastExecutablePath = [self lastExecutablePath];
    if ([currentLastExecutablePath isEqualToString:theQuakePath])
        return;
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:DefaultsLastExecutablePath forKey:DefaultsKey];
    if (currentLastExecutablePath != nil)
        [userInfo setObject:currentLastExecutablePath forKey:DefaultsOldValue];
    [userInfo setObject:theQuakePath forKey:DefaultsNewValue];
    
    [[NSUserDefaults standardUserDefaults] setValue:theQuakePath forKey:DefaultsLastExecutablePath];
    [[NSNotificationCenter defaultCenter] postNotificationName:DefaultsDidChange object:self userInfo:userInfo];
    [userInfo release];
}

- (NSArray *)availableExecutables {
    NSMutableArray* result = [NSMutableArray array];
    
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    
    BOOL directory;
    BOOL exists = [fileManager fileExistsAtPath:[self quakePath] isDirectory:&directory];
    
    if (exists && directory) {
        NSArray* contents = [fileManager contentsOfDirectoryAtPath:[self quakePath] error:NULL];
        NSEnumerator* filenameEn = [contents objectEnumerator];
        NSString* filename;
        while ((filename = [filenameEn nextObject])) {
            NSString* filePath = [NSString pathWithComponents:[NSArray arrayWithObjects:[self quakePath], filename, nil]];
            [fileManager fileExistsAtPath:filePath isDirectory:&directory];
            if (directory && [@"app" isEqualToString:[filePath pathExtension]] && [workspace isFilePackageAtPath:filePath])
                [result addObject:filePath];
        }
    }
    
    return result;
}

- (int)lastCompilerProfileIndex {
    return [[NSUserDefaults standardUserDefaults] integerForKey:DefaultsLastCompilerProfileIndex];
}

- (void)setLastCompilerProfileIndex:(int)theIndex {
    int currentIndex = [self lastCompilerProfileIndex];
    if (currentIndex == theIndex)
        return;
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:DefaultsLastCompilerProfileIndex forKey:DefaultsKey];
    [userInfo setObject:[NSNumber numberWithInt:currentIndex] forKey:DefaultsOldValue];
    [userInfo setObject:[NSNumber numberWithInt:theIndex] forKey:DefaultsNewValue];
    
    [[NSUserDefaults standardUserDefaults] setInteger:theIndex forKey:DefaultsLastCompilerProfileIndex];
    [[NSNotificationCenter defaultCenter] postNotificationName:DefaultsDidChange object:self userInfo:userInfo];
    [userInfo release];
}

- (float)cameraFov {
    return [[NSUserDefaults standardUserDefaults] floatForKey:DefaultsCameraFov];
}

- (void)setCameraFov:(float)theCameraFov {
    float currentCameraFov = [self cameraFov];
    if (currentCameraFov == theCameraFov)
        return;
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:DefaultsCameraFov forKey:DefaultsKey];
    [userInfo setObject:[NSNumber numberWithFloat:currentCameraFov] forKey:DefaultsOldValue];
    [userInfo setObject:[NSNumber numberWithFloat:theCameraFov] forKey:DefaultsNewValue];
    
    [[NSUserDefaults standardUserDefaults] setFloat:theCameraFov forKey:DefaultsCameraFov];
    [[NSNotificationCenter defaultCenter] postNotificationName:DefaultsDidChange object:self userInfo:userInfo];
    [userInfo release];
}

- (float)cameraNear {
    return [[NSUserDefaults standardUserDefaults] floatForKey:DefaultsCameraNear];
}

- (void)setCameraNear:(float)theCameraNear {
    float currentCameraNear = [self cameraNear];
    if (currentCameraNear == theCameraNear)
        return;
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:DefaultsCameraNear forKey:DefaultsKey];
    [userInfo setObject:[NSNumber numberWithFloat:currentCameraNear] forKey:DefaultsOldValue];
    [userInfo setObject:[NSNumber numberWithFloat:theCameraNear] forKey:DefaultsNewValue];
    
    [[NSUserDefaults standardUserDefaults] setFloat:theCameraNear forKey:DefaultsCameraNear];
    [[NSNotificationCenter defaultCenter] postNotificationName:DefaultsDidChange object:self userInfo:userInfo];
    [userInfo release];
}

- (float)cameraFar {
    return [[NSUserDefaults standardUserDefaults] floatForKey:DefaultsCameraFar];
}

- (void)setCameraFar:(float)theCameraFar {
    float currentCameraFar = [self cameraFar];
    if (currentCameraFar == theCameraFar)
        return;
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:DefaultsCameraFar forKey:DefaultsKey];
    [userInfo setObject:[NSNumber numberWithFloat:currentCameraFar] forKey:DefaultsOldValue];
    [userInfo setObject:[NSNumber numberWithFloat:theCameraFar] forKey:DefaultsNewValue];
    
    [[NSUserDefaults standardUserDefaults] setFloat:theCameraFar forKey:DefaultsCameraFar];
    [[NSNotificationCenter defaultCenter] postNotificationName:DefaultsDidChange object:self userInfo:userInfo];
    [userInfo release];
}

- (BOOL)inspectorVisible {
    return [[NSUserDefaults standardUserDefaults] boolForKey:DefaultsInspectorVisible];
}

- (void)setInspectorVisible:(BOOL)isInspectorVisible {
    BOOL currentIsInspectorVisible = [self inspectorVisible];
    if (currentIsInspectorVisible == isInspectorVisible)
        return;
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:DefaultsInspectorVisible forKey:DefaultsKey];
    [userInfo setObject:[NSNumber numberWithBool:currentIsInspectorVisible] forKey:DefaultsOldValue];
    [userInfo setObject:[NSNumber numberWithBool:isInspectorVisible] forKey:DefaultsNewValue];
    
    [[NSUserDefaults standardUserDefaults] setFloat:isInspectorVisible forKey:DefaultsInspectorVisible];
    [[NSNotificationCenter defaultCenter] postNotificationName:DefaultsDidChange object:self userInfo:userInfo];
    [userInfo release];
}

- (BOOL)inspectorSeparate {
    return [[NSUserDefaults standardUserDefaults] boolForKey:DefaultsInspectorSeparate];
}

- (void)setInspectorSeparate:(BOOL)isInspectorSeparate {
    BOOL currentIsInspectorSeparate = [self inspectorSeparate];
    if (currentIsInspectorSeparate == isInspectorSeparate)
        return;
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:DefaultsInspectorSeparate forKey:DefaultsKey];
    [userInfo setObject:[NSNumber numberWithBool:currentIsInspectorSeparate] forKey:DefaultsOldValue];
    [userInfo setObject:[NSNumber numberWithBool:isInspectorSeparate] forKey:DefaultsNewValue];
    
    [[NSUserDefaults standardUserDefaults] setBool:isInspectorSeparate forKey:DefaultsInspectorSeparate];
    [[NSNotificationCenter defaultCenter] postNotificationName:DefaultsDidChange object:self userInfo:userInfo];
    [userInfo release];
}

- (float)brightness {
    return [[NSUserDefaults standardUserDefaults] floatForKey:DefaultsBrightness];
}

- (void)setBrightness:(float)theBrightness {
    float currentBrightness = [self brightness];
    if (currentBrightness == theBrightness)
        return;
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:DefaultsBrightness forKey:DefaultsKey];
    [userInfo setObject:[NSNumber numberWithFloat:currentBrightness] forKey:DefaultsOldValue];
    [userInfo setObject:[NSNumber numberWithFloat:theBrightness] forKey:DefaultsNewValue];
    
    [[NSUserDefaults standardUserDefaults] setFloat:theBrightness forKey:DefaultsBrightness];
    [[NSNotificationCenter defaultCenter] postNotificationName:DefaultsDidChange object:self userInfo:userInfo];
    [userInfo release];
}

@end
