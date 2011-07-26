//
//  EntityUsageFilter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "EntityDefinitionFilter.h"

@interface EntityDefinitionUsageFilter : NSObject <EntityDefinitionFilter> {
    id<EntityDefinitionFilter> filter;
}

- (id)initWithFilter:(id<EntityDefinitionFilter>)theFilter;
@end
