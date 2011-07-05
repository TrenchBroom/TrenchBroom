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
}

- (id)initWithModelPath:(NSString *)theModelPath;
- (id)initWithFlagName:(NSString *)theFlagName modelPath:(NSString *)theModelPath;

- (NSString *)flagName;
- (NSString *)modelPath;

@end
