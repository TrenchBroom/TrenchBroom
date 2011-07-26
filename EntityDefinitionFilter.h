//
//  EntityFilter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@class EntityDefinition;

@protocol EntityDefinitionFilter <NSObject>

- (BOOL)passes:(EntityDefinition *)entityDefinition;

@end
