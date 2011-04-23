//
//  EntitiyDefinitionManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class EntityDefinition;

@interface EntityDefinitionManager : NSObject {
    NSMutableDictionary* definitions;
    NSMutableArray* definitionsByName;
}

- (id)initWithDefinitionFile:(NSString *)thePath;

- (EntityDefinition *)definitionForName:(NSString *)name;
- (NSArray *)definitions;

@end
