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

#import "ModelProperty.h"


@implementation ModelProperty

- (id)initWithModelPath:(NSString *)theModelPath {
    NSAssert(theModelPath != nil, @"model path must not be nil");
    
    if ((self = [self init])) {
        modelPath = [theModelPath retain];
    }
    
    return self;
}

- (id)initWithModelPath:(NSString *)theModelPath skinIndex:(int)theSkinIndex {
    NSAssert(theModelPath != nil, @"model path must not be nil");
    NSAssert(theSkinIndex >= 0, @"skin index must be at least 0");
    
    if ((self = [self init])) {
        modelPath = [theModelPath retain];
        skinIndex = theSkinIndex;
    }
    
    return self;
}

- (id)initWithFlagName:(NSString *)theFlagName modelPath:(NSString *)theModelPath {
    NSAssert(theFlagName != nil, @"flag name must not be nil");
    NSAssert(theModelPath != nil, @"model path must not be nil");
    
    if ((self = [self init])) {
        flagName = [theFlagName retain];
        modelPath = [theModelPath retain];
    }
    
    return self;
}

- (id)initWithFlagName:(NSString *)theFlagName modelPath:(NSString *)theModelPath skinIndex:(int)theSkinIndex {
    NSAssert(theFlagName != nil, @"flag name must not be nil");
    NSAssert(theModelPath != nil, @"model path must not be nil");
    NSAssert(theSkinIndex >= 0, @"skin index must be at least 0");
    
    if ((self = [self init])) {
        flagName = [theFlagName retain];
        modelPath = [theModelPath retain];
        skinIndex = theSkinIndex;
    }
    
    return self;
}

- (void)dealloc {
    [modelPath release];
    [super dealloc];
}

- (EEntityDefinitionPropertyType)type {
    return EDP_MODEL;
}

- (NSString *)flagName {
    return flagName;
}

- (NSString *)modelPath {
    return modelPath;
}

- (int)skinIndex {
    return skinIndex;
}

- (NSString *)description {
    NSMutableString* desc = [[NSMutableString alloc] initWithString:modelPath];
    [desc appendFormat:@", skin: ", skinIndex];
    if (flagName != nil)
        [desc appendFormat:@", flag: ", flagName];

    return [desc autorelease];
}

@end
