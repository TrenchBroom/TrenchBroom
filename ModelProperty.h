//
//  ModelProperty.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "EntityDefinitionProperty.h"

@interface ModelProperty : NSObject <EntityDefinitionProperty> {
    NSString* flagName;
    NSString* modelPath;
    int skinIndex;
}

- (id)initWithModelPath:(NSString *)theModelPath;
- (id)initWithModelPath:(NSString *)theModelPath skinIndex:(int)theSkinIndex;
- (id)initWithFlagName:(NSString *)theFlagName modelPath:(NSString *)theModelPath;
- (id)initWithFlagName:(NSString *)theFlagName modelPath:(NSString *)theModelPath skinIndex:(int)theSkinIndex;

- (NSString *)flagName;
- (NSString *)modelPath;
- (int)skinIndex;

@end
