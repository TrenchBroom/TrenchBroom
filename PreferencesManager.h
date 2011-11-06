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
static NSString* const DefaultsBrightness               = @"Brightness";

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

- (float)brightness;
- (void)setBrightness:(float)theBrightness;

@end
