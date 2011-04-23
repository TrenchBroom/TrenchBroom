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
    NSString* modelPath;
}

- (id)initWithModelPath:(NSString *)theModelPath;

- (NSString *)modelPath;

@end
