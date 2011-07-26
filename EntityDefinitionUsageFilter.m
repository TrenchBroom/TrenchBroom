//
//  EntityUsageFilter.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityDefinitionUsageFilter.h"
#import "EntityDefinition.h"

@implementation EntityDefinitionUsageFilter

- (id)initWithFilter:(id<EntityDefinitionFilter>)theFilter {
    if (self = [self init]) {
        filter = [theFilter retain];
    }
    
    return self;
}

- (BOOL)passes:(EntityDefinition *)entityDefinition {
    if (filter != nil && ![filter passes:entityDefinition])
        return NO;
    
    return [entityDefinition usageCount] > 0;
}

- (void)dealloc {
    [filter release];
    [super dealloc];
}

@end
