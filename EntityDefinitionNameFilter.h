//
//  EntityDefinitionNameFilter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "EntityDefinitionFilter.h"

@interface EntityDefinitionNameFilter : NSObject <EntityDefinitionFilter> {
    NSString* pattern;
    id <EntityDefinitionFilter> filter;
}

- (id)initWithPattern:(NSString *)thePattern;
- (id)initWithPattern:(NSString *)thePattern filter:(id<EntityDefinitionFilter>)theFilter;

@end
