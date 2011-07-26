//
//  EntityDefinitionNameFilter.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityDefinitionNameFilter.h"
#import "EntityDefinition.h"

@implementation EntityDefinitionNameFilter

- (id)initWithPattern:(NSString *)thePattern {
    if (self = [self init]) {
        pattern = [thePattern retain];
    }
    
    return self;
}

- (id)initWithPattern:(NSString *)thePattern filter:(id<EntityDefinitionFilter>)theFilter {
    if (self = [self initWithPattern:thePattern]) {
        filter = [theFilter retain];
    }
    
    return self;
}

- (BOOL)passes:(EntityDefinition *)entityDefinition {
    if (filter != nil && ![filter passes:entityDefinition])
        return NO;
    
    return [[entityDefinition name] rangeOfString:pattern].location != NSNotFound;
}

- (void)dealloc {
    [pattern release];
    [filter release];
    [super dealloc];
}

@end
