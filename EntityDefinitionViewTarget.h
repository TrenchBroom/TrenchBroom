//
//  EntityViewTarget.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@class EntityDefinition;

@protocol EntityDefinitionViewTarget <NSObject>

- (void)entityDefinitionSelected:(EntityDefinition *)theDefinition;

@end
