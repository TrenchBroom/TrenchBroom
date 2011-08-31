//
//  PreferencesManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

static NSString* const DefaultsQuakePath                = @"GamePath";
static NSString* const DefaultsQuakeExecutable          = @"GameExecutable";
static NSString* const DefaultsLastExecutablePath       = @"LastExecutablePath";
static NSString* const DefaultsLastCompilerProfileIndex = @"LastCompilerProfileIndex";
static NSString* const DefaultsCameraFov                = @"CameraFov";
static NSString* const DefaultsCameraNear               = @"CameraNearClippingPlane";
static NSString* const DefaultsCameraFar                = @"CameraFarClippingPlane";
static NSString* const DefaultsInspectorSeparate        = @"InspectorSeparateWindow";
static NSString* const DefaultsInspectorVisible         = @"InspectorVisible";

static NSString* const DefaultsKey                      = @"Key";
static NSString* const DefaultsOldValue                 = @"OldValue";
static NSString* const DefaultsNewValue                 = @"NewValue";
static NSString* const DefaultsDidChange                = @"DefaultsDidChangeNotification";

@interface PreferencesManager : NSObject

+ (PreferencesManager *)sharedManager;

- (NSString *)quakePath;
- (void)setQuakePath:(NSString *)theQuakePath;

- (NSString *)quakeExecutable;
- (void)setQuakeExecutable:(NSString*)theQuakeExecutable;

- (NSString *)lastExecutablePath;
- (void)setLastExecutablePath:(NSString *)theQuakePath;

- (NSArray *)availableExecutables;

- (int)lastCompilerProfileIndex;
- (void)setLastCompilerProfileIndex:(int)theIndex;

- (float)cameraFov;
- (void)setCameraFov:(float)theCameraFov;

- (float)cameraNear;
- (void)setCameraNear:(float)theCameraNear;

- (float)cameraFar;
- (void)setCameraFar:(float)theCameraFar;

- (BOOL)inspectorVisible;
- (void)setInspectorVisible:(BOOL)isInspectorVisible;

- (BOOL)inspectorSeparate;
- (void)setInspectorSeparate:(BOOL)isInspectorSeparate;

@end
