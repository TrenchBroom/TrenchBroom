//
//  EntitiyDefinitionManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "EntityDefinition.h"

typedef enum {
    ES_NAME,
    ES_USAGE
} EEntityDefinitionSortCriterion;

@interface EntityDefinitionManager : NSObject {
    NSMutableDictionary* definitions;
    NSMutableArray* definitionsByName;
}

- (id)initWithDefinitionFile:(NSString *)thePath;

- (EntityDefinition *)definitionForName:(NSString *)name;
- (NSArray *)definitions;
- (NSArray *)definitionsOfType:(EEntityDefinitionType)type;
- (NSArray *)definitionsOfType:(EEntityDefinitionType)type sortCriterion:(EEntityDefinitionSortCriterion)criterion;

@end
